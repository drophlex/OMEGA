// OMEGA — by Polaris Sound Lab (Mérida, Yucatán). Design: Dark Funk Durcal.
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "Engine.h"

class OmegaProcessor : public juce::AudioProcessor, private juce::Timer
{
public:
    OmegaProcessor();
    ~OmegaProcessor() override;

    void prepareToPlay (double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Omega"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.1; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::UndoManager undo;
    juce::AudioProcessorValueTreeState apvts;
    om::Engine engine;
    om::LoudnessMeter meterIn, meterOut;

    // ── live info for the UI ──
    std::atomic<float> truePeakDb { -100.f }, truePeakMax { -100.f }, inPeakDb { -100.f }, outPeakDb { -100.f };
    std::atomic<float> matchGainDb { 0 };
    std::atomic<bool>  playing { false };
    float getIntegrated() { return meterOut.integrated(); }
    float getRange()      { return meterOut.range(); }
    void  resetMeters()   { meterOut.resetIntegrated(); meterIn.resetIntegrated(); truePeakMax = -100.f; }

    // spectra (message thread, 10 octave bands, long-term and fast) for visuals, tone compass and reference match
    std::array<float, 10> specOutFast {}, specOutLong {}, specInLong {};
    std::array<float, 64> spectrumLine {};          // fine log spectrum of the output (dB) for the horizon
    float toneSlope = -2.5f;                          // dB / octave (63 Hz … 8 kHz)
    void balanceTone();

    // assistants
    void startAutoInput();
    void startLoudnessMatch();
    bool isAutoInput() const { return autoInputLeft > 0; }
    bool isLoudMatch() const { return loudLeft > 0; }
    static float targetLufs (int idx);

    // reference
    bool loadReference (const juce::File&);
    juce::String refName; float refLufs = -100; std::array<float, 10> refBands {}; bool hasRef = false;

    // surround file mastering (background)
    void renderFile (const juce::File&);
    std::atomic<float> renderProgress { -1.0f };
    juce::String renderResult;

    // A/B snapshots
    void storeSnapshot (int i);
    void recallSnapshot (int i);
    int  currentSnapshot = 0;

    // ── licence ──
    struct Licence
    {
        std::atomic<bool> licensed { false }; juce::String owner; std::atomic<int> daysLeft { 15 };
        Licence() = default;
        Licence (Licence&& o) : licensed (o.licensed.load()), owner (o.owner), daysLeft (o.daysLeft.load()) {}
        bool expired() const noexcept { return ! licensed.load() && daysLeft.load() <= 0; }
    };
    static Licence& licence();
    static bool activate (const juce::String& email, const juce::String& key);

private:
    void timerCallback() override;
    void fillSettings (om::Settings&);
    void analyseFifo();
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();

    std::atomic<float>* P (const char* id) { return apvts.getRawParameterValue (id); }
    double sr = 48000; int maxBlock = 512; int qualityPrepared = -1;
    juce::AudioBuffer<float> dry, tpBuf;
    std::unique_ptr<juce::dsp::Oversampling<float>> tpOs;
    juce::AbstractFifo fifo { 1 << 15 }; std::vector<float> fifoOut, fifoIn;
    juce::dsp::FFT fft { 12 }; std::vector<float> fftWork;
    std::vector<float> histOut, histIn; int histPos = 0;
    std::array<double, 10> accOut {}, accIn {}; double accN = 0;
    float autoInputLeft = 0, autoInputMax = -100, loudLeft = 0, loudWindowMax = -100; int loudStable = 0;
    std::array<juce::ValueTree, 2> snapshots;
    std::unique_ptr<juce::Thread> renderThread;
    std::array<std::atomic<float>, 10> matchCurve;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OmegaProcessor)
};
