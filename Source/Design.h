// Polaris Sound Lab — graphic system ("signal print"): flat colour fields, Swiss grid, condensed grotesk,
// monospaced technical labels, katakana, registration marks and barcodes. Shared by OMEGA and CONSTELLATION.
// Fonts: Archivo (OFL), Space Mono (OFL), Noto Sans JP subset (OFL) — embedded as BinaryData.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "BinaryData.h"

namespace psl
{
namespace C
{
    inline const juce::Colour ink   { 0xff0b0b0c };
    inline const juce::Colour ink2  { 0xff17171a };
    inline const juce::Colour ink3  { 0xff232327 };
    inline const juce::Colour rule  { 0xff3b3a37 };
    inline const juce::Colour paper { 0xffede9df };
    inline const juce::Colour grey  { 0xff8f8b82 };
    inline const juce::Colour orange{ 0xffff4f12 };
    inline const juce::Colour yellow{ 0xfff2e205 };
    inline const juce::Colour cyan  { 0xff00a9e0 };
    inline const juce::Colour magenta{ 0xffff2e88 };
    inline const juce::Colour green { 0xff21d07a };
}

struct Fonts
{
    juce::Typeface::Ptr display, ui, body, mono, monoBold, jp;
    static Fonts& get()
    {
        static Fonts f;
        return f;
    }
private:
    Fonts()
    {
        auto tf = [] (const void* d, int s) { return juce::Typeface::createSystemTypefaceFor (d, (size_t) s); };
        display  = tf (BinaryData::ArchivoCondensedBlack_ttf, BinaryData::ArchivoCondensedBlack_ttfSize);
        ui       = tf (BinaryData::ArchivoSemiCondBold_ttf, BinaryData::ArchivoSemiCondBold_ttfSize);
        body     = tf (BinaryData::ArchivoRegular_ttf, BinaryData::ArchivoRegular_ttfSize);
        mono     = tf (BinaryData::SpaceMonoRegular_ttf, BinaryData::SpaceMonoRegular_ttfSize);
        monoBold = tf (BinaryData::SpaceMonoBold_ttf, BinaryData::SpaceMonoBold_ttfSize);
        jp       = tf (BinaryData::NotoJPBoldsub_ttf, BinaryData::NotoJPBoldsub_ttfSize);
    }
};

inline juce::Font mk (juce::Typeface::Ptr t, float h) { return juce::Font (juce::FontOptions (t).withHeight (h)); }
// toUpperCase that also handles Spanish accents (JUCE leaves them lower-case)
inline juce::String upper (const juce::String& in)
{
    auto s = in.toUpperCase();
    const char* lo[] = { "á","é","í","ó","ú","ñ","ü" }; const char* up[] = { "Á","É","Í","Ó","Ú","Ñ","Ü" };
    for (int i = 0; i < 7; ++i) s = s.replace (juce::String::fromUTF8 (lo[i]), juce::String::fromUTF8 (up[i]));
    return s;
}

inline juce::Font display (float h) { return mk (Fonts::get().display, h); }
inline juce::Font ui (float h)      { return mk (Fonts::get().ui, h); }
inline juce::Font body (float h)    { return mk (Fonts::get().body, h); }
inline juce::Font mono (float h, bool bold = false) { return mk (bold ? Fonts::get().monoBold : Fonts::get().mono, h); }
inline juce::Font jp (float h)      { return mk (Fonts::get().jp, h); }

inline void text (juce::Graphics& g, const juce::String& s, juce::Rectangle<float> r, const juce::Font& f, juce::Colour c,
                  juce::Justification j = juce::Justification::centredLeft)
{
    g.setColour (c); g.setFont (f); g.drawText (s, r, j, false);
}

// registration mark ⊕
inline void regMark (juce::Graphics& g, juce::Point<float> c, float r, juce::Colour col)
{
    g.setColour (col);
    g.drawEllipse (c.x - r * 0.6f, c.y - r * 0.6f, r * 1.2f, r * 1.2f, 1.0f);
    g.drawLine (c.x - r, c.y, c.x + r, c.y, 1.0f); g.drawLine (c.x, c.y - r, c.x, c.y + r, 1.0f);
}

// deterministic barcode
inline void barcode (juce::Graphics& g, juce::Rectangle<float> r, int seed, juce::Colour col)
{
    juce::Random rnd (seed); float x = r.getX();
    g.setColour (col);
    while (x < r.getRight())
    {
        const float w = (float) (1 + rnd.nextInt (3));
        if (rnd.nextFloat() > 0.38f) g.fillRect (x, r.getY(), juce::jmin (w, r.getRight() - x), r.getHeight());
        x += w + (float) (1 + rnd.nextInt (2));
    }
}

// diagonal hazard hatch
inline void hatch (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour col, float step = 6.0f)
{
    juce::Graphics::ScopedSaveState s (g);
    g.reduceClipRegion (r.toNearestInt());
    g.setColour (col);
    for (float x = r.getX() - r.getHeight(); x < r.getRight(); x += step)
        g.drawLine (x, r.getBottom(), x + r.getHeight(), r.getY(), 1.4f);
}

// section header: [01] TITLE  カタカナ ——————
inline void sectionTab (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& num, const juce::String& title,
                        const juce::String& kana, juce::Colour key)
{
    auto numBox = r.removeFromLeft (r.getHeight() * 1.5f);
    g.setColour (key); g.fillRect (numBox);
    text (g, num, numBox, mono (r.getHeight() * 0.62f, true), C::ink, juce::Justification::centred);
    r.removeFromLeft (6);
    const auto f = ui (r.getHeight() * 0.78f);
    const float tw = juce::GlyphArrangement::getStringWidth (f, title);
    text (g, title, r.withWidth (tw + 2), f, C::paper);
    r.removeFromLeft (tw + 8);
    if (kana.isNotEmpty())
    {
        const auto jf = jp (r.getHeight() * 0.62f);
        const float kw = juce::GlyphArrangement::getStringWidth (jf, kana);
        text (g, kana, r.withWidth (kw + 2), jf, C::grey);
        r.removeFromLeft (kw + 8);
    }
    g.setColour (C::rule); g.fillRect (r.getX(), r.getCentreY(), r.getWidth(), 1.0f);
}

// small spaced technical label (mono caps)
inline void tech (juce::Graphics& g, const juce::String& s, juce::Rectangle<float> r, juce::Colour c,
                  juce::Justification j = juce::Justification::centredLeft, float h = 9.5f)
{
    text (g, upper (s), r, mono (h), c, j);
}

// faint dot grid
inline void dotGrid (juce::Graphics& g, juce::Rectangle<float> r, float step, juce::Colour c)
{
    g.setColour (c);
    for (float y = r.getY() + step * 0.5f; y < r.getBottom(); y += step)
        for (float x = r.getX() + step * 0.5f; x < r.getRight(); x += step)
            g.fillRect (x, y, 1.0f, 1.0f);
}

// segmented meter bar (vertical), value 0..1
inline void segBar (juce::Graphics& g, juce::Rectangle<float> r, float v, juce::Colour on, juce::Colour off, int segs = 24)
{
    const float h = r.getHeight() / (float) segs;
    for (int i = 0; i < segs; ++i)
    {
        const float y = r.getBottom() - (float) (i + 1) * h;
        const bool lit = ((float) i + 0.5f) / (float) segs <= v;
        g.setColour (lit ? on : off);
        g.fillRect (r.getX(), y + 1.0f, r.getWidth(), h - 2.0f);
    }
}

// ───────────────────────────── flat look & feel ─────────────────────────────
class FlatLnF : public juce::LookAndFeel_V4
{
public:
    juce::Colour key { C::orange };
    explicit FlatLnF (juce::Colour k) : key (k)
    {
        setColour (juce::ComboBox::backgroundColourId, C::ink); setColour (juce::ComboBox::textColourId, C::paper);
        setColour (juce::ComboBox::outlineColourId, C::rule); setColour (juce::ComboBox::arrowColourId, key);
        setColour (juce::PopupMenu::backgroundColourId, C::ink); setColour (juce::PopupMenu::textColourId, C::paper);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, key); setColour (juce::PopupMenu::highlightedTextColourId, C::ink);
        setColour (juce::TextButton::buttonColourId, C::ink); setColour (juce::TextButton::buttonOnColourId, key);
        setColour (juce::TextButton::textColourOffId, C::paper); setColour (juce::TextButton::textColourOnId, C::ink);
        setColour (juce::Label::textColourId, C::paper);
        setColour (juce::Slider::textBoxTextColourId, C::paper); setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::TextEditor::backgroundColourId, C::ink); setColour (juce::TextEditor::textColourId, C::paper);
        setColour (juce::TextEditor::outlineColourId, C::rule); setColour (juce::TextEditor::focusedOutlineColourId, key);
        setColour (juce::CaretComponent::caretColourId, key);
    }
    juce::Typeface::Ptr getTypefaceForFont (const juce::Font&) override { return Fonts::get().ui; }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s) override
    {
        auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4);
        const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f; const auto c = b.getCentre();
        const auto col = s.findColour (juce::Slider::rotarySliderFillColourId);
        // ticks
        for (int i = 0; i <= 10; ++i)
        {
            const float a = a0 + (a1 - a0) * (float) i / 10.0f;
            const auto p1 = c.getPointOnCircumference (r, a), p2 = c.getPointOnCircumference (r - (i % 5 == 0 ? 6.0f : 3.0f), a);
            g.setColour (i % 5 == 0 ? C::paper.withAlpha (0.7f) : C::grey.withAlpha (0.6f)); g.drawLine (p1.x, p1.y, p2.x, p2.y, 1.0f);
        }
        const bool bip = s.getMinimum() < 0 && s.getMaximum() > 0;
        const float z = bip ? (float) (-s.getMinimum() / (s.getMaximum() - s.getMinimum())) : 0.0f;
        const float as = a0 + z * (a1 - a0), av = a0 + pos * (a1 - a0);
        juce::Path arc; arc.addCentredArc (c.x, c.y, r - 10, r - 10, 0, juce::jmin (as, av), juce::jmax (as, av), true);
        g.setColour (col); g.strokePath (arc, juce::PathStrokeType (5.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::butt));
        const float ir = r - 15;
        g.setColour (C::ink2); g.fillEllipse (c.x - ir, c.y - ir, ir * 2, ir * 2);
        g.setColour (C::rule); g.drawEllipse (c.x - ir, c.y - ir, ir * 2, ir * 2, 1.0f);
        const auto tip = c.getPointOnCircumference (ir, av), base = c.getPointOnCircumference (ir * 0.25f, av);
        g.setColour (C::paper); g.drawLine (base.x, base.y, tip.x, tip.y, 2.0f);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) override
    {
        auto r = b.getLocalBounds().toFloat().reduced (0.5f);
        const auto on = b.findColour (juce::TextButton::buttonOnColourId);
        const auto off = b.findColour (juce::TextButton::buttonColourId);
        if (b.getToggleState()) { g.setColour (on); g.fillRect (r); }
        else
        {
            g.setColour (off.brighter (over ? 0.12f : 0.0f).darker (down ? 0.2f : 0.0f)); g.fillRect (r);
            g.setColour (over ? on : C::rule.brighter (0.25f)); g.drawRect (r, 1.0f);
        }
    }
    void drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool) override
    {
        const auto c = b.findColour (b.getToggleState() ? juce::TextButton::textColourOnId : juce::TextButton::textColourOffId);
        g.setColour (b.isEnabled() ? c : c.withAlpha (0.4f));
        g.setFont (ui (juce::jmin (14.0f, (float) b.getHeight() * 0.46f)));
        g.drawFittedText (upper (b.getButtonText()), b.getLocalBounds().reduced (4, 2), juce::Justification::centred, 1, 0.5f);
    }
    void drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& cb) override
    {
        auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h);
        g.setColour (C::ink); g.fillRect (r);
        g.setColour (cb.isMouseOver() ? key : C::rule.brighter (0.25f)); g.drawRect (r.reduced (0.5f), 1.0f);
        auto ar = r.removeFromRight ((float) h);
        g.setColour (key); g.fillRect (ar.reduced (3));
        juce::Path t; t.addTriangle (ar.getCentreX() - 4, ar.getCentreY() - 2, ar.getCentreX() + 4, ar.getCentreY() - 2, ar.getCentreX(), ar.getCentreY() + 3);
        g.setColour (C::ink); g.fillPath (t);
    }
    juce::Font getComboBoxFont (juce::ComboBox&) override { return ui (14.0f); }
    void positionComboBoxText (juce::ComboBox& cb, juce::Label& l) override { l.setBounds (6, 1, cb.getWidth() - cb.getHeight() - 8, cb.getHeight() - 2); l.setFont (ui (14.0f)); }
    juce::Font getPopupMenuFont() override { return ui (15.0f); }
    juce::Font getLabelFont (juce::Label& l) override { return l.getFont(); }
    juce::Label* createSliderTextBox (juce::Slider& s) override
    {
        auto* l = LookAndFeel_V4::createSliderTextBox (s);
        l->setFont (mono (12.0f, true));
        l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
        l->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        l->setColour (juce::Label::textColourId, C::paper);
        l->setColour (juce::Label::outlineWhenEditingColourId, key);
        return l;
    }
    void drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s) override
    {
        const auto col = s.findColour (juce::Slider::trackColourId);
        const float cy = (float) y + (float) h * 0.5f;
        g.setColour (C::rule); g.fillRect ((float) x, cy - 1, (float) w, 2.0f);
        for (int i = 0; i <= 10; ++i) { const float tx = (float) x + (float) w * (float) i / 10.0f; g.fillRect (tx, cy - (i % 5 == 0 ? 6.0f : 3.0f), 1.0f, i % 5 == 0 ? 12.0f : 6.0f); }
        g.setColour (col); g.fillRect ((float) x, cy - 3, pos - (float) x, 6.0f);
        g.setColour (C::paper); g.fillRect (pos - 3, cy - 9, 6.0f, 18.0f);
    }
    void fillTextEditorBackground (juce::Graphics& g, int w, int h, juce::TextEditor&) override { g.setColour (C::ink); g.fillRect (0, 0, w, h); }
    void drawTextEditorOutline (juce::Graphics& g, int w, int h, juce::TextEditor& te) override
    {
        g.setColour (te.hasKeyboardFocus (true) ? key : C::rule.brighter (0.3f)); g.drawRect (0, 0, w, h, 1);
    }
};
} // namespace psl
