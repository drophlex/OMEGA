// OMEGA offline test: loudness accuracy, ceiling, assistants, level-matched compare, surround render, licence, speed.
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstdio>
using namespace om;

struct Head : juce::AudioPlayHead
{
    bool play = true; juce::int64 pos = 0;
    juce::Optional<PositionInfo> getPosition() const override { PositionInfo i; i.setIsPlaying (play); i.setTimeInSamples (pos); return i; }
};
static void setP (OmegaProcessor& p, const char* id, float v) { auto* q = p.apvts.getParameter (id); q->setValueNotifyingHost (q->convertTo0to1 (v)); }
static void pump (int ms) { juce::MessageManager::getInstance()->runDispatchLoopUntil (ms); }

// a "mix": kick, bass, hats, chords, noise bed — fixed seed
struct Mix
{
    double t = 0, sr = 48000; juce::Random r { 3 };
    float lpL = 0, lpR = 0;
    void fill (juce::AudioBuffer<float>& b, float gain)
    {
        for (int i = 0; i < b.getNumSamples(); ++i)
        {
            const double beat = std::fmod (t * 2.0, 1.0);
            const float kick = (float) (std::sin (2 * M_PI * (50 + 90 * std::exp (-beat * 30)) * beat) * std::exp (-beat * 7));
            const float bass = (float) (0.35 * std::sin (2 * M_PI * 55 * t) * (0.6 + 0.4 * std::sin (2 * M_PI * 0.5 * t)));
            const double h = std::fmod (t * 8.0, 1.0);
            const float hat = (r.nextFloat() * 2 - 1) * (float) std::exp (-h * 40) * 0.25f;
            const float ch = (float) (0.12 * (std::sin (2 * M_PI * 329.6 * t) + std::sin (2 * M_PI * 392 * t) + std::sin (2 * M_PI * 493.9 * t)));
            lpL += 0.05f * ((r.nextFloat() * 2 - 1) - lpL); lpR += 0.05f * ((r.nextFloat() * 2 - 1) - lpR);
            b.setSample (0, i, gain * (kick + bass + hat + ch * 0.9f + 0.2f * lpL));
            b.setSample (1, i, gain * (kick + bass + hat * 0.6f + ch * 1.1f + 0.2f * lpR));
            t += 1.0 / sr;
        }
    }
};

int main()
{
    juce::ScopedJuceInitialiser_GUI gui;
    int fails = 0;
    auto check = [&] (bool ok, const char* what) { std::printf ("%s %s\n", ok ? "[ok]  " : "[FAIL]", what); if (! ok) ++fails; };
    const double sr = 48000; const int bs = 512;

    // 1. loudness meter accuracy: 997 Hz sine at -20 dBFS stereo → -20 LUFS (+ -3.01 per channel + summing = -20.0)
    {
        LoudnessMeter m; m.prepare (sr, 2, juce::AudioChannelSet::stereo());
        juce::AudioBuffer<float> b (2, bs); double ph = 0;
        for (int k = 0; k < 400; ++k) { for (int i = 0; i < bs; ++i) { const float v = 0.1f * (float) std::sin (ph); ph += 2 * M_PI * 997 / sr; b.setSample (0, i, v); b.setSample (1, i, v); } m.process (b.getArrayOfReadPointers(), bs); }
        std::printf ("        997 Hz @ -20 dBFS stereo: short-term %.2f LUFS, integrated %.2f\n", m.shortTerm.load(), m.integrated());
        check (std::abs (m.integrated() - (-20.0f)) < 0.3f, "BS.1770 meter reads -20 LUFS (±0.3)");
    }

    OmegaProcessor p; Head head; p.setPlayHead (&head);
    p.setPlayConfigDetails (2, 2, sr, bs); p.prepareToPlay (sr, bs);
    std::printf ("        latency: %d samples (%.2f ms)\n", p.getLatencySamples(), p.getLatencySamples() * 1000.0 / sr);
    check (p.getLatencySamples() > 0 && p.getLatencySamples() < 400, "latency reported and small");

    Mix mix; juce::AudioBuffer<float> b (2, bs); juce::MidiBuffer midi;
    auto run = [&] (int blocks, float g, std::function<void()> each = {})
    {
        float mx = 0; bool finite = true;
        for (int k = 0; k < blocks; ++k)
        {
            mix.fill (b, g); p.processBlock (b, midi); head.pos += bs;
            for (int c = 0; c < 2; ++c) for (int i = 0; i < bs; ++i) { const float v = b.getSample (c, i); if (! std::isfinite (v)) finite = false; mx = std::max (mx, std::abs (v)); }
            if (k % 8 == 0) { pump (1); if (each) each(); }
        }
        return std::make_pair (mx, finite);
    };

    // 2. assistants: auto input then match target −14
    p.startAutoInput();
    for (int k = 0; k < 220 && (k < 5 || p.isAutoInput()); ++k) { run (6, 0.25f); pump (34); }
    std::printf ("        auto input set to %.1f dB (input momentary ≈ %.1f LUFS)\n", p.apvts.getRawParameterValue ("input")->load(), p.meterIn.momentary.load() + p.apvts.getRawParameterValue ("input")->load());
    for (int u : { 0, 1, 2, 3, 4, 5 })
    {
        setP (p, "universe", (float) u); setP (p, "amount", 0.7f); setP (p, "drive", 0.0f);
        p.startLoudnessMatch();
        for (int k = 0; k < 280 && p.isLoudMatch(); ++k) { run (6, 0.25f); pump (34); }
        auto res = run (400, 0.25f);
        p.resetMeters(); run (600, 0.25f);
        const float integ = p.getIntegrated();
        std::printf ("        %-14s drive %.1f dB → integrated %.2f LUFS, true peak max %.2f dBTP, sample peak %.2f dBFS\n", kUniverses[u].en,
                     p.apvts.getRawParameterValue ("drive")->load(), integ, p.truePeakMax.load(), gainToDb (res.first));
        check (res.second, "finite output");
        check (std::abs (integ - (-14.0f)) < 1.2f, "  loudness assistant lands on −14 LUFS (±1.2)");
        check (p.truePeakMax.load() <= -1.0f + 0.35f, "  true peak stays at the −1 dBTP ceiling (≤ −0.65)");
        check (gainToDb (res.first) <= -1.0f + 0.001f, "  sample peak never above the ceiling");
    }

    // 3. level-matched compare
    {
        const float before = p.meterOut.shortTerm.load();
        setP (p, "compare", 1.0f); run (300, 0.25f);
        // measure what the user hears
        LoudnessMeter heard; heard.prepare (sr, 2, juce::AudioChannelSet::stereo());
        for (int k = 0; k < 300; ++k) { mix.fill (b, 0.25f); p.processBlock (b, midi); heard.process (b.getArrayOfReadPointers(), bs); if (k % 8 == 0) pump (1); }
        std::printf ("        compare: OMEGA %.1f LUFS vs bypass heard %.1f LUFS\n", before, heard.shortTerm.load());
        check (std::abs (heard.shortTerm.load() - before) < 1.0f, "COMPARE plays the dry mix at the same loudness (±1 LU)");
        setP (p, "compare", 0.0f);
    }

    // 4. speed
    {
        const auto t0 = juce::Time::getMillisecondCounterHiRes();
        run (2000, 0.25f);
        const double ms = juce::Time::getMillisecondCounterHiRes() - t0;
        const double audioMs = 2000.0 * bs * 1000.0 / sr;
        std::printf ("        CPU: %.1f ms for %.0f ms of audio (%.1f%% of one core, 4x oversampling)\n", ms, audioMs, 100.0 * ms / audioMs);
        check (ms / audioMs < 0.25, "realtime-safe CPU (< 25% of one core in a debug-ish build)");
    }

    // 5. surround file mastering (CONSTELLATION 7.1.4 export)
    {
        juce::File src ("/home/claude/constellation/build/test_7.1.4.wav");
        if (src.existsAsFile())
        {
            p.renderFile (src);
            for (int k = 0; k < 400 && p.renderProgress.load() < 1.5f; ++k) pump (25);
            juce::WavAudioFormat w;
            std::unique_ptr<juce::AudioFormatReader> a (w.createReaderFor (src.createInputStream().release(), true));
            std::unique_ptr<juce::AudioFormatReader> o (w.createReaderFor (juce::File (p.renderResult).createInputStream().release(), true));
            check (o != nullptr && a != nullptr && o->numChannels == 12 && o->lengthInSamples == a->lengthInSamples, "7.1.4 file mastered: same channels, same length (latency compensated)");
            if (o) std::printf ("        rendered %s: %d ch, %lld samples, layout %s\n", juce::File (p.renderResult).getFileName().toRawUTF8(), (int) o->numChannels, (long long) o->lengthInSamples, o->getChannelLayout().getDescription().toRawUTF8());
        }
    }

    // 6. licence
    {
        auto& L = OmegaProcessor::licence();
        std::printf ("        trial days left %d\n", L.daysLeft.load());
        check (! L.expired() && L.daysLeft.load() == 15, "15-day trial on first run");
        {
            std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor()); ed->setSize (1220, 780);
            dynamic_cast<OmegaEditor*> (ed.get())->showLicence();
            for (int k = 0; k < 20; ++k) { mix.fill (b, 0.25f); p.processBlock (b, midi); pump (34); }
            juce::Image img (juce::Image::ARGB, 1220, 780, true); { juce::Graphics g (img); ed->paintEntireComponent (g, true); }
            juce::File f ("/home/claude/omega/build/omega_licence.png"); f.deleteFile(); juce::FileOutputStream os (f); juce::PNGImageFormat().writeImageToStream (img, os);
        }
        check (! OmegaProcessor::activate ("tester@polaris.mx", "ABCDEF12"), "bad key rejected");
        check (OmegaProcessor::activate ("Tester@Polaris.mx", "491D34CD-733C16EF-F8F41AC7-DF4A453C-D3C457A9-BDB9CF83-197370B8-B5CA96C9-3C22CB22-B55DD592-C4F84AF2-52AAED45-3A7E3EBE-E61B59B7-10CA280F-5045A35D-DFE84BB0-BC435804-B7F94A55-41D76B93-0297885E-8E160250-BE45869A-EB54863E-3C268182-2E63F4D2-7D3C8B50-602BC827-D37F99F4-85AFFB5E-7BFE04E4-B67F2651"), "valid OMEGA key accepted");
    }

    // 7. screenshots
    auto shot = [&] (const char* file, std::function<void (OmegaEditor&)> prep)
    {
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor()); ed->setSize (1220, 780);
        auto* oe = dynamic_cast<OmegaEditor*> (ed.get());
        if (prep) prep (*oe);
        for (int k = 0; k < 40; ++k) { mix.fill (b, 0.25f); p.processBlock (b, midi); pump (34); }
        juce::Image img (juce::Image::ARGB, 1220, 780, true);
        { juce::Graphics g (img); ed->paintEntireComponent (g, true); }
        juce::File f (juce::String ("/home/claude/omega/build/") + file); f.deleteFile();
        juce::FileOutputStream os (f); juce::PNGImageFormat().writeImageToStream (img, os);
    };
    setP (p, "universe", 2.0f); setP (p, "amount", 0.7f);
    shot ("omega_main.png", {});
    shot ("omega_adv.png", [] (OmegaEditor& e) { for (auto* c : e.getChildren()) if (auto* b = dynamic_cast<juce::TextButton*> (c)) if (b->getButtonText() == "ADVANCED") b->triggerClick(); });
    langFlag() = 1; setP (p, "universe", 4.0f);
    shot ("omega_es.png", {});
    langFlag() = 0;

    std::printf (fails == 0 ? "\nALL TESTS PASSED\n" : "\n%d FAILURES\n", fails);
    p.setPlayHead (nullptr);
    return fails;
}
