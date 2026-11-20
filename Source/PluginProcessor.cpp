// OMEGA — by Polaris Sound Lab (Mérida, Yucatán). Design: Dark Funk Durcal.
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Lang.h"
#include "LicenceKey.h"
#include <juce_cryptography/juce_cryptography.h>

using namespace om;

juce::AudioProcessorValueTreeState::ParameterLayout OmegaProcessor::layout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> v;
    auto fl = [&] (const char* id, const char* name, float lo, float hi, float def, const char* label = "", float skew = 1.0f)
    {
        NormalisableRange<float> r (lo, hi, 0.0001f); r.setSkewForCentre (skew == 1.0f ? (lo + hi) * 0.5f : skew);
        if (skew == 1.0f) r = NormalisableRange<float> (lo, hi, 0.0001f);
        const String lab (label);
        const int dec = lab == "dBTP" ? 1 : (lab == "Hz" ? 0 : (hi - lo <= 2.0f ? 2 : 1));
        auto str = [lab, dec, id] (float val, int) -> String
        {
            if (String (id) == "amount") return String (roundToInt (val * 100.0f)) + "%";
            if (String (id) == "bassmono" && val < 20.0f) return "off";
            return String (val, dec) + (lab.isNotEmpty() ? " " + lab : String());
        };
        v.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, r, def,
                        AudioParameterFloatAttributes().withLabel (label).withStringFromValueFunction (str)));
    };
    fl ("amount", "Omega", 0.0f, 1.0f, 0.5f);
    StringArray uni; for (auto& u : kUniverses) uni.add (u.en);
    v.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "universe", 1 }, "Universe", uni, 0));
    fl ("input", "Input", -24.0f, 24.0f, 0.0f, "dB");
    fl ("output", "Output", -24.0f, 12.0f, 0.0f, "dB");
    fl ("low", "Low", -6.0f, 6.0f, 0.0f, "dB");
    fl ("mid", "Mid", -6.0f, 6.0f, 0.0f, "dB");
    fl ("high", "High", -6.0f, 6.0f, 0.0f, "dB");
    fl ("air", "Air", -6.0f, 6.0f, 0.0f, "dB");
    for (auto* id : { "tame", "punch", "glue", "harmonics", "width", "clip" })
    {
        String nm (id); nm = nm.substring (0, 1).toUpperCase() + nm.substring (1);
        fl (id, nm.toRawUTF8(), -1.0f, 1.0f, 0.0f);
    }
    fl ("bassmono", "Bass Mono", 0.0f, 300.0f, 0.0f, "Hz");
    fl ("ceiling", "Ceiling", -3.0f, 0.0f, -1.0f, "dBTP");
    fl ("drive", "Loudness", 0.0f, 18.0f, 0.0f, "dB");
    v.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "target", 1 }, "Target",
                    StringArray { "Off", "Streaming -14", "Apple Music -16", "Loud -10", "Club -7" }, 1));
    fl ("match", "Reference Match", 0.0f, 1.0f, 0.0f);
    v.push_back (std::make_unique<AudioParameterBool> (ParameterID { "compare", 1 }, "Compare (level-matched)", false));
    v.push_back (std::make_unique<AudioParameterBool> (ParameterID { "delta", 1 }, "Delta", false));
    v.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "quality", 1 }, "Quality", StringArray { "2x", "4x", "8x" }, 1,
                    AudioParameterChoiceAttributes().withAutomatable (false)));
    return { v.begin(), v.end() };
}

float OmegaProcessor::targetLufs (int idx)
{
    static const float t[] = { -14.0f, -14.0f, -16.0f, -10.0f, -7.0f };
    return t[juce::jlimit (0, 4, idx)];
}

OmegaProcessor::OmegaProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, &undo, "OMEGA", layout())
{
    for (auto& m : matchCurve) m.store (0.0f);
    specOutFast.fill (-80); specOutLong.fill (-80); specInLong.fill (-80); spectrumLine.fill (-90);
    licence();
    startTimerHz (30);
}

OmegaProcessor::~OmegaProcessor()
{
    stopTimer();
    if (renderThread) renderThread->stopThread (4000);
}

bool OmegaProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    using CS = juce::AudioChannelSet;
    const auto in = l.getMainInputChannelSet(), out = l.getMainOutputChannelSet();
    if (in != out) return false;
    for (auto& s : { CS::stereo(), CS::create5point1(), CS::create7point1(), CS::create5point1point4(), CS::create7point1point2(), CS::create7point1point4() })
        if (out == s) return true;
    return false;
}

void OmegaProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate; maxBlock = juce::jmax (1, samplesPerBlock);
    const auto set = getBus (false, 0) != nullptr ? getBus (false, 0)->getCurrentLayout() : juce::AudioChannelSet::stereo();
    const int nCh = juce::jmax (1, set.size());
    qualityPrepared = (int) P ("quality")->load();
    engine.prepare (sr, maxBlock, nCh, qualityPrepared + 1, set);
    setLatencySamples (engine.getLatency());
    meterIn.prepare (sr, nCh, set); meterOut.prepare (sr, nCh, set);
    dry.setSize (nCh, maxBlock * 2);
    tpBuf.setSize (nCh, maxBlock * 2);
    tpOs = std::make_unique<juce::dsp::Oversampling<float>> ((size_t) nCh, 2, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true);
    tpOs->initProcessing ((size_t) maxBlock * 2);
    fifo.reset(); fifoOut.assign ((size_t) fifo.getTotalSize(), 0.0f); fifoIn.assign ((size_t) fifo.getTotalSize(), 0.0f);
    fftWork.assign (8192, 0.0f); histOut.assign (4096, 0.0f); histIn.assign (4096, 0.0f); histPos = 0;
}

void OmegaProcessor::fillSettings (Settings& s)
{
    s.universe = (int) P ("universe")->load();
    s.amount = P ("amount")->load();
    s.inputDb = P ("input")->load(); s.outputDb = P ("output")->load();
    s.low = P ("low")->load(); s.mid = P ("mid")->load(); s.high = P ("high")->load(); s.air = P ("air")->load();
    s.tame = P ("tame")->load(); s.punch = P ("punch")->load(); s.glue = P ("glue")->load();
    s.harmonics = P ("harmonics")->load(); s.width = P ("width")->load(); s.clip = P ("clip")->load();
    s.bassMonoHz = P ("bassmono")->load();
    s.ceilingDb = P ("ceiling")->load(); s.driveDb = P ("drive")->load();
    s.match = hasRef ? P ("match")->load() : 0.0f;
    for (int b = 0; b < 10; ++b) s.matchDb[(size_t) b] = matchCurve[(size_t) b].load();
}

void OmegaProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals nd;
    const int n = buffer.getNumSamples();
    if (n == 0) return;
    if (auto* ph = getPlayHead()) if (auto pos = ph->getPosition()) playing = pos->getIsPlaying();
    if (licence().expired()) return;                        // trial over: untouched pass-through
    if (n > dry.getNumSamples()) { dry.setSize (dry.getNumChannels(), n, false, false, true); tpBuf.setSize (tpBuf.getNumChannels(), n, false, false, true); }

    meterIn.process (buffer.getArrayOfReadPointers(), n);
    float ip = 0; for (int c = 0; c < buffer.getNumChannels(); ++c) ip = std::max (ip, buffer.getMagnitude (c, 0, n));
    inPeakDb = std::max (gainToDb (ip), inPeakDb.load() - 1.0f);

    Settings s; fillSettings (s);
    engine.process (buffer, s);
    engine.readDry (dry, n);

    meterOut.process (buffer.getArrayOfReadPointers(), n);
    // true peak (4× FIR) of the processed output
    {
        for (int c = 0; c < buffer.getNumChannels() && c < tpBuf.getNumChannels(); ++c) tpBuf.copyFrom (c, 0, buffer, c, 0, n);
        juce::dsp::AudioBlock<float> b (tpBuf.getArrayOfWritePointers(), (size_t) tpBuf.getNumChannels(), (size_t) n);
        auto up = tpOs->processSamplesUp (b);
        float m = 0;
        for (size_t c = 0; c < up.getNumChannels(); ++c) { auto* p = up.getChannelPointer (c); for (size_t i = 0; i < up.getNumSamples(); ++i) m = std::max (m, std::abs (p[i])); }
        const float db = gainToDb (m);
        truePeakDb = std::max (db, truePeakDb.load() - 0.5f);
        truePeakMax = std::max (truePeakMax.load(), db);
        float op = 0; for (int c = 0; c < buffer.getNumChannels(); ++c) op = std::max (op, buffer.getMagnitude (c, 0, n));
        outPeakDb = std::max (gainToDb (op), outPeakDb.load() - 1.0f);
    }
    // spectrum feed (mono)
    {
        int s1, z1, s2, z2; fifo.prepareToWrite (n, s1, z1, s2, z2);
        auto put = [&] (int start, int len, int off)
        {
            for (int i = 0; i < len; ++i)
            {
                float o = 0, d = 0;
                for (int c = 0; c < buffer.getNumChannels(); ++c) { o += buffer.getSample (c, off + i); d += dry.getSample (c, off + i); }
                fifoOut[(size_t) (start + i)] = o; fifoIn[(size_t) (start + i)] = d;
            }
        };
        put (s1, z1, 0); put (s2, z2, z1); fifo.finishedWrite (z1 + z2);
    }
    // level-matched compare / delta (dry is aligned with the output latency)
    const bool cmp = P ("compare")->load() > 0.5f, del = P ("delta")->load() > 0.5f;
    if (cmp || del)
    {
        const float g = dbToGain (matchGainDb.load());
        for (int c = 0; c < buffer.getNumChannels() && c < dry.getNumChannels(); ++c)
        {
            float* y = buffer.getWritePointer (c); const float* d = dry.getReadPointer (c);
            for (int i = 0; i < n; ++i) y[i] = cmp ? d[i] * g : y[i] - d[i] * g;
        }
    }
}

// ───────────────────────────── analysis + assistants (message thread) ─────────────────────────────
void OmegaProcessor::analyseFifo()
{
    int s1, z1, s2, z2;
    const int avail = fifo.getNumReady();
    fifo.prepareToRead (avail, s1, z1, s2, z2);
    auto take = [&] (int start, int len)
    {
        for (int i = 0; i < len; ++i)
        {
            histOut[(size_t) histPos] = fifoOut[(size_t) (start + i)]; histIn[(size_t) histPos] = fifoIn[(size_t) (start + i)];
            if (++histPos >= 4096)
            {
                histPos = 0;
                auto bands = [&] (const std::vector<float>& src, std::array<float, 10>& outB, bool line)
                {
                    std::fill (fftWork.begin(), fftWork.end(), 0.0f);
                    for (int k = 0; k < 4096; ++k) fftWork[(size_t) k] = src[(size_t) k] * (0.5f - 0.5f * std::cos (2.0f * kPi * (float) k / 4095.0f));
                    fft.performFrequencyOnlyForwardTransform (fftWork.data());
                    const float binHz = (float) sr / 4096.0f;
                    for (int b = 0; b < 10; ++b)
                    {
                        const int lo = juce::jmax (1, (int) (kOct[b] / 1.414f / binHz)), hi = juce::jmin (2047, (int) (kOct[b] * 1.414f / binHz));
                        double p = 0; for (int k = lo; k <= hi; ++k) p += (double) fftWork[(size_t) k] * fftWork[(size_t) k];
                        outB[(size_t) b] = (float) (10.0 * std::log10 (p / (2048.0 * 2048.0 * 0.25) + 1e-12));
                    }
                    if (line)
                        for (int j = 0; j < 64; ++j)
                        {
                            const float f0 = 30.0f * std::pow (2.0f, (float) j / 64.0f * 9.4f), f1 = f0 * std::pow (2.0f, 9.4f / 64.0f);
                            const int lo = juce::jmax (1, (int) (f0 / binHz)), hi = juce::jmax (lo, juce::jmin (2047, (int) (f1 / binHz)));
                            double p = 0; for (int k = lo; k <= hi; ++k) p += (double) fftWork[(size_t) k] * fftWork[(size_t) k];
                            const float db = (float) (10.0 * std::log10 (p / (2048.0 * 2048.0 * 0.25) + 1e-12)) + 3.0f * std::log2 (f0 / 1000.0f);
                            spectrumLine[(size_t) j] += 0.35f * (db - spectrumLine[(size_t) j]);
                        }
                };
                std::array<float, 10> bo {}, bi {};
                bands (histOut, bo, true); bands (histIn, bi, false);
                specOutFast = bo;
                const bool sound = bo[5] > -75.0f;
                if (sound)
                    for (int b = 0; b < 10; ++b)
                    {
                        specOutLong[(size_t) b] += 0.04f * (bo[(size_t) b] - specOutLong[(size_t) b]);
                        specInLong[(size_t) b] += 0.04f * (bi[(size_t) b] - specInLong[(size_t) b]);
                    }
            }
        }
    };
    take (s1, z1); take (s2, z2); fifo.finishedRead (z1 + z2);

    // tone compass: regression slope of octave bands 63 Hz … 8 kHz (dB per octave)
    double sx = 0, sy = 0, sxx = 0, sxy = 0; int cnt = 0;
    for (int b = 1; b <= 8; ++b) { const double x = b, y = specOutLong[(size_t) b]; sx += x; sy += y; sxx += x * x; sxy += x * y; ++cnt; }
    const double den = cnt * sxx - sx * sx;
    if (den > 0) toneSlope = (float) ((cnt * sxy - sx * sy) / den);

    // reference match curve
    if (hasRef)
    {
        float rm = 0, mm = 0;
        for (int b = 1; b < 9; ++b) { rm += refBands[(size_t) b]; mm += specInLong[(size_t) b]; }
        rm /= 8; mm /= 8;
        for (int b = 0; b < 10; ++b)
        {
            float d = (refBands[(size_t) b] - rm) - (specInLong[(size_t) b] - mm);
            if (b == 0 || b == 9) d *= 0.5f;
            matchCurve[(size_t) b].store (juce::jlimit (-4.0f, 4.0f, d));
        }
    }
}

void OmegaProcessor::balanceTone()
{
    const float delta = toneSlope - (-2.6f);                  // >0 → too bright, <0 → too dark
    auto setP = [this] (const char* id, float v) { if (auto* p = apvts.getParameter (id)) { p->beginChangeGesture(); p->setValueNotifyingHost (p->convertTo0to1 (v)); p->endChangeGesture(); } };
    setP ("high", juce::jlimit (-6.0f, 6.0f, P ("high")->load() - juce::jlimit (-3.0f, 3.0f, delta * 1.6f)));
    setP ("low",  juce::jlimit (-6.0f, 6.0f, P ("low")->load()  + juce::jlimit (-2.0f, 2.0f, delta * 0.8f)));
}

void OmegaProcessor::startAutoInput() { autoInputLeft = 6.0f; autoInputMax = -100.0f; }
void OmegaProcessor::startLoudnessMatch() { loudLeft = 24.0f; loudWindowMax = -100.0f; loudStable = 0; }

void OmegaProcessor::timerCallback()
{
    if ((int) P ("quality")->load() != qualityPrepared && qualityPrepared >= 0)
    {
        suspendProcessing (true); prepareToPlay (sr, maxBlock); suspendProcessing (false);
    }
    analyseFifo();
    const float dt = 1.0f / 30.0f;
    const float inST = meterIn.shortTerm.load(), outST = meterOut.shortTerm.load();
    if (inST > -60 && outST > -60) matchGainDb = matchGainDb.load() + 0.05f * ((outST - inST) - matchGainDb.load());

    auto setP = [this] (const char* id, float v) { if (auto* p = apvts.getParameter (id)) { p->beginChangeGesture(); p->setValueNotifyingHost (p->convertTo0to1 (v)); p->endChangeGesture(); } };
    if (autoInputLeft > 0 && playing.load())
    {
        autoInputMax = std::max (autoInputMax, meterIn.momentary.load());
        autoInputLeft -= dt;
        if (autoInputLeft <= 0 && autoInputMax > -60) setP ("input", juce::jlimit (-24.0f, 24.0f, -17.0f - autoInputMax));
    }
    if (loudLeft > 0 && playing.load())
    {
        static float window = 0; window += dt; loudLeft -= dt;
        loudWindowMax = std::max (loudWindowMax, outST);
        if (window >= 1.6f)
        {
            window = 0;
            const float tgt = targetLufs ((int) P ("target")->load());
            if (loudWindowMax > -60)
            {
                const float err = tgt - loudWindowMax;
                setP ("drive", juce::jlimit (0.0f, 18.0f, P ("drive")->load() + juce::jlimit (-3.0f, 3.0f, 0.75f * err)));
                loudStable = std::abs (err) < 0.4f ? loudStable + 1 : 0;
                if (loudStable >= 2) loudLeft = 0;
            }
            loudWindowMax = -100.0f;
        }
    }
}

// ───────────────────────────── reference ─────────────────────────────
bool OmegaProcessor::loadReference (const juce::File& f)
{
    juce::AudioFormatManager fm; fm.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> r (fm.createReaderFor (f));
    if (r == nullptr) return false;
    const int ch = (int) juce::jmin (2u, r->numChannels);
    const juce::int64 len = juce::jmin (r->lengthInSamples, (juce::int64) (r->sampleRate * 600));
    LoudnessMeter lm; lm.prepare (r->sampleRate, ch, ch == 1 ? juce::AudioChannelSet::mono() : juce::AudioChannelSet::stereo());
    juce::AudioBuffer<float> b (ch, 4096);
    std::array<double, 10> acc {}; int frames = 0;
    juce::dsp::FFT f12 (12); std::vector<float> w (8192);
    const float binHz = (float) r->sampleRate / 4096.0f;
    for (juce::int64 pos = 0; pos + 4096 <= len; pos += 4096)
    {
        r->read (&b, 0, 4096, pos, true, ch > 1);
        lm.process (b.getArrayOfReadPointers(), 4096);
        std::fill (w.begin(), w.end(), 0.0f);
        for (int k = 0; k < 4096; ++k) { float m = 0; for (int c = 0; c < ch; ++c) m += b.getSample (c, k); w[(size_t) k] = m * (0.5f - 0.5f * std::cos (2.0f * kPi * (float) k / 4095.0f)); }
        f12.performFrequencyOnlyForwardTransform (w.data());
        double tot = 0; for (int k = 1; k < 2048; ++k) tot += (double) w[(size_t) k] * w[(size_t) k];
        if (tot < 1e-3) continue;
        for (int bb = 0; bb < 10; ++bb)
        {
            const int lo = juce::jmax (1, (int) (kOct[bb] / 1.414f / binHz)), hi = juce::jmin (2047, (int) (kOct[bb] * 1.414f / binHz));
            double p = 0; for (int k = lo; k <= hi; ++k) p += (double) w[(size_t) k] * w[(size_t) k];
            acc[(size_t) bb] += p;
        }
        ++frames;
    }
    if (frames == 0) return false;
    for (int bb = 0; bb < 10; ++bb) refBands[(size_t) bb] = (float) (10.0 * std::log10 (acc[(size_t) bb] / frames / (2048.0 * 2048.0 * 0.25) + 1e-12));
    refLufs = lm.integrated();
    refName = f.getFileNameWithoutExtension();
    hasRef = true;
    return true;
}

// ───────────────────────────── surround / multichannel file mastering ─────────────────────────────
void OmegaProcessor::renderFile (const juce::File& file)
{
    if (renderThread && renderThread->isThreadRunning()) return;
    Settings s; fillSettings (s);
    const int q = (int) P ("quality")->load();
    struct R : juce::Thread
    {
        R (OmegaProcessor& o, juce::File f, Settings st, int qq) : juce::Thread ("OMEGA render"), owner (o), file (std::move (f)), s (st), q (qq) {}
        void run() override
        {
            owner.renderProgress = 0.0f;
            juce::AudioFormatManager fm; fm.registerBasicFormats();
            std::unique_ptr<juce::AudioFormatReader> r (fm.createReaderFor (file));
            if (r == nullptr) { owner.renderResult = "?"; owner.renderProgress = -1.0f; return; }
            const int ch = (int) r->numChannels;
            auto set = r->getChannelLayout();
            if (set.size() != ch) set = juce::AudioChannelSet::discreteChannels (ch);
            Engine e; e.prepare (r->sampleRate, 2048, ch, q + 1, set);
            const int lat = e.getLatency();
            auto outFile = file.getSiblingFile (file.getFileNameWithoutExtension() + "_OMEGA.wav");
            outFile.deleteFile();
            std::unique_ptr<juce::OutputStream> os (outFile.createOutputStream());
            juce::WavAudioFormat wav;
            std::unique_ptr<juce::AudioFormatWriter> w (wav.createWriterFor (os.get(), r->sampleRate, set, 24, {}, 0));
            if (w == nullptr) { owner.renderResult = "?"; owner.renderProgress = -1.0f; return; }
            os.release();
            juce::AudioBuffer<float> b (ch, 2048);
            juce::int64 written = 0, readPos = 0; const juce::int64 total = r->lengthInSamples;
            int skip = lat;
            while (written < total && ! threadShouldExit())
            {
                b.clear();
                const int toRead = (int) std::max<juce::int64> (0, std::min<juce::int64> (2048, total - readPos));
                if (toRead > 0) r->read (&b, 0, toRead, readPos, true, true);
                readPos += 2048;
                e.process (b, s);
                int start = juce::jmin (skip, 2048); skip -= start;
                const int len = (int) std::min<juce::int64> ((juce::int64) (2048 - start), total - written);
                if (len > 0)
                {
                    const float* ptr[kMaxCh] {}; for (int c = 0; c < ch && c < kMaxCh; ++c) ptr[c] = b.getReadPointer (c) + start;
                    w->writeFromFloatArrays (ptr, ch, len);
                    written += len;
                }
                owner.renderProgress = (float) written / (float) std::max<juce::int64> (1, total);
            }
            w.reset();
            owner.renderResult = outFile.getFullPathName();
            owner.renderProgress = 2.0f;   // done
        }
        OmegaProcessor& owner; juce::File file; Settings s; int q;
    };
    if (renderThread) renderThread->stopThread (2000);
    renderThread = std::make_unique<R> (*this, file, s, q);
    renderThread->startThread();
}

// ───────────────────────────── snapshots + state ─────────────────────────────
void OmegaProcessor::storeSnapshot (int i) { snapshots[(size_t) i] = apvts.copyState(); }
void OmegaProcessor::recallSnapshot (int i)
{
    if (! snapshots[(size_t) i].isValid()) { storeSnapshot (i); return; }
    apvts.replaceState (snapshots[(size_t) i].createCopy());
}

void OmegaProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto st = apvts.copyState();
    st.setProperty ("lang", langFlag().load(), nullptr);
    st.setProperty ("refName", refName, nullptr); st.setProperty ("refLufs", refLufs, nullptr); st.setProperty ("hasRef", hasRef, nullptr);
    juce::String rb; for (auto v : refBands) rb << juce::String (v, 2) << ";";
    st.setProperty ("refBands", rb, nullptr);
    if (auto xml = st.createXml()) copyXmlToBinary (*xml, dest);
}

void OmegaProcessor::setStateInformation (const void* data, int size)
{
    auto xml = getXmlFromBinary (data, size);
    if (xml == nullptr) return;
    auto st = juce::ValueTree::fromXml (*xml);
    if (! st.isValid()) return;
    langFlag() = (int) st.getProperty ("lang", 0);
    hasRef = (bool) st.getProperty ("hasRef", false);
    refName = st.getProperty ("refName", "").toString(); refLufs = (float) st.getProperty ("refLufs", -100.0f);
    auto parts = juce::StringArray::fromTokens (st.getProperty ("refBands", "").toString(), ";", "");
    for (int b = 0; b < 10 && b < parts.size(); ++b) refBands[(size_t) b] = parts[b].getFloatValue();
    apvts.replaceState (st);
}

// ───────────────────────────── licence (15-day trial, offline RSA key) ─────────────────────────────
namespace
{
juce::File licenceFile()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
             .getChildFile ("Polaris Sound Lab").getChildFile ("Omega").getChildFile ("licence.xml");
}
juce::String normEmail (const juce::String& e) { return e.trim().toLowerCase(); }
bool verifyKey (const juce::String& email, const juce::String& key)
{
    const auto hex = key.retainCharacters ("0123456789abcdefABCDEF");
    if (hex.length() < 64 || normEmail (email).isEmpty()) return false;
    juce::BigInteger n, sgn, e (65537);
    n.parseString (PSL_LICENCE_N, 16); sgn.parseString (hex, 16);
    if (sgn >= n) return false;
    const auto msg = juce::String ("OMEGA|1|") + normEmail (email);
    juce::BigInteger m; m.parseString (juce::SHA256 (msg.toUTF8()).toHexString(), 16);
    sgn.exponentModulo (e, n);
    return sgn == m;
}
} // namespace

OmegaProcessor::Licence& OmegaProcessor::licence()
{
    static Licence L = []
    {
        Licence l;
        auto f = licenceFile();
        std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (f));
        const juce::int64 now = juce::Time::currentTimeMillis();
        juce::int64 first = now;
        if (xml != nullptr && xml->hasAttribute ("t")) first = xml->getStringAttribute ("t").getHexValue64() ^ (juce::int64) 0x0e6a0e6a1234abcdLL;
        else
        {
            f.getParentDirectory().createDirectory();
            juce::XmlElement x ("licence"); x.setAttribute ("t", juce::String::toHexString (now ^ (juce::int64) 0x0e6a0e6a1234abcdLL)); x.writeTo (f);
        }
        if (first > now) first = now;
        l.daysLeft = juce::jmax (0, 15 - (int) std::floor ((double) (now - first) / 86400000.0));
        if (xml != nullptr && verifyKey (xml->getStringAttribute ("email"), xml->getStringAttribute ("key")))
        { l.licensed = true; l.owner = normEmail (xml->getStringAttribute ("email")); }
        return l;
    }();
    return L;
}

bool OmegaProcessor::activate (const juce::String& email, const juce::String& key)
{
    if (! verifyKey (email, key)) return false;
    auto f = licenceFile();
    std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (f));
    if (xml == nullptr) xml = std::make_unique<juce::XmlElement> ("licence");
    xml->setAttribute ("email", normEmail (email));
    xml->setAttribute ("key", key.retainCharacters ("0123456789abcdefABCDEF"));
    f.getParentDirectory().createDirectory(); xml->writeTo (f);
    auto& l = licence(); l.licensed = true; l.owner = normEmail (email);
    return true;
}

juce::AudioProcessorEditor* OmegaProcessor::createEditor() { return new OmegaEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new OmegaProcessor(); }
