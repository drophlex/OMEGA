// OMEGA — editor: a living universe on top of honest, complete metering.
#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "Lang.h"

namespace om
{
struct Pal
{
    static juce::Colour bg()    { return juce::Colour (0xff04030c); }
    static juce::Colour panel() { return juce::Colour (0xff0b0920); }
    static juce::Colour line()  { return juce::Colour (0xff221d4d); }
    static juce::Colour text()  { return juce::Colour (0xffeeeaff); }
    static juce::Colour dim()   { return juce::Colour (0xff8a84b8); }
    static juce::Colour gold()  { return juce::Colour (0xffffd166); }
    static juce::Colour cyan()  { return juce::Colour (0xff58e1ff); }
    static juce::Colour pink()  { return juce::Colour (0xffff4fd8); }
    static juce::Colour red()   { return juce::Colour (0xffff5a4e); }
    static juce::Colour green() { return juce::Colour (0xff7affc1); }
};

class LnF : public juce::LookAndFeel_V4
{
public:
    LnF();
    void drawRotarySlider (juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawComboBox (juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getTextButtonFont (juce::TextButton&, int) override;
    juce::Font getLabelFont (juce::Label&) override;
};

class Galaxy : public juce::Component
{
public:
    explicit Galaxy (OmegaProcessor&);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    void tick (float dt);
private:
    OmegaProcessor& p;
    struct Star { float r, a, z, s, tw; int arm; };
    std::vector<Star> stars;
    struct Bg { float x, y, s, ph; };
    std::vector<Bg> bg;
    float rot = 0, energy = 0, gr = 0, sat = 0, width = 1, time = 0, dragStart = 0, dragAmount = 0;
    juce::Colour tint { 0xffb67cff };
    void setAmount (float v);
};

class Meter : public juce::Component       // vertical LUFS meter with target / sweet-spot zone
{
public:
    std::function<float()> value;           // LUFS
    float zoneLo = -20, zoneHi = -14, target = -100; juce::Colour col { 0xff58e1ff };
    juce::String label;
    void paint (juce::Graphics&) override;
};

class Compass : public juce::Component
{
public:
    explicit Compass (OmegaProcessor& pr) : p (pr) {}
    void paint (juce::Graphics&) override;
    OmegaProcessor& p; float needle = 0;
};

class Path : public juce::Component        // signal path: every stage shows its activity
{
public:
    explicit Path (OmegaProcessor& pr) : p (pr) {}
    void paint (juce::Graphics&) override;
    OmegaProcessor& p;
};

class Readout : public juce::Component     // loudness numbers
{
public:
    explicit Readout (OmegaProcessor& pr) : p (pr) {}
    void paint (juce::Graphics&) override;
    OmegaProcessor& p; float integ = -100, range = 0; int slow = 0;
};
} // namespace om

class OmegaEditor : public juce::AudioProcessorEditor, public juce::FileDragAndDropTarget, private juce::Timer
{
public:
    explicit OmegaEditor (OmegaProcessor&);
    ~OmegaEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    bool isInterestedInFileDrag (const juce::StringArray&) override { return true; }
    void filesDropped (const juce::StringArray&, int, int) override;
    void showLicence();
    void mouseDown (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void refreshTexts();
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    OmegaProcessor& proc;
    om::LnF lnf;
    om::Galaxy galaxy;
    om::Meter inMeter, outMeter;
    om::Compass compass;
    om::Path path;
    om::Readout readout;

    juce::TextButton uni[om::kNumUniverses];
    juce::TextButton snapA, snapB, undoBtn, redoBtn, licBtn, langBtn, helpBtn;
    juce::TextButton autoIn, learnLoud, balanceBtn, compareBtn, deltaBtn, advBtn, loadRef, renderBtn;
    juce::ComboBox target, quality;
    juce::Label refLabel, renderLabel, streamLabel;

    struct K { juce::Slider s; juce::Label l; std::unique_ptr<SA> a; const char* key; };
    std::vector<std::unique_ptr<K>> mainKnobs, advKnobs;
    std::unique_ptr<BA> cmpAtt, delAtt;
    std::unique_ptr<CA> tgtAtt, qAtt;
    std::unique_ptr<juce::Component> help, lic;
    std::unique_ptr<juce::FileChooser> chooser;
    bool advanced = false;
    int lastLang = -1;

    K* knob (std::vector<std::unique_ptr<K>>& v, const char* id, const char* key, juce::Colour c, const char* suffix = "");

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OmegaEditor)
};
