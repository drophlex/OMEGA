// OMEGA — mix-bus / mastering engine. Polaris Sound Lab. Channel-count agnostic (stereo … 7.1.4).
#pragma once
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <array>
#include <vector>
#include <deque>
#include <cmath>

namespace om
{
constexpr float kPi = 3.14159265358979f;
inline float dbToGain (float db) noexcept { return std::pow (10.0f, db * 0.05f); }
inline float gainToDb (float g)  noexcept { return 20.0f * std::log10 (std::max (g, 1.0e-9f)); }
inline float sat01 (float x)     noexcept { return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x); }

// ───────────────────────────── Biquad (RBJ, TDF-II) ─────────────────────────────
struct Biquad
{
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
    inline float process (float x) noexcept { const float y = b0 * x + z1; z1 = b1 * x - a1 * y + z2; z2 = b2 * x - a2 * y; return y; }
    void clear() noexcept { z1 = z2 = 0; }
    void set (float nb0, float nb1, float nb2, float na0, float na1, float na2) noexcept { const float i = 1.0f / na0; b0 = nb0 * i; b1 = nb1 * i; b2 = nb2 * i; a1 = na1 * i; a2 = na2 * i; }
    static float fc (double sr, float f) { return juce::jlimit (10.0f, (float) (sr * 0.45), f); }
    void lowpass (double sr, float f, float q = 0.7071f)  { f = fc (sr, f); const float w = 2 * kPi * f / (float) sr, c = std::cos (w), al = std::sin (w) / (2 * q); set ((1 - c) / 2, 1 - c, (1 - c) / 2, 1 + al, -2 * c, 1 - al); }
    void highpass (double sr, float f, float q = 0.7071f) { f = fc (sr, f); const float w = 2 * kPi * f / (float) sr, c = std::cos (w), al = std::sin (w) / (2 * q); set ((1 + c) / 2, -(1 + c), (1 + c) / 2, 1 + al, -2 * c, 1 - al); }
    void bandpass (double sr, float f, float q)          { f = fc (sr, f); const float w = 2 * kPi * f / (float) sr, c = std::cos (w), al = std::sin (w) / (2 * q); set (al, 0, -al, 1 + al, -2 * c, 1 - al); }
    void peak (double sr, float f, float q, float db)
    {
        f = fc (sr, f); const float A = std::pow (10.0f, db / 40.0f), w = 2 * kPi * f / (float) sr, c = std::cos (w), al = std::sin (w) / (2 * q);
        set (1 + al * A, -2 * c, 1 - al * A, 1 + al / A, -2 * c, 1 - al / A);
    }
    void lowShelf (double sr, float f, float db, float S = 0.8f)
    {
        f = fc (sr, f); const float A = std::pow (10.0f, db / 40.0f), w = 2 * kPi * f / (float) sr, c = std::cos (w), s = std::sin (w);
        const float al = s / 2 * std::sqrt ((A + 1 / A) * (1 / S - 1) + 2), sa = 2 * std::sqrt (A) * al;
        set (A * ((A + 1) - (A - 1) * c + sa), 2 * A * ((A - 1) - (A + 1) * c), A * ((A + 1) - (A - 1) * c - sa), (A + 1) + (A - 1) * c + sa, -2 * ((A - 1) + (A + 1) * c), (A + 1) + (A - 1) * c - sa);
    }
    void highShelf (double sr, float f, float db, float S = 0.8f)
    {
        f = fc (sr, f); const float A = std::pow (10.0f, db / 40.0f), w = 2 * kPi * f / (float) sr, c = std::cos (w), s = std::sin (w);
        const float al = s / 2 * std::sqrt ((A + 1 / A) * (1 / S - 1) + 2), sa = 2 * std::sqrt (A) * al;
        set (A * ((A + 1) + (A - 1) * c + sa), -2 * A * ((A - 1) + (A + 1) * c), A * ((A + 1) + (A - 1) * c - sa), (A + 1) - (A - 1) * c + sa, 2 * ((A - 1) - (A + 1) * c), (A + 1) - (A - 1) * c - sa);
    }
};

// Linkwitz–Riley 4th order (two cascaded Butterworth) — LP + HP sum flat in magnitude
struct LR4
{
    Biquad lp1, lp2, hp1, hp2;
    void set (double sr, float f) { lp1.lowpass (sr, f); lp2.lowpass (sr, f); hp1.highpass (sr, f); hp2.highpass (sr, f); }
    void clear() { lp1.clear(); lp2.clear(); hp1.clear(); hp2.clear(); }
    inline void split (float x, float& lo, float& hi) noexcept { lo = lp2.process (lp1.process (x)); hi = hp2.process (hp1.process (x)); }
};

// ───────────────────────────── Universes (character presets) ─────────────────────────────
struct Universe
{
    const char* en; const char* es; const char* tagEn; const char* tagEs;
    float low, mid, high, air;           // dB at amount = 1
    float glue;                          // multiband compression depth 0..1
    float attackMs, releaseMs;
    float punch, harmonics, width, clip, tame; // 0..1 (width: 0..1 → +0..+40 %)
    uint32_t colour;
};
inline const Universe kUniverses[] = {
    { "Nebula", "Nebulosa", "Warm glue, smooth top", "Pegamento cálido, agudos suaves", 1.6f, 0.0f, 0.8f, 1.2f, 0.55f, 18.f, 160.f, 0.15f, 0.45f, 0.20f, 0.20f, 0.45f, 0xffb67cff },
    { "Pulsar", "Púlsar", "Punch and groove", "Golpe y groove",              2.2f, -0.6f, 1.4f, 1.0f, 0.60f, 32.f, 110.f, 0.60f, 0.35f, 0.15f, 0.45f, 0.35f, 0xff58e1ff },
    { "Quasar", "Cuásar", "Bright, wide, modern", "Brillante, ancho, moderno", 2.6f, -1.0f, 2.2f, 2.6f, 0.70f, 22.f, 120.f, 0.35f, 0.50f, 0.40f, 0.45f, 0.30f, 0xffffd166 },
    { "Supernova", "Supernova", "Loud and dense", "Fuerte y denso",          2.2f, -0.4f, 1.8f, 1.8f, 0.85f, 15.f, 90.f, 0.40f, 0.60f, 0.25f, 0.75f, 0.40f, 0xffff5a4e },
    { "Event Horizon", "Horizonte", "Dark, vintage, heavy", "Oscuro, vintage, pesado", 2.4f, 0.4f, -1.4f, -0.6f, 0.65f, 25.f, 180.f, 0.25f, 0.70f, 0.10f, 0.35f, 0.70f, 0xffff4fd8 },
    { "Pure Light", "Luz pura", "Transparent mastering", "Mastering transparente", 0.0f, 0.0f, 0.0f, 0.3f, 0.30f, 30.f, 200.f, 0.0f, 0.05f, 0.0f, 0.10f, 0.25f, 0xffe8ecff },
};
inline constexpr int kNumUniverses = (int) (sizeof (kUniverses) / sizeof (kUniverses[0]));

// Settings the processor computes once per block from parameters
struct Settings
{
    int   universe = 0;
    float amount = 0.5f;
    float inputDb = 0, outputDb = 0;
    float low = 0, mid = 0, high = 0, air = 0;            // user offsets dB
    float tame = 0, punch = 0, glue = 0, harmonics = 0, width = 0, clip = 0; // user offsets -1..1
    float bassMonoHz = 0;                                 // 0 = off
    float ceilingDb = -1.0f, driveDb = 0;
    float match = 0;
    std::array<float, 10> matchDb {};                    // reference match curve (dB per octave band)
};

constexpr int kMaxCh = 16;
inline constexpr float kOct[10] = { 31.5f, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000 };

// ───────────────────────────── Loudness (ITU-R BS.1770 / EBU R128) ─────────────────────────────
struct LoudnessMeter
{
    double sr = 48000; int numCh = 2;
    Biquad pre[kMaxCh], rlb[kMaxCh];
    std::vector<double> blocks;          // 100 ms block mean-square sums (weighted, summed channels)
    double acc = 0; int accN = 0, blockLen = 4800;
    std::vector<float> weights;
    std::atomic<float> momentary { -100.f }, shortTerm { -100.f };
    std::deque<double> hist;             // block powers for integrated (message thread copies)
    juce::SpinLock histLock;
    std::vector<double> histAudio; int histCount = 0;

    void prepare (double s, int ch, const juce::AudioChannelSet& set)
    {
        sr = s; numCh = juce::jmin (ch, kMaxCh);
        // K-weighting (approximation of BS.1770 at any rate): high shelf +4 dB @1.68k, HPF 38 Hz
        // exact BS.1770-4 K-weighting at any sample rate (bilinear design, as in libebur128)
        {
            double f0 = 1681.974450955533, G = 3.999843853973347, Q = 0.7071752369554196;
            double K = std::tan (juce::MathConstants<double>::pi * f0 / sr), Vh = std::pow (10.0, G / 20.0), Vb = std::pow (Vh, 0.4996667741545416);
            double a0 = 1.0 + K / Q + K * K;
            const float pb0 = (float) ((Vh + Vb * K / Q + K * K) / a0), pb1 = (float) (2.0 * (K * K - Vh) / a0), pb2 = (float) ((Vh - Vb * K / Q + K * K) / a0);
            const float pa1 = (float) (2.0 * (K * K - 1.0) / a0), pa2 = (float) ((1.0 - K / Q + K * K) / a0);
            f0 = 38.13547087602444; Q = 0.5003270373238773; K = std::tan (juce::MathConstants<double>::pi * f0 / sr);
            a0 = 1.0 + K / Q + K * K;
            const float ra1 = (float) (2.0 * (K * K - 1.0) / a0), ra2 = (float) ((1.0 - K / Q + K * K) / a0);
            for (int c = 0; c < numCh; ++c)
            {
                pre[c].set (pb0, pb1, pb2, 1.0f, pa1, pa2); rlb[c].set (1.0f, -2.0f, 1.0f, 1.0f, ra1, ra2);
                pre[c].clear(); rlb[c].clear();
            }
        }
        blockLen = (int) (sr * 0.1); blocks.assign (30, 0.0); acc = 0; accN = 0;
        weights.assign ((size_t) numCh, 1.0f);
        using CS = juce::AudioChannelSet;
        for (int c = 0; c < numCh && c < set.size(); ++c)
        {
            const auto t = set.getTypeOfChannel (c);
            if (t == CS::LFE || t == CS::LFE2) weights[(size_t) c] = 0.0f;
            else if (t == CS::leftSurround || t == CS::rightSurround || t == CS::leftSurroundSide || t == CS::rightSurroundSide
                     || t == CS::leftSurroundRear || t == CS::rightSurroundRear) weights[(size_t) c] = 1.41f;
        }
        histAudio.assign (36000, 0.0); histCount = 0;
        { const juce::SpinLock::ScopedLockType sl (histLock); hist.clear(); }
        momentary = -100.f; shortTerm = -100.f;
    }
    static float toLufs (double ms) { return ms <= 1e-12 ? -100.0f : (float) (-0.691 + 10.0 * std::log10 (ms)); }

    void process (const float* const* ch, int n)
    {
        for (int i = 0; i < n; ++i)
        {
            double s = 0;
            for (int c = 0; c < numCh; ++c) { const float y = rlb[c].process (pre[c].process (ch[c][i])); s += (double) weights[(size_t) c] * y * y; }
            acc += s; ++accN;
            if (accN >= blockLen)
            {
                const double ms = acc / accN; acc = 0; accN = 0;
                std::rotate (blocks.begin(), blocks.begin() + 1, blocks.end()); blocks.back() = ms;
                double m4 = 0; for (size_t k = blocks.size() - 4; k < blocks.size(); ++k) m4 += blocks[k];
                double m30 = 0; for (auto b : blocks) m30 += b;
                momentary = toLufs (m4 / 4.0); shortTerm = toLufs (m30 / 30.0);
                const juce::SpinLock::ScopedTryLockType tl (histLock);
                if (tl.isLocked()) { hist.push_back (m4 / 4.0); if (hist.size() > 36000) hist.pop_front(); }
            }
        }
    }
    // integrated loudness with absolute (-70) and relative (-10 LU) gates — message thread
    float integrated()
    {
        std::vector<double> h;
        { const juce::SpinLock::ScopedLockType sl (histLock); h.assign (hist.begin(), hist.end()); }
        double s = 0; int n = 0;
        for (auto v : h) if (toLufs (v) > -70.0f) { s += v; ++n; }
        if (n == 0) return -100.0f;
        const float rel = toLufs (s / n) - 10.0f;
        s = 0; n = 0;
        for (auto v : h) if (toLufs (v) > -70.0f && toLufs (v) > rel) { s += v; ++n; }
        return n == 0 ? -100.0f : toLufs (s / n);
    }
    // loudness range (EBU Tech 3342, from 3 s windows approximated by momentary history)
    float range()
    {
        std::vector<double> h;
        { const juce::SpinLock::ScopedLockType sl (histLock); h.assign (hist.begin(), hist.end()); }
        std::vector<float> st;
        for (size_t i = 30; i < h.size(); i += 10) { double m = 0; for (size_t k = i - 30; k < i; ++k) m += h[k]; st.push_back (toLufs (m / 30)); }
        std::vector<float> g; for (auto v : st) if (v > -70) g.push_back (v);
        if (g.size() < 4) return 0;
        double s = 0; for (auto v : g) s += std::pow (10.0, v / 10.0);
        const float rel = (float) (10 * std::log10 (s / g.size())) - 20.0f;
        std::vector<float> r; for (auto v : g) if (v > rel) r.push_back (v);
        if (r.size() < 4) return 0;
        std::sort (r.begin(), r.end());
        return r[(size_t) (0.95 * (r.size() - 1))] - r[(size_t) (0.10 * (r.size() - 1))];
    }
    void resetIntegrated() { const juce::SpinLock::ScopedLockType sl (histLock); hist.clear(); }
};

// ───────────────────────────── The OMEGA chain ─────────────────────────────
class Engine
{
public:
    // live analysis (read by UI)
    std::atomic<float> grBand[3] { {0}, {0}, {0} };  // dB (<=0)
    std::atomic<float> grLimit { 0 }, clipAct { 0 }, satAct { 0 }, tameAct { 0 }, punchAct { 0 }, widthNow { 1 }, corr { 1 };

    void prepare (double sampleRate, int maxBlock, int channels, int oversampleFactorLog2, const juce::AudioChannelSet& set)
    {
        sr = sampleRate; numCh = juce::jlimit (1, kMaxCh, channels); osLog2 = oversampleFactorLog2;
        lfe = -1;
        for (int c = 0; c < set.size() && c < numCh; ++c)
            if (set.getTypeOfChannel (c) == juce::AudioChannelSet::LFE) lfe = c;
        // L/R pair for stereo processing
        lch = 0; rch = numCh > 1 ? 1 : 0;
        oversampler = std::make_unique<juce::dsp::Oversampling<float>> ((size_t) numCh, (size_t) osLog2,
                          juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true);
        oversampler->initProcessing ((size_t) maxBlock);
        osRate = sr * (double) (1 << osLog2);
        // lookahead limiter at the oversampled rate (1.5 ms)
        laLen = juce::jmax (4, (int) std::round (0.0015 * osRate));
        laDelay.assign ((size_t) numCh, std::vector<float> ((size_t) laLen + 2, 0.0f));
        laPos = 0;
        minQ.assign ((size_t) laLen * 4 + 8, { 0, 1.0f }); qHead = qTail = 0; idx = 0;
        boxBuf.assign ((size_t) laLen, 1.0f); boxSum = (double) laLen; boxPos = 0;
        limGain = 1.0f; limEnv = 1.0f;
        for (auto& x : xo1) x.clear();
        for (auto& x : xo2) x.clear();
        for (auto& x : ap) x.clear();
        for (int c = 0; c < kMaxCh; ++c) { xo1[c].set (sr, 150.0f); xo2[c].set (sr, 3000.0f); ap[c].set (sr, 3000.0f); }
        for (auto& b : bandEnv) b = 0;
        for (auto& b : bandGr) b = 0;
        for (int c = 0; c < kMaxCh; ++c) { dcBlock[c].highpass (osRate, 8.0f, 0.7071f); dcBlock[c].clear(); sideHp[0].clear(); }
        tameDet.bandpass (sr, 4200.0f, 0.9f); tameDet.clear(); broadDet = tameEnv = 0; tameGrDb = 0;
        sideHp[0].highpass (sr, 120.0f); sideHp[1].highpass (sr, 120.0f);
        punchFast = punchSlow = 0;
        dryDelay.assign ((size_t) numCh, std::vector<float> (16384, 0.0f)); dryPos = 0;
        tmp.setSize (numCh, maxBlock * 2);
        last = {}; force = true;
        corrN = 0; corrD1 = corrD2 = 1e-9f;
        updateLatency();
    }

    int getLatency() const noexcept { return latency; }
    double getOsRate() const noexcept { return osRate; }

    void process (juce::AudioBuffer<float>& buf, const Settings& s)
    {
        const int n = buf.getNumSamples();
        const int nc = juce::jmin (numCh, buf.getNumChannels());
        if (n == 0) return;
        updateCoeffs (s);
        const auto& U = kUniverses[juce::jlimit (0, kNumUniverses - 1, s.universe)];
        const float a = s.amount;

        // keep the dry signal (for level-matched bypass / delta)
        for (int c = 0; c < nc; ++c)
        {
            auto& d = dryDelay[(size_t) c]; const float* x = buf.getReadPointer (c);
            for (int i = 0; i < n; ++i) d[(size_t) ((dryPos + i) & 16383)] = x[i];
        }

        // ── 1. input gain + tone EQ (+ reference match) ──
        const float inG = dbToGain (s.inputDb);
        for (int c = 0; c < nc; ++c)
        {
            float* x = buf.getWritePointer (c);
            if (c == lfe) { juce::FloatVectorOperations::multiply (x, inG, n); continue; }
            for (int i = 0; i < n; ++i)
            {
                float v = x[i] * inG;
                v = eqLow[c].process (v); v = eqMid[c].process (v); v = eqHigh[c].process (v); v = eqAir[c].process (v);
                if (matchOn) for (int b = 0; b < 10; ++b) v = eqMatch[c][b].process (v);
                x[i] = v;
            }
        }

        // ── 2. tame (dynamic de-harsh 2.5–7 kHz), punch (transient shaper), 3-band glue ──
        const float tameAmt = sat01 (U.tame * (0.4f + 0.6f * a) + s.tame * 0.5f);
        const float punchAmt = juce::jlimit (0.0f, 1.0f, U.punch * a + s.punch * 0.5f);
        const float glueAmt = juce::jlimit (0.0f, 1.0f, U.glue * (0.3f + 0.7f * a) + s.glue * 0.4f);
        const float atkC = std::exp (-1.0f / (U.attackMs * 0.001f * (float) sr));
        const float relC = std::exp (-1.0f / (U.releaseMs * 0.001f * (float) sr));
        const float detC = std::exp (-1.0f / (0.008f * (float) sr));
        const float pfA = std::exp (-1.0f / (0.0008f * (float) sr)), pfR = std::exp (-1.0f / (0.05f * (float) sr));
        const float psA = std::exp (-1.0f / (0.02f * (float) sr)), psR = std::exp (-1.0f / (0.2f * (float) sr));
        const float tA = std::exp (-1.0f / (0.004f * (float) sr)), tR = std::exp (-1.0f / (0.08f * (float) sr));
        static constexpr float thrBase[3] = { -18.0f, -21.0f, -25.0f };
        static constexpr float ratioMax[3] = { 3.0f, 2.6f, 2.8f };
        float bandGrMin[3] { 0, 0, 0 }, tameMax = 0, punchMax = 0;
        float* ch[kMaxCh] {}; for (int c = 0; c < nc; ++c) ch[c] = buf.getWritePointer (c);

        for (int i = 0; i < n; ++i)
        {
            // linked detection on the max of L/R (all non-LFE channels for surround)
            float peak = 0, tameIn = 0;
            for (int c = 0; c < nc; ++c) if (c != lfe) peak = std::max (peak, std::abs (ch[c][i]));
            // punch
            const float pIn = peak;
            punchFast = pIn > punchFast ? pfA * punchFast + (1 - pfA) * pIn : pfR * punchFast + (1 - pfR) * pIn;
            punchSlow = pIn > punchSlow ? psA * punchSlow + (1 - psA) * pIn : psR * punchSlow + (1 - psR) * pIn;
            float pg = 1.0f;
            if (punchAmt > 0.001f && punchSlow > 1e-5f)
            {
                const float ratio = punchFast / punchSlow;
                pg = juce::jlimit (0.7f, 1.8f, std::pow (ratio, 0.7f * punchAmt));
                punchMax = std::max (punchMax, gainToDb (pg));
            }
            // tame detector on the mono sum
            float mono = 0; for (int c = 0; c < nc; ++c) if (c != lfe) mono += ch[c][i];
            mono /= (float) juce::jmax (1, nc - (lfe >= 0 ? 1 : 0));
            const float th = tameDet.process (mono);
            tameEnv = th * th > tameEnv ? tA * tameEnv + (1 - tA) * th * th : tR * tameEnv + (1 - tR) * th * th;
            broadDet = mono * mono > broadDet ? tA * broadDet + (1 - tA) * mono * mono : tR * broadDet + (1 - tR) * mono * mono;
            tameIn = 10.0f * std::log10 ((tameEnv + 1e-12f) / (broadDet + 1e-12f)); // band-to-broadband ratio (dB)
            const float over = tameIn + 9.0f;                                             // harsh when 2.5–7k is within 9 dB of the full band
            const float tgt = over > 0 ? -juce::jmin (6.0f, over * 0.8f) * tameAmt : 0.0f;
            tameGrDb += 0.002f * (tgt - tameGrDb);
            tameMax = std::min (tameMax, tameGrDb);
            if (std::abs (tameGrDb - tameApplied) > 0.1f) { tameApplied = tameGrDb; for (int c = 0; c < nc; ++c) tameEq[c].peak (sr, 4200.0f, 0.8f, tameApplied); }

            // per-channel split into 3 bands
            float lo[kMaxCh], mi[kMaxCh], hi[kMaxCh];
            float bpk[3] { 0, 0, 0 };
            for (int c = 0; c < nc; ++c)
            {
                float v = ch[c][i] * pg;
                v = tameEq[c].process (v);
                if (c == lfe) { lo[c] = v; mi[c] = hi[c] = 0; continue; }
                float l, rest; xo1[c].split (v, l, rest);
                float m, h; xo2[c].split (rest, m, h);
                float la, lb; ap[c].split (l, la, lb); l = la + lb;   // phase-align the low band
                lo[c] = l; mi[c] = m; hi[c] = h;
                bpk[0] = std::max (bpk[0], std::abs (l)); bpk[1] = std::max (bpk[1], std::abs (m)); bpk[2] = std::max (bpk[2], std::abs (h));
            }
            float bg[3];
            for (int b = 0; b < 3; ++b)
            {
                bandEnv[b] = detC * bandEnv[b] + (1 - detC) * bpk[b] * bpk[b];
                const float lvl = 10.0f * std::log10 (bandEnv[b] + 1e-12f);
                const float thr = thrBase[b] + (1.0f - glueAmt) * 14.0f;
                const float ratio = 1.0f + (ratioMax[b] - 1.0f) * glueAmt;
                const float knee = 8.0f, ov = lvl - thr;
                float gr = 0;
                if (ov > knee * 0.5f) gr = -(ov) * (1 - 1 / ratio);
                else if (ov > -knee * 0.5f) gr = -(1 - 1 / ratio) * (ov + knee * 0.5f) * (ov + knee * 0.5f) / (2 * knee);
                // program-dependent release: faster when the band is far over threshold
                const float rc = gr < bandGr[b] ? atkC : (relC * (1.0f - 0.4f * sat01 (-bandGr[b] / 12.0f)));
                bandGr[b] = rc * bandGr[b] + (1 - rc) * gr;
                bandGrMin[b] = std::min (bandGrMin[b], bandGr[b]);
                const float makeup = (1 - 1 / ratio) * juce::jmax (0.0f, -thr - 12.0f) * 0.18f * glueAmt;
                bg[b] = dbToGain (bandGr[b] + makeup);
            }
            for (int c = 0; c < nc; ++c) ch[c][i] = c == lfe ? lo[c] : lo[c] * bg[0] + mi[c] * bg[1] + hi[c] * bg[2];
        }
        for (int b = 0; b < 3; ++b) grBand[b].store (bandGrMin[b]);
        tameAct.store (tameMax); punchAct.store (punchMax);

        // ── 3. stereo: width on the sides above ~120 Hz, mono bass, correlation guard ──
        const float wAmt = juce::jlimit (-1.0f, 1.0f, U.width * a + s.width * 0.6f);
        float widthTarget = 1.0f + 0.4f * wAmt;
        if (nc >= 2)
        {
            float* L = ch[lch]; float* R = ch[rch];
            for (int i = 0; i < n; ++i)
            {
                const float l = L[i], r = R[i];
                corrN = l * r + 0.9997f * (corrN - l * r);
                corrD1 = l * l + 0.9997f * (corrD1 - l * l);
                corrD2 = r * r + 0.9997f * (corrD2 - r * r);
            }
            const float cc = corrN / std::sqrt (corrD1 * corrD2 + 1e-12f);
            corr.store (cc);
            // guard: never let the widener push correlation below ~0.15
            if (cc < 0.35f) widthTarget = std::min (widthTarget, 1.0f + 0.4f * wAmt * sat01 ((cc - 0.15f) / 0.2f));
            for (int i = 0; i < n; ++i)
            {
                widthSm += 0.0005f * (widthTarget - widthSm);
                const float mid = 0.5f * (L[i] + R[i]); float side = 0.5f * (L[i] - R[i]);
                const float sideHi = sideHp[0].process (side);
                const float sideLo = side - sideHi;
                side = (monoBassOn ? 0.0f : sideLo) + sideHi * widthSm;
                L[i] = mid + side; R[i] = mid - side;
            }
            widthNow.store (widthSm);
        }

        // ── 4. oversampled harmonics + clipper + true-peak-aware lookahead limiter ──
        const float harm = juce::jlimit (0.0f, 1.0f, U.harmonics * a + s.harmonics * 0.5f);
        const float clipAmt = juce::jlimit (0.0f, 1.0f, U.clip * a + s.clip * 0.5f);
        const float drive = dbToGain (s.driveDb);
        const float ceiling = dbToGain (s.ceilingDb - 0.25f);              // margin for the downsampling filter
        const float clipThr = ceiling * (1.0f + (1.0f - clipAmt) * 1.2f);
        juce::dsp::AudioBlock<float> block (buf.getArrayOfWritePointers(), (size_t) nc, (size_t) n);
        auto os = oversampler->processSamplesUp (block);
        const int on = (int) os.getNumSamples();
        float satMax = 0, clipMax = 0, grMin = 0;
        const float k = 1.0f + 3.5f * harm;
        const float limRel = std::exp (-1.0f / (0.12f * (float) osRate));
        const float limRelFast = std::exp (-1.0f / (0.035f * (float) osRate));
        for (int i = 0; i < on; ++i)
        {
            float pk = 0;
            for (int c = 0; c < nc; ++c)
            {
                float v = os.getSample (c, i) * drive;
                if (harm > 0.001f && c != lfe)
                {
                    // tape/tube blend: asymmetric soft saturation, level-compensated
                    const float sh = std::tanh (k * (v + 0.08f * harm * v * v)) / k;
                    const float wet = 0.35f + 0.65f * harm;
                    const float y = v + wet * (sh - v);
                    satMax = std::max (satMax, std::abs (y - v));
                    v = dcBlock[c].process (y);
                }
                // soft clipper
                const float av = std::abs (v);
                if (av > clipThr * 0.7f)
                {
                    const float knee0 = clipThr * 0.7f, span = clipThr - knee0;
                    const float t = (av - knee0) / span;
                    const float out = knee0 + span * std::tanh (t);
                    clipMax = std::max (clipMax, av - out);
                    v = std::copysign (out, v);
                }
                os.setSample (c, i, v);
                pk = std::max (pk, std::abs (v));
            }
            // lookahead limiter: required gain → sliding window min → box smoothing (no overshoot)
            const float req = pk > ceiling ? ceiling / pk : 1.0f;
            while (qTail > qHead && minQ[(size_t) ((qTail - 1) % minQ.size())].second >= req) --qTail;
            minQ[(size_t) (qTail % minQ.size())] = { idx, req }; ++qTail;
            while (minQ[(size_t) (qHead % minQ.size())].first <= idx - laLen - 1) ++qHead;
            const float wmin = minQ[(size_t) (qHead % minQ.size())].second;
            ++idx;
            float tgt = wmin;
            // release: adaptive (long GR → slower)
            limEnv = tgt < limEnv ? tgt : (limEnv * (limEnv < 0.8f ? limRel : limRelFast) + (1 - (limEnv < 0.8f ? limRel : limRelFast)) * tgt);
            boxSum += (double) limEnv - (double) boxBuf[(size_t) boxPos]; boxBuf[(size_t) boxPos] = limEnv; boxPos = (boxPos + 1) % laLen;
            const float g = (float) (boxSum / laLen);
            grMin = std::min (grMin, gainToDb (g));
            for (int c = 0; c < nc; ++c)
            {
                auto& d = laDelay[(size_t) c];
                const float delayed = d[(size_t) laPos];
                d[(size_t) laPos] = os.getSample (c, i);
                os.setSample (c, i, delayed * g);
            }
            laPos = (laPos + 1) % laLen;
        }
        oversampler->processSamplesDown (block);
        satAct.store (satMax); clipAct.store (gainToDb (1.0f + clipMax)); grLimit.store (grMin);

        // ── 5. output trim + final sample-peak safety at the ceiling ──
        const float outG = dbToGain (s.outputDb);
        const float hard = dbToGain (s.ceilingDb);
        for (int c = 0; c < nc; ++c)
        {
            float* x = buf.getWritePointer (c);
            for (int i = 0; i < n; ++i) x[i] = juce::jlimit (-hard, hard, x[i] * outG);
        }
        dryPos = (dryPos + n) & 16383;
    }

    // dry signal aligned with the processed output (same latency)
    void readDry (juce::AudioBuffer<float>& dst, int n)
    {
        const int nc = juce::jmin (numCh, dst.getNumChannels());
        const int start = dryPos - n - latency;
        for (int c = 0; c < nc; ++c)
        {
            auto& d = dryDelay[(size_t) c]; float* y = dst.getWritePointer (c);
            for (int i = 0; i < n; ++i) y[i] = d[(size_t) ((start + i) & 16383)];
        }
    }

private:
    void updateLatency()
    {
        latency = (int) std::round (oversampler->getLatencyInSamples() + (double) laLen / (double) (1 << osLog2));
    }

    void updateCoeffs (const Settings& s)
    {
        const auto& U = kUniverses[juce::jlimit (0, kNumUniverses - 1, s.universe)];
        const float a = s.amount;
        const float low = U.low * a + s.low, mid = U.mid * a + s.mid, high = U.high * a + s.high, air = U.air * a + s.air;
        const bool changed = force || std::abs (low - last.low) > 0.01f || std::abs (mid - last.mid) > 0.01f || std::abs (high - last.high) > 0.01f
                          || std::abs (air - last.air) > 0.01f;
        if (changed)
            for (int c = 0; c < numCh; ++c)
            { eqLow[c].lowShelf (sr, 90.0f, low); eqMid[c].peak (sr, 1100.0f, 0.6f, mid); eqHigh[c].highShelf (sr, 5500.0f, high); eqAir[c].highShelf (sr, 14000.0f, air, 1.0f); }
        last.low = low; last.mid = mid; last.high = high; last.air = air;

        bool mChanged = force || std::abs (s.match - lastMatch) > 0.005f;
        for (int b = 0; b < 10; ++b) if (std::abs (s.matchDb[(size_t) b] - lastMatchDb[(size_t) b]) > 0.02f) mChanged = true;
        if (mChanged)
        {
            matchOn = s.match > 0.005f;
            for (int c = 0; c < numCh; ++c)
                for (int b = 0; b < 10; ++b) eqMatch[c][b].peak (sr, kOct[b], 1.1f, juce::jlimit (-4.0f, 4.0f, s.matchDb[(size_t) b]) * s.match);
            lastMatch = s.match; lastMatchDb = s.matchDb;
        }
        const bool monoBass = s.bassMonoHz >= 20.0f;
        if (force || std::abs (s.bassMonoHz - lastMono) > 0.5f)
        {
            sideHp[0].highpass (sr, monoBass ? s.bassMonoHz : 120.0f);
            lastMono = s.bassMonoHz;
        }
        monoBassOn = monoBass;
        monoSubOn = false;
        force = false;
    }

    double sr = 48000, osRate = 192000;
    int numCh = 2, osLog2 = 2, lfe = -1, lch = 0, rch = 1, latency = 0;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    Biquad eqLow[kMaxCh], eqMid[kMaxCh], eqHigh[kMaxCh], eqAir[kMaxCh], eqMatch[kMaxCh][10], subHp[kMaxCh], tameEq[kMaxCh], dcBlock[kMaxCh];
    Biquad tameDet, sideHp[2];
    LR4 xo1[kMaxCh], xo2[kMaxCh], ap[kMaxCh];
    float bandEnv[3] {}, bandGr[3] {};
    float tameEnv = 0, broadDet = 0, tameGrDb = 0, tameApplied = 0;
    float punchFast = 0, punchSlow = 0;
    float widthSm = 1.0f, corrN = 0, corrD1 = 1e-9f, corrD2 = 1e-9f;
    bool monoBassOn = false, monoSubOn = false, matchOn = false, force = true;
    Settings last; float lastMatch = -1, lastMono = -1; std::array<float, 10> lastMatchDb {};
    // limiter
    int laLen = 64, laPos = 0;
    std::vector<std::vector<float>> laDelay;
    std::vector<std::pair<long long, float>> minQ; long long qHead = 0, qTail = 0, idx = 0;
    std::vector<float> boxBuf; double boxSum = 0; int boxPos = 0;
    float limGain = 1, limEnv = 1;
    // dry
    std::vector<std::vector<float>> dryDelay; int dryPos = 0;
    juce::AudioBuffer<float> tmp;
};
} // namespace om
