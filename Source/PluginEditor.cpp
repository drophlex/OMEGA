// OMEGA — editor. Design: Dark Funk Durcal · Polaris Sound Lab, Mérida, Yucatán
#include "PluginEditor.h"

using namespace om;

namespace
{
juce::Font F (float h, bool bold = true) { return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain)); }
const char* kBuyOmega  = "https://www.paypal.com/cgi-bin/webscr?cmd=_xclick&business=drophlex%40gmail.com&item_name=OMEGA%20licence%20(send%20your%20e-mail)&amount=49.00&currency_code=USD&no_shipping=1";
const char* kBuyBundle = "https://www.paypal.com/cgi-bin/webscr?cmd=_xclick&business=drophlex%40gmail.com&item_name=OMEGA%20%2B%20CONSTELLATION%20licences%20(send%20your%20e-mail)&amount=69.00&currency_code=USD&no_shipping=1";

juce::Path omegaGlyph (juce::Rectangle<float> r)
{
    // Ω drawn as a path so it renders identically everywhere
    juce::Path p;
    const float cx = r.getCentreX(), w = r.getWidth(), h = r.getHeight();
    const float cy = r.getY() + h * 0.46f, rad = w * 0.38f;
    p.addCentredArc (cx, cy, rad, rad * 1.02f, 0.0f, juce::MathConstants<float>::pi * 1.22f, juce::MathConstants<float>::pi * 2.78f, true);
    juce::Path s; juce::PathStrokeType (w * 0.11f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded).createStrokedPath (s, p);
    s.addRoundedRectangle (cx - w * 0.48f, r.getBottom() - h * 0.11f, w * 0.34f, h * 0.1f, h * 0.04f);
    s.addRoundedRectangle (cx + w * 0.14f, r.getBottom() - h * 0.11f, w * 0.34f, h * 0.1f, h * 0.04f);
    return s;
}

struct HelpOverlay : juce::Component
{
    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xeb04030c));
        auto r = getLocalBounds().reduced (70, 50).toFloat();
        g.setColour (Pal::panel()); g.fillRoundedRectangle (r, 18);
        g.setColour (Pal::gold().withAlpha (0.6f)); g.drawRoundedRectangle (r, 18, 1.5f);
        g.setColour (Pal::text()); g.setFont (F (14.5f, false));
        g.drawFittedText (tr ("help"), r.reduced (32, 28).toNearestInt(), juce::Justification::topLeft, 40, 1.0f);
    }
    void mouseDown (const juce::MouseEvent&) override { setVisible (false); }
};

struct LicencePanel : juce::Component
{
    juce::TextEditor email, key;
    juce::TextButton buy, bundle, activate, close;
    juce::String status; bool ok = false;
    std::function<void()> onDone;
    LicencePanel()
    {
        for (auto* t : { &email, &key })
        {
            addAndMakeVisible (*t); t->setFont (F (15.0f, false));
            t->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff070516));
            t->setColour (juce::TextEditor::outlineColourId, Pal::line().brighter (0.4f));
            t->setColour (juce::TextEditor::focusedOutlineColourId, Pal::gold());
            t->setColour (juce::TextEditor::textColourId, Pal::text());
        }
        key.setMultiLine (true); key.setReturnKeyStartsNewLine (false);
        for (auto* b : { &buy, &bundle, &activate, &close }) addAndMakeVisible (*b);
        buy.setColour (juce::TextButton::buttonOnColourId, Pal::gold()); buy.setToggleState (true, juce::dontSendNotification);
        bundle.setColour (juce::TextButton::buttonOnColourId, Pal::cyan());
        activate.setColour (juce::TextButton::buttonOnColourId, Pal::gold()); activate.setToggleState (true, juce::dontSendNotification);
        buy.onClick = [] { juce::URL (kBuyOmega).launchInDefaultBrowser(); };
        bundle.onClick = [] { juce::URL (kBuyBundle).launchInDefaultBrowser(); };
        activate.onClick = [this]
        {
            ok = OmegaProcessor::activate (email.getText(), key.getText());
            status = ok ? tr ("actOk") : tr ("actBad"); repaint();
            if (ok && onDone) onDone();
        };
        close.onClick = [this] { setVisible (false); };
        refresh();
    }
    void refresh()
    {
        buy.setButtonText (tr ("buyOmega")); bundle.setButtonText (tr ("buyBundle"));
        activate.setButtonText (tr ("activate")); close.setButtonText (tr ("close"));
        email.setTextToShowWhenEmpty (tr ("email"), Pal::dim()); key.setTextToShowWhenEmpty (tr ("key"), Pal::dim());
        repaint();
    }
    juce::Rectangle<int> box() const { return getLocalBounds().withSizeKeepingCentre (juce::jmin (660, getWidth() - 40), juce::jmin (540, getHeight() - 40)); }
    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xeb04030c));
        auto r = box().toFloat();
        g.setColour (Pal::panel()); g.fillRoundedRectangle (r, 18);
        g.setColour (Pal::gold().withAlpha (0.7f)); g.drawRoundedRectangle (r, 18, 1.5f);
        auto in = r.reduced (30, 24);
        g.setColour (Pal::text()); g.setFont (F (23));
        g.drawText (tr ("licTitle"), in.removeFromTop (32).toNearestInt(), juce::Justification::centredLeft);
        auto& L = OmegaProcessor::licence();
        g.setFont (F (13));
        g.setColour (L.licensed ? Pal::green() : (L.expired() ? Pal::red() : Pal::gold()));
        g.drawText (L.licensed ? tr ("licensed") + ": " + L.owner : (L.expired() ? tr ("expired") : tr ("trial").replace ("%d", juce::String (L.daysLeft.load()))),
                    in.removeFromTop (22).toNearestInt(), juce::Justification::centredLeft);
        g.setColour (Pal::text().withAlpha (0.85f)); g.setFont (F (13.5f, false));
        g.drawFittedText (tr ("licText"), in.removeFromTop (118).toNearestInt(), juce::Justification::topLeft, 12, 1.0f);
        if (status.isNotEmpty()) { g.setColour (ok ? Pal::green() : Pal::red()); g.setFont (F (13)); g.drawFittedText (status, box().reduced (30, 24).removeFromBottom (64).removeFromTop (22), juce::Justification::centredLeft, 2); }
    }
    void resized() override
    {
        auto in = box().reduced (30, 24);
        in.removeFromTop (32 + 22 + 118 + 8);
        auto row = in.removeFromTop (40);
        buy.setBounds (row.removeFromLeft (200)); row.removeFromLeft (10); bundle.setBounds (row.removeFromLeft (300));
        in.removeFromTop (14);
        email.setBounds (in.removeFromTop (34)); in.removeFromTop (8);
        key.setBounds (in.removeFromTop (70));
        auto b = in.removeFromBottom (38);
        close.setBounds (b.removeFromRight (110)); b.removeFromRight (10); activate.setBounds (b.removeFromRight (150));
    }
    void mouseDown (const juce::MouseEvent& e) override { if (! box().contains (e.getPosition())) setVisible (false); }
};
} // namespace

// ───────────────────────────── LookAndFeel ─────────────────────────────
LnF::LnF()
{
    setColour (juce::ComboBox::backgroundColourId, Pal::panel()); setColour (juce::ComboBox::textColourId, Pal::text());
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff0a0820)); setColour (juce::PopupMenu::textColourId, Pal::text());
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Pal::gold().withAlpha (0.25f));
    setColour (juce::TextButton::textColourOffId, Pal::text()); setColour (juce::TextButton::textColourOnId, juce::Colours::black);
    setColour (juce::Label::textColourId, Pal::text());
    setColour (juce::Slider::textBoxTextColourId, Pal::text());
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}
juce::Font LnF::getComboBoxFont (juce::ComboBox&) { return F (13); }
juce::Font LnF::getTextButtonFont (juce::TextButton&, int h) { return F (juce::jmin (13.0f, h * 0.48f)); }
juce::Font LnF::getLabelFont (juce::Label& l) { return l.getFont(); }

void LnF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (3);
    const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f; const auto c = b.getCentre();
    const auto col = s.findColour (juce::Slider::rotarySliderFillColourId);
    juce::Path bg; bg.addCentredArc (c.x, c.y, r - 3, r - 3, 0, a0, a1, true);
    g.setColour (Pal::line()); g.strokePath (bg, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    const bool bip = s.getMinimum() < 0 && s.getMaximum() > 0;
    const float z = bip ? (float) (-s.getMinimum() / (s.getMaximum() - s.getMinimum())) : 0.0f;
    const float as = a0 + z * (a1 - a0), av = a0 + pos * (a1 - a0);
    juce::Path arc; arc.addCentredArc (c.x, c.y, r - 3, r - 3, 0, juce::jmin (as, av), juce::jmax (as, av), true);
    g.setColour (col.withAlpha (0.25f)); g.strokePath (arc, juce::PathStrokeType (9.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (col); g.strokePath (arc, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    const float ir = r * 0.6f;
    juce::ColourGradient gr (juce::Colour (0xff201a4a), c.x, c.y - ir, juce::Colour (0xff060414), c.x, c.y + ir, false);
    g.setGradientFill (gr); g.fillEllipse (c.x - ir, c.y - ir, ir * 2, ir * 2);
    const auto tip = c.getPointOnCircumference (ir * 0.75f, av);
    g.setColour (juce::Colours::white); g.fillEllipse (tip.x - 2.5f, tip.y - 2.5f, 5, 5);
}

void LnF::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (1);
    const auto acc = b.findColour (juce::TextButton::buttonOnColourId);
    if (b.getToggleState())
    {
        juce::ColourGradient gr (acc.brighter (0.25f), r.getX(), r.getY(), acc.darker (0.25f), r.getX(), r.getBottom(), false);
        g.setGradientFill (gr); g.fillRoundedRectangle (r, 8);
        g.setColour (acc.withAlpha (0.35f)); g.drawRoundedRectangle (r.expanded (1.5f), 9, 2.5f);
    }
    else
    {
        g.setColour (Pal::panel().brighter (over ? 0.25f : 0.08f).darker (down ? 0.3f : 0)); g.fillRoundedRectangle (r, 8);
        g.setColour (over ? acc.withAlpha (0.8f) : Pal::line().brighter (0.3f)); g.drawRoundedRectangle (r, 8, 1);
    }
}

void LnF::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& cb)
{
    auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1);
    g.setColour (Pal::panel().brighter (cb.isMouseOver() ? 0.2f : 0.06f)); g.fillRoundedRectangle (r, 8);
    g.setColour (cb.isMouseOver() ? Pal::gold() : Pal::line().brighter (0.3f)); g.drawRoundedRectangle (r, 8, 1);
    juce::Path t; const float ax = (float) w - 15, ay = (float) h * 0.5f; t.addTriangle (ax - 4, ay - 2, ax + 4, ay - 2, ax, ay + 3);
    g.setColour (Pal::gold()); g.fillPath (t);
}

// ───────────────────────────── Galaxy: the living universe (and the big OMEGA control) ─────────────────────────────
Galaxy::Galaxy (OmegaProcessor& pr) : p (pr)
{
    juce::Random r (21);
    for (int i = 0; i < 1100; ++i)
    {
        const int arm = i % 3;
        const float rr = std::pow (r.nextFloat(), 0.7f);
        stars.push_back ({ rr, (float) arm * 2.0944f + rr * 4.2f + (r.nextFloat() - 0.5f) * (0.55f + rr * 0.6f), (r.nextFloat() - 0.5f) * 0.08f,
                           0.5f + r.nextFloat() * 1.7f, r.nextFloat() * 6.28f, arm });
    }
    for (int i = 0; i < 320; ++i) bg.push_back ({ r.nextFloat(), r.nextFloat(), 0.4f + r.nextFloat() * 1.4f, r.nextFloat() * 6.28f });
    setOpaque (true);
}

void Galaxy::setAmount (float v)
{
    if (auto* prm = p.apvts.getParameter ("amount")) prm->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, v));
}
void Galaxy::mouseDown (const juce::MouseEvent& e)
{
    if (auto* prm = p.apvts.getParameter ("amount")) { prm->beginChangeGesture(); dragAmount = prm->getValue(); }
    dragStart = (float) e.position.y;
}
void Galaxy::mouseDrag (const juce::MouseEvent& e) { setAmount (dragAmount + (dragStart - (float) e.position.y) / (e.mods.isShiftDown() ? 900.0f : 250.0f)); }
void Galaxy::mouseUp (const juce::MouseEvent&) { if (auto* prm = p.apvts.getParameter ("amount")) prm->endChangeGesture(); }
void Galaxy::mouseDoubleClick (const juce::MouseEvent&) { setAmount (0.5f); }
void Galaxy::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w)
{
    if (auto* prm = p.apvts.getParameter ("amount")) setAmount (prm->getValue() + w.deltaY * 0.15f);
}

void Galaxy::tick (float dt)
{
    time += dt;
    const float st = p.meterOut.shortTerm.load();
    const float e = sat01 ((st + 40.0f) / 34.0f);
    energy += 0.08f * (e - energy);
    gr += 0.2f * (-p.engine.grLimit.load() - gr);
    sat += 0.1f * (sat01 (p.engine.satAct.load() * 6.0f) - sat);
    width += 0.05f * (p.engine.widthNow.load() - width);
    rot += dt * (0.05f + 0.55f * energy) * (p.playing.load() ? 1.0f : 0.35f);
    const auto& U = kUniverses[juce::jlimit (0, kNumUniverses - 1, (int) p.apvts.getRawParameterValue ("universe")->load())];
    tint = tint.interpolatedWith (juce::Colour (U.colour), 0.08f);
    repaint();
}

void Galaxy::paint (juce::Graphics& g)
{
    const float W = (float) getWidth(), H = (float) getHeight();
    juce::ColourGradient sky (juce::Colour (0xff07051a), 0, 0, juce::Colour (0xff010006), 0, H, false);
    g.setGradientFill (sky); g.fillAll();
    auto neb = [&] (float x, float y, float r, juce::Colour c) { juce::ColourGradient q (c, x, y, c.withAlpha (0.0f), x + r, y, true); g.setGradientFill (q); g.fillEllipse (x - r, y - r, 2 * r, 2 * r); };
    neb (W * (0.22f + 0.03f * std::sin (time * 0.05f)), H * 0.25f, W * 0.40f, tint.withAlpha (0.10f));
    neb (W * (0.82f + 0.02f * std::cos (time * 0.04f)), H * 0.70f, W * 0.35f, juce::Colour (0x1aff4fd8));
    neb (W * 0.6f, H * 0.15f, W * 0.22f, juce::Colour (0x1458e1ff));
    for (auto& s : bg)
    {
        const float tw = 0.5f + 0.5f * std::sin (time * (0.5f + s.s) + s.ph);
        g.setColour (juce::Colours::white.withAlpha (0.10f + 0.4f * tw * s.s / 1.8f));
        g.fillEllipse (s.x * W, s.y * H, s.s, s.s);
    }

    // spectrum horizon (output) + reference
    {
        juce::Path pth; const float base = H - 6.0f;
        for (int j = 0; j < 64; ++j)
        {
            const float x = W * (float) j / 63.0f;
            const float v = sat01 ((p.spectrumLine[(size_t) j] + 70.0f) / 60.0f);
            const float y = base - v * H * 0.22f;
            if (j == 0) pth.startNewSubPath (x, y); else pth.lineTo (x, y);
        }
        juce::Path fill = pth; fill.lineTo (W, H); fill.lineTo (0, H); fill.closeSubPath();
        juce::ColourGradient hg (tint.withAlpha (0.28f), 0, H * 0.75f, tint.withAlpha (0.0f), 0, H, false);
        g.setGradientFill (hg); g.fillPath (fill);
        g.setColour (tint.withAlpha (0.7f)); g.strokePath (pth, juce::PathStrokeType (1.6f));
        if (p.hasRef)
        {
            juce::Path rp; float rm = 0, mm = 0; for (int b = 1; b < 9; ++b) { rm += p.refBands[(size_t) b]; mm += p.specOutLong[(size_t) b]; } rm /= 8; mm /= 8;
            for (int b = 0; b < 10; ++b)
            {
                const float x = W * (std::log2 (kOct[b] / 30.0f) / 9.4f);
                const float lvl = p.refBands[(size_t) b] - rm + mm + 3.0f * std::log2 (kOct[b] / 1000.0f);
                const float y = base - sat01 ((lvl + 70.0f) / 60.0f) * H * 0.22f;
                if (b == 0) rp.startNewSubPath (x, y); else rp.lineTo (x, y);
            }
            juce::Path dashed; const float d[] = { 5, 5 }; juce::PathStrokeType (1.4f).createDashedStroke (dashed, rp, d, 2);
            g.setColour (Pal::gold().withAlpha (0.8f)); g.fillPath (dashed);
        }
    }

    // galaxy
    const float cx = W * 0.5f, cy = H * 0.44f;
    const float R = juce::jmin (W * 0.36f, H * 0.40f);
    const float amount = p.apvts.getRawParameterValue ("amount")->load();
    const float tilt = 0.42f;
    // halo
    neb (cx, cy, R * (1.15f + 0.2f * energy), tint.withAlpha (0.06f + 0.08f * energy));
    for (auto& s : stars)
    {
        const float spread = 1.0f + (width - 1.0f) * 1.6f;
        const float ang = s.a + rot * (1.2f - s.r * 0.7f);
        const float rr = s.r * R * (0.92f + 0.12f * energy);
        const float x = cx + std::cos (ang) * rr * spread;
        const float y = cy + std::sin (ang) * rr * tilt + s.z * R;
        const float tw = 0.6f + 0.4f * std::sin (time * 2.0f + s.tw);
        const float br = (1.0f - s.r * 0.45f) * (0.55f + 0.45f * energy) * tw;
        const auto c = s.r < 0.22f ? juce::Colour (0xfffff3d6) : tint.interpolatedWith (juce::Colours::white, 0.15f + 0.45f * (1.0f - s.r));
        const float sz = s.s * (1.0f + 0.9f * (1.0f - s.r)) * (0.9f + 0.4f * energy);
        if (s.arm == 0 && s.r > 0.15f) { g.setColour (c.withAlpha (0.10f * br)); g.fillEllipse (x - sz * 2.2f, y - sz * 2.2f, sz * 4.4f, sz * 4.4f); }
        g.setColour (c.withAlpha (juce::jlimit (0.08f, 1.0f, br)));
        g.fillEllipse (x - sz * 0.5f, y - sz * 0.5f, sz, sz);
    }
    // multiband "orbits": thickness = gain reduction of low / mid / high
    for (int b = 0; b < 3; ++b)
    {
        const float grb = sat01 (-p.engine.grBand[b].load() / 8.0f);
        const float rr = R * (0.55f + 0.17f * (float) b);
        const juce::Colour oc = b == 0 ? juce::Colour (0xffff7a5c) : (b == 1 ? Pal::gold() : Pal::cyan());
        g.setColour (oc.withAlpha (0.10f + 0.6f * grb));
        g.drawEllipse (cx - rr, cy - rr * tilt, rr * 2, rr * 2 * tilt, 0.8f + 4.0f * grb);
    }
    // core: brightness from harmonics; event horizon grows with limiting
    const float core = R * (0.16f + 0.05f * energy);
    juce::ColourGradient cg (juce::Colours::white.withAlpha (0.9f), cx, cy, tint.withAlpha (0.0f), cx + core * (1.4f + 1.0f * sat), cy, true);
    g.setGradientFill (cg); g.fillEllipse (cx - core * 2.4f, cy - core * 1.4f, core * 4.8f, core * 2.8f);
    const float eh = core * (0.25f + 0.12f * juce::jmin (gr, 8.0f));
    g.setColour (juce::Colours::black.withAlpha (0.92f)); g.fillEllipse (cx - eh, cy - eh * 0.75f, eh * 2, eh * 1.5f);
    g.setColour (juce::Colour (0xffffc58a).withAlpha (0.4f + 0.5f * sat01 (gr / 4.0f))); g.drawEllipse (cx - eh, cy - eh * 0.75f, eh * 2, eh * 1.5f, 1.5f);

    // amount ring (the big knob)
    {
        const float rr = R * 1.08f;
        juce::Path ring; ring.addCentredArc (cx, cy, rr, rr * 0.62f, 0, -2.4f, 2.4f, true);
        g.setColour (Pal::line().withAlpha (0.8f)); g.strokePath (ring, juce::PathStrokeType (3.0f));
        juce::Path val; val.addCentredArc (cx, cy, rr, rr * 0.62f, 0, -2.4f, -2.4f + 4.8f * amount, true);
        g.setColour (tint.withAlpha (0.35f)); g.strokePath (val, juce::PathStrokeType (10.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (tint.brighter (0.4f)); g.strokePath (val, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    // title + amount
    g.setColour (Pal::text()); g.setFont (F (15.0f));
    const auto& U = kUniverses[juce::jlimit (0, kNumUniverses - 1, (int) p.apvts.getRawParameterValue ("universe")->load())];
    const bool es = langFlag().load() == 1;
    g.drawText (juce::String::fromUTF8 (es ? U.es : U.en).toUpperCase() + "   " + juce::String (juce::roundToInt (amount * 100)) + "%",
                0, (int) (cy + R * 0.62f), (int) W, 22, juce::Justification::centred);
    g.setColour (Pal::dim()); g.setFont (F (11.5f, false));
    g.drawText (juce::String::fromUTF8 (es ? U.tagEs : U.tagEn), 0, (int) (cy + R * 0.62f) + 20, (int) W, 16, juce::Justification::centred);
    g.drawText (tr ("dragHint"), 0, 8, (int) W, 16, juce::Justification::centred);
    g.setColour (Pal::line()); g.drawRect (getLocalBounds(), 1);
}

// ───────────────────────────── meters & panels ─────────────────────────────
void Meter::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xff070516)); g.fillRoundedRectangle (r, 6);
    auto bar = r.reduced (6, 18);
    auto yOf = [&] (float lufs) { return bar.getBottom() - sat01 ((lufs + 40.0f) / 40.0f) * bar.getHeight(); };
    if (zoneHi > zoneLo) { g.setColour (Pal::green().withAlpha (0.16f)); g.fillRect (bar.getX(), yOf (zoneHi), bar.getWidth(), yOf (zoneLo) - yOf (zoneHi)); }
    const float v = value ? value() : -100.0f;
    const float y = yOf (v);
    juce::ColourGradient gr (col, 0, bar.getBottom(), col.brighter (0.6f), 0, bar.getY(), false);
    g.setGradientFill (gr); g.fillRoundedRectangle (bar.getX() + 3, y, bar.getWidth() - 6, bar.getBottom() - y, 3);
    if (target > -60) { g.setColour (Pal::gold()); g.drawLine (bar.getX(), yOf (target), bar.getRight(), yOf (target), 2); }
    g.setColour (Pal::dim()); g.setFont (F (9.5f, false));
    for (int l : { -36, -24, -14, -6 }) g.drawText (juce::String (l), (int) bar.getX(), (int) yOf ((float) l) - 6, (int) bar.getWidth(), 12, juce::Justification::centredRight);
    g.setColour (Pal::text()); g.setFont (F (11));
    g.drawText (label, r.removeFromTop (16).toNearestInt(), juce::Justification::centred);
    g.drawText (v > -60 ? juce::String (v, 1) : juce::String ("-"), r.removeFromBottom (16).toNearestInt(), juce::Justification::centred);
}

void Compass::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xff070516)); g.fillRoundedRectangle (r, 8);
    g.setColour (Pal::dim()); g.setFont (F (10.5f)); g.drawText (tr ("tone"), r.removeFromTop (18).toNearestInt(), juce::Justification::centred);
    const float c = r.getCentreX(), base = r.getBottom() - 16, rad = juce::jmin (r.getWidth() * 0.42f, r.getHeight() - 22);
    for (int i = 0; i < 3; ++i)
    {
        juce::Path a; const float a0 = -juce::MathConstants<float>::halfPi - 1.3f + (float) i * 0.8667f;
        a.addCentredArc (c, base, rad, rad, 0, a0, a0 + 0.8667f, true);
        g.setColour ((i == 1 ? Pal::green() : (i == 0 ? juce::Colour (0xffff7a5c) : Pal::cyan())).withAlpha (0.6f));
        g.strokePath (a, juce::PathStrokeType (6.0f));
    }
    const float t = juce::jlimit (-1.0f, 1.0f, (p.toneSlope + 2.6f) / 2.4f);
    needle += 0.1f * (t - needle);
    const float ang = -juce::MathConstants<float>::halfPi + needle * 1.3f;
    g.setColour (Pal::text()); g.drawLine (c, base, c + std::cos (ang) * rad * 0.9f, base + std::sin (ang) * rad * 0.9f, 2.5f);
    g.fillEllipse (c - 4, base - 4, 8, 8);
    g.setFont (F (10, false)); g.setColour (Pal::dim());
    g.drawText (tr ("dark"), (int) (c - rad - 10), (int) base, 50, 14, juce::Justification::centredLeft);
    g.drawText (tr ("bright"), (int) (c + rad - 40), (int) base, 50, 14, juce::Justification::centredRight);
    g.setColour (std::abs (needle) < 0.34f ? Pal::green() : Pal::text());
    g.drawText (std::abs (needle) < 0.34f ? tr ("balanced") : (needle < 0 ? tr ("dark") : tr ("bright")), (int) (c - 50), (int) (base - rad * 0.5f), 100, 14, juce::Justification::centred);
}

void Path::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xff070516)); g.fillRoundedRectangle (r, 8);
    g.setColour (Pal::dim()); g.setFont (F (10)); g.drawText (tr ("chain"), r.removeFromTop (15).reduced (8, 0).toNearestInt(), juce::Justification::centredLeft);
    auto& E = p.engine;
    struct St { const char* n; float act; juce::Colour c; juce::String v; };
    const float in = p.apvts.getRawParameterValue ("input")->load();
    const St st[] = {
        { "IN", sat01 (std::abs (in) / 12.0f), Pal::text(), juce::String (in, 1) + " dB" },
        { "EQ", 0.6f, Pal::gold(), "" },
        { "TAME", sat01 (-E.tameAct.load() / 4.0f), Pal::pink(), juce::String (E.tameAct.load(), 1) },
        { "PUNCH", sat01 (E.punchAct.load() / 3.0f), juce::Colour (0xffff7a5c), "+" + juce::String (E.punchAct.load(), 1) },
        { "GLUE", sat01 (-(E.grBand[0].load() + E.grBand[1].load() + E.grBand[2].load()) / 9.0f), Pal::gold(),
                  juce::String (E.grBand[0].load(), 1) + "/" + juce::String (E.grBand[1].load(), 1) + "/" + juce::String (E.grBand[2].load(), 1) },
        { "WIDTH", sat01 (std::abs (E.widthNow.load() - 1.0f) * 4.0f), Pal::cyan(), juce::String (juce::roundToInt (E.widthNow.load() * 100)) + "%" },
        { "HARM", sat01 (E.satAct.load() * 8.0f), juce::Colour (0xffffc58a), "" },
        { "CLIP", sat01 (E.clipAct.load() / 1.5f), Pal::red(), juce::String (E.clipAct.load(), 1) },
        { "LIMIT", sat01 (-E.grLimit.load() / 6.0f), Pal::green(), juce::String (E.grLimit.load(), 1) },
        { "OUT", 0.5f, Pal::text(), juce::String (p.truePeakDb.load(), 1) + " TP" },
    };
    const int nSt = (int) (sizeof (st) / sizeof (st[0]));
    auto row = r.reduced (6, 4);
    const float w = row.getWidth() / (float) nSt;
    for (int i = 0; i < nSt; ++i)
    {
        auto b = juce::Rectangle<float> (row.getX() + w * (float) i, row.getY(), w, row.getHeight()).reduced (3, 2);
        g.setColour (st[i].c.withAlpha (0.08f + 0.5f * st[i].act)); g.fillRoundedRectangle (b, 6);
        g.setColour (st[i].c.withAlpha (0.35f + 0.6f * st[i].act)); g.drawRoundedRectangle (b, 6, 1);
        g.setColour (Pal::text()); g.setFont (F (11)); g.drawText (st[i].n, b.removeFromTop (b.getHeight() * 0.55f).toNearestInt(), juce::Justification::centredBottom);
        g.setColour (Pal::dim()); g.setFont (F (9.5f, false)); g.drawText (st[i].v, b.toNearestInt(), juce::Justification::centredTop);
        if (i < nSt - 1) { g.setColour (Pal::dim()); g.drawText (">", (int) (row.getX() + w * (float) (i + 1) - 4), (int) row.getCentreY() - 6, 8, 12, juce::Justification::centred); }
    }
}

void Readout::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xff070516)); g.fillRoundedRectangle (r, 8);
    if (++slow >= 10) { slow = 0; integ = p.getIntegrated(); range = p.getRange(); }
    const float st = p.meterOut.shortTerm.load();
    auto top = r.removeFromTop (60);
    g.setColour (Pal::text()); g.setFont (F (34));
    g.drawText (st > -60 ? juce::String (st, 1) : juce::String ("--.-"), top.removeFromTop (40).toNearestInt(), juce::Justification::centred);
    g.setColour (Pal::dim()); g.setFont (F (10.5f)); g.drawText ("LUFS " + tr ("short"), top.toNearestInt(), juce::Justification::centred);
    auto line = [&] (const juce::String& k, const juce::String& v, juce::Colour c)
    {
        auto l = r.removeFromTop (17).reduced (10, 0);
        g.setColour (Pal::dim()); g.setFont (F (11, false)); g.drawText (k, l.toNearestInt(), juce::Justification::centredLeft);
        g.setColour (c); g.setFont (F (11.5f)); g.drawText (v, l.toNearestInt(), juce::Justification::centredRight);
    };
    const float m = p.meterOut.momentary.load(), tp = p.truePeakMax.load();
    line (tr ("momentary"), m > -60 ? juce::String (m, 1) + " LUFS" : "-", Pal::text());
    line (tr ("integrated"), integ > -60 ? juce::String (integ, 1) + " LUFS" : "-", Pal::gold());
    line (tr ("truepeak") + " (max)", tp > -60 ? juce::String (tp, 2) + " dBTP" : "-", tp > p.apvts.getRawParameterValue ("ceiling")->load() + 0.05f ? Pal::red() : Pal::green());
    line (tr ("range"), juce::String (range, 1) + " LU", Pal::text());
    line (tr ("dynamics"), integ > -60 ? juce::String (tp - integ, 1) + " dB" : "-", Pal::text());
}

// ───────────────────────────── editor ─────────────────────────────
OmegaEditor::K* OmegaEditor::knob (std::vector<std::unique_ptr<K>>& v, const char* id, const char* key, juce::Colour c, const char* suffix)
{
    auto k = std::make_unique<K>();
    k->key = key;
    k->s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 15);
    k->s.setColour (juce::Slider::rotarySliderFillColourId, c);
    k->s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    juce::ignoreUnused (suffix);
    k->a = std::make_unique<SA> (proc.apvts, id, k->s);
    k->l.setJustificationType (juce::Justification::centred); k->l.setFont (F (11)); k->l.setColour (juce::Label::textColourId, Pal::dim());
    addAndMakeVisible (k->s); addAndMakeVisible (k->l);
    v.push_back (std::move (k));
    return v.back().get();
}

OmegaEditor::OmegaEditor (OmegaProcessor& p)
    : AudioProcessorEditor (p), proc (p), galaxy (p), compass (p), path (p), readout (p)
{
    setLookAndFeel (&lnf);
    for (auto* c : std::initializer_list<juce::Component*> { &galaxy, &inMeter, &outMeter, &compass, &path, &readout }) addAndMakeVisible (c);
    inMeter.value = [this] { return proc.meterIn.momentary.load() + proc.apvts.getRawParameterValue ("input")->load(); };
    inMeter.zoneLo = -21; inMeter.zoneHi = -13; inMeter.col = Pal::cyan();
    outMeter.value = [this] { return proc.meterOut.shortTerm.load(); };
    outMeter.zoneLo = 0; outMeter.zoneHi = 0; outMeter.col = Pal::gold();

    auto tb = [this] (juce::TextButton& b, juce::Colour c, bool toggle = false) { b.setColour (juce::TextButton::buttonOnColourId, c); b.setClickingTogglesState (toggle); addAndMakeVisible (b); };
    for (int i = 0; i < kNumUniverses; ++i)
    {
        tb (uni[i], juce::Colour (kUniverses[i].colour));
        uni[i].onClick = [this, i] { if (auto* prm = proc.apvts.getParameter ("universe")) { prm->beginChangeGesture(); prm->setValueNotifyingHost (prm->convertTo0to1 ((float) i)); prm->endChangeGesture(); } };
    }
    tb (snapA, Pal::gold()); tb (snapB, Pal::gold());
    snapA.setButtonText ("A"); snapB.setButtonText ("B");
    snapA.onClick = [this] { if (proc.currentSnapshot != 0) { proc.storeSnapshot (1); proc.recallSnapshot (0); proc.currentSnapshot = 0; } };
    snapB.onClick = [this] { if (proc.currentSnapshot != 1) { proc.storeSnapshot (0); proc.recallSnapshot (1); proc.currentSnapshot = 1; } };
    tb (undoBtn, Pal::cyan()); tb (redoBtn, Pal::cyan());
    undoBtn.onClick = [this] { proc.undo.undo(); }; redoBtn.onClick = [this] { proc.undo.redo(); };
    tb (licBtn, Pal::gold()); licBtn.onClick = [this] { showLicence(); };
    tb (langBtn, Pal::cyan()); langBtn.onClick = [this] { langFlag() = langFlag().load() ? 0 : 1; };
    tb (helpBtn, Pal::gold()); helpBtn.setButtonText ("?"); helpBtn.onClick = [this] { help->setVisible (true); help->toFront (false); };
    tb (autoIn, Pal::cyan()); autoIn.onClick = [this] { proc.startAutoInput(); };
    tb (learnLoud, Pal::gold()); learnLoud.onClick = [this] { proc.startLoudnessMatch(); };
    tb (balanceBtn, Pal::green()); balanceBtn.onClick = [this] { proc.balanceTone(); };
    tb (compareBtn, Pal::cyan(), true); tb (deltaBtn, Pal::pink(), true);
    cmpAtt = std::make_unique<BA> (proc.apvts, "compare", compareBtn);
    delAtt = std::make_unique<BA> (proc.apvts, "delta", deltaBtn);
    tb (advBtn, Pal::gold(), true); advBtn.onClick = [this] { advanced = advBtn.getToggleState(); resized(); };
    tb (loadRef, Pal::gold());
    loadRef.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> (tr ("loadRef"), juce::File::getSpecialLocation (juce::File::userMusicDirectory), "*.wav;*.aif;*.aiff;*.flac;*.mp3;*.ogg;*.m4a");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) proc.loadReference (fc.getResult()); });
    };
    tb (renderBtn, Pal::cyan());
    renderBtn.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> (tr ("surround"), juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("Constellation Surround"), "*.wav");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) proc.renderFile (fc.getResult()); });
    };
    addAndMakeVisible (target); target.addItemList ({ "Off", "Streaming -14", "Apple Music -16", "Loud -10", "Club -7" }, 1);
    tgtAtt = std::make_unique<CA> (proc.apvts, "target", target);
    addAndMakeVisible (quality); quality.addItemList ({ "2x", "4x", "8x" }, 1);
    qAtt = std::make_unique<CA> (proc.apvts, "quality", quality);
    for (auto* l : { &refLabel, &renderLabel, &streamLabel }) { addAndMakeVisible (*l); l->setFont (F (11, false)); l->setColour (juce::Label::textColourId, Pal::dim()); }
    renderLabel.addMouseListener (this, false);

    knob (mainKnobs, "input", "inGain", Pal::cyan(), " dB");
    knob (mainKnobs, "output", "outGain", Pal::text(), " dB");
    knob (mainKnobs, "ceiling", "ceiling", Pal::green(), " dBTP");
    knob (mainKnobs, "drive", "drive", Pal::gold(), " dB");
    knob (advKnobs, "low", "eqLow", juce::Colour (0xffff7a5c), " dB");
    knob (advKnobs, "mid", "eqMid", Pal::gold(), " dB");
    knob (advKnobs, "high", "eqHigh", Pal::cyan(), " dB");
    knob (advKnobs, "air", "eqAir", juce::Colour (0xffd6f6ff), " dB");
    knob (advKnobs, "tame", "tame", Pal::pink());
    knob (advKnobs, "punch", "punch", juce::Colour (0xffff7a5c));
    knob (advKnobs, "glue", "glue", Pal::gold());
    knob (advKnobs, "harmonics", "harmonics", juce::Colour (0xffffc58a));
    knob (advKnobs, "width", "width", Pal::cyan());
    knob (advKnobs, "clip", "clip", Pal::red());
    knob (advKnobs, "bassmono", "bassMono", Pal::green(), " Hz");
    knob (advKnobs, "match", "match", Pal::gold());

    help = std::make_unique<HelpOverlay>(); addChildComponent (*help);
    auto lp = std::make_unique<LicencePanel>(); lp->onDone = [this] { refreshTexts(); }; lic = std::move (lp); addChildComponent (*lic);
    if (OmegaProcessor::licence().expired()) juce::MessageManager::callAsync ([sp = juce::Component::SafePointer<OmegaEditor> (this)] { if (sp) sp->showLicence(); });

    refreshTexts();
    setResizable (true, true); setResizeLimits (1000, 660, 2200, 1400);
    setSize (1220, 780);
    startTimerHz (30);
}

OmegaEditor::~OmegaEditor() { stopTimer(); setLookAndFeel (nullptr); }

void OmegaEditor::showLicence()
{
    static_cast<LicencePanel*> (lic.get())->refresh();
    lic->setBounds (getLocalBounds()); lic->setVisible (true); lic->toFront (true);
}

void OmegaEditor::filesDropped (const juce::StringArray& files, int, int)
{
    if (! files.isEmpty()) proc.loadReference (juce::File (files[0]));
}

void OmegaEditor::refreshTexts()
{
    lastLang = langFlag().load();
    const bool es = lastLang == 1;
    for (int i = 0; i < kNumUniverses; ++i) uni[i].setButtonText (juce::String::fromUTF8 (es ? kUniverses[i].es : kUniverses[i].en));
    undoBtn.setButtonText (tr ("undo")); redoBtn.setButtonText (tr ("redo")); langBtn.setButtonText (tr ("lang"));
    autoIn.setButtonText (tr ("autoInput")); learnLoud.setButtonText (tr ("learnLoud")); balanceBtn.setButtonText (tr ("balance"));
    compareBtn.setButtonText (tr ("compare")); deltaBtn.setButtonText (tr ("delta"));
    advBtn.setButtonText (advanced ? tr ("simple") : tr ("advanced"));
    loadRef.setButtonText (tr ("loadRef")); renderBtn.setButtonText (tr ("surround"));
    for (auto* v : { &mainKnobs, &advKnobs }) for (auto& k : *v) k->l.setText (tr (k->key), juce::dontSendNotification);
    inMeter.label = tr ("input"); outMeter.label = tr ("output");
    if (help) help->repaint();
    if (lic) static_cast<LicencePanel*> (lic.get())->refresh();
    repaint();
}

void OmegaEditor::timerCallback()
{
    if (lastLang != langFlag().load()) refreshTexts();
    galaxy.tick (1.0f / 30.0f);
    const int u = (int) proc.apvts.getRawParameterValue ("universe")->load();
    for (int i = 0; i < kNumUniverses; ++i) uni[i].setToggleState (i == u, juce::dontSendNotification);
    snapA.setToggleState (proc.currentSnapshot == 0, juce::dontSendNotification);
    snapB.setToggleState (proc.currentSnapshot == 1, juce::dontSendNotification);
    autoIn.setButtonText (proc.isAutoInput() ? tr ("learning") : tr ("autoInput"));
    autoIn.setToggleState (proc.isAutoInput(), juce::dontSendNotification);
    learnLoud.setButtonText (proc.isLoudMatch() ? tr ("learning") : tr ("learnLoud"));
    learnLoud.setToggleState (proc.isLoudMatch(), juce::dontSendNotification);
    const int ti = (int) proc.apvts.getRawParameterValue ("target")->load();
    outMeter.target = ti > 0 ? OmegaProcessor::targetLufs (ti) : -100.0f;
    const float integ = readout.integ;
    if (integ > -60)
    {
        const float over = integ - (-14.0f);
        streamLabel.setText (over > 0.5f ? tr ("stream").replace ("%s", juce::String (over, 1)) : tr ("streamOk"), juce::dontSendNotification);
        streamLabel.setColour (juce::Label::textColourId, over > 0.5f ? Pal::gold() : Pal::green());
    }
    else streamLabel.setText ({}, juce::dontSendNotification);
    refLabel.setText (proc.hasRef ? juce::String::fromUTF8 ("\xe2\x98\x85 ") + proc.refName + "  (" + juce::String (proc.refLufs, 1) + " LUFS)" : tr ("noRef"), juce::dontSendNotification);
    const float rp = proc.renderProgress.load();
    renderLabel.setText (rp < 0 ? juce::String() : (rp > 1.5f ? tr ("rendered") + juce::File (proc.renderResult).getFileName() : tr ("rendering") + " " + juce::String (juce::roundToInt (rp * 100)) + "%"),
                         juce::dontSendNotification);
    auto& L = OmegaProcessor::licence();
    licBtn.setVisible (! L.licensed.load());
    licBtn.setButtonText (L.expired() ? tr ("buy") : tr ("trial").replace ("%d", juce::String (L.daysLeft.load())));
    licBtn.setToggleState (L.expired(), juce::dontSendNotification);
    for (auto* c : std::initializer_list<juce::Component*> { &inMeter, &outMeter, &compass, &path, &readout }) c->repaint();
}

void OmegaEditor::paint (juce::Graphics& g)
{
    g.fillAll (Pal::bg());
    auto top = getLocalBounds().removeFromTop (58);
    juce::ColourGradient tg (juce::Colour (0xff120c33), 0, 0, juce::Colour (0xff06041a), 0, 58, false);
    g.setGradientFill (tg); g.fillRect (top);
    auto glyph = omegaGlyph ({ 14, 10, 36, 38 });
    juce::ColourGradient og (juce::Colours::white, 32, 14, Pal::gold(), 32, 48, false);
    g.setGradientFill (og); g.fillPath (glyph);
    g.setColour (Pal::text()); g.setFont (F (24)); g.drawText ("OMEGA", 60, 8, 200, 28, juce::Justification::centredLeft);
    g.setColour (Pal::dim()); g.setFont (F (10.5f, false)); g.drawText (juce::String::fromUTF8 ("by Polaris Sound Lab  \xc2\xb7  v1.0"), 61, 34, 220, 14, juce::Justification::centredLeft);
    g.setColour (Pal::line()); g.drawLine (0, 58, (float) getWidth(), 58);
    auto foot = getLocalBounds().removeFromBottom (26);
    g.setColour (Pal::dim()); g.setFont (F (10.5f, false)); g.drawText (tr ("footer"), foot.reduced (14, 0), juce::Justification::centredLeft);
}

void OmegaEditor::mouseDown (const juce::MouseEvent& e)
{
    if (e.eventComponent == &renderLabel && proc.renderProgress.load() > 1.5f) juce::File (proc.renderResult).revealToUser();
}

void OmegaEditor::resized()
{
    auto r = getLocalBounds();
    auto top = r.removeFromTop (58).reduced (10, 12);
    top.removeFromLeft (230);
    helpBtn.setBounds (top.removeFromRight (34)); top.removeFromRight (5);
    langBtn.setBounds (top.removeFromRight (40)); top.removeFromRight (5);
    licBtn.setBounds (top.removeFromRight (160)); top.removeFromRight (12);
    redoBtn.setBounds (top.removeFromRight (64)); top.removeFromRight (4);
    undoBtn.setBounds (top.removeFromRight (74)); top.removeFromRight (12);
    snapB.setBounds (top.removeFromRight (34)); top.removeFromRight (4);
    snapA.setBounds (top.removeFromRight (34)); top.removeFromRight (12);
    const int uw = top.getWidth() / kNumUniverses;
    for (int i = 0; i < kNumUniverses; ++i) uni[i].setBounds (top.removeFromLeft (uw).reduced (2, 0));

    r.removeFromBottom (26);
    auto bottom = r.removeFromBottom (54).reduced (12, 8);
    advBtn.setBounds (bottom.removeFromRight (130));
    bottom.removeFromRight (10);
    compareBtn.setBounds (bottom.removeFromLeft (260)); bottom.removeFromLeft (8);
    deltaBtn.setBounds (bottom.removeFromLeft (170)); bottom.removeFromLeft (14);
    streamLabel.setBounds (bottom);

    // left: input
    auto left = r.removeFromLeft (230).reduced (12, 10);
    auto lk = left.removeFromTop (110);
    mainKnobs[0]->l.setBounds (lk.removeFromTop (14)); mainKnobs[0]->s.setBounds (lk.removeFromLeft (100));
    inMeter.setBounds (lk.reduced (6, 0));
    left.removeFromTop (6);
    autoIn.setBounds (left.removeFromTop (32)); left.removeFromTop (10);
    compass.setBounds (left.removeFromTop (130)); left.removeFromTop (6);
    balanceBtn.setBounds (left.removeFromTop (30));

    // right: output
    auto right = r.removeFromRight (260).reduced (12, 10);
    readout.setBounds (right.removeFromTop (160)); right.removeFromTop (8);
    target.setBounds (right.removeFromTop (30)); right.removeFromTop (6);
    learnLoud.setBounds (right.removeFromTop (32)); right.removeFromTop (6);
    auto kr = right.removeFromTop (100);
    const int kw = kr.getWidth() / 3;
    for (int i : { 1, 2, 3 })
    {
        auto c = kr.removeFromLeft (kw);
        mainKnobs[(size_t) i]->l.setBounds (c.removeFromTop (14)); mainKnobs[(size_t) i]->s.setBounds (c);
    }
    right.removeFromTop (6);
    outMeter.setBounds (right.withTrimmedLeft (right.getWidth() / 2 - 30).withWidth (60));

    // centre
    auto centre = r.reduced (4, 10);
    if (advanced)
    {
        auto adv = centre.removeFromBottom (190);
        auto row1 = adv.removeFromTop (100);
        const int n1 = 8, w1 = row1.getWidth() / n1;
        for (int i = 0; i < n1; ++i) { auto c = row1.removeFromLeft (w1); advKnobs[(size_t) i]->l.setBounds (c.removeFromTop (14)); advKnobs[(size_t) i]->s.setBounds (c); }
        auto row2 = adv.removeFromTop (86);
        auto kk = row2.removeFromLeft (w1 * 4);
        for (int i = 8; i < 12; ++i) { auto c = kk.removeFromLeft (w1); advKnobs[(size_t) i]->l.setBounds (c.removeFromTop (14)); advKnobs[(size_t) i]->s.setBounds (c); }
        row2.removeFromLeft (10);
        auto a = row2.removeFromTop (30); loadRef.setBounds (a.removeFromLeft (170)); a.removeFromLeft (8); quality.setBounds (a.removeFromRight (70)); refLabel.setBounds (a);
        row2.removeFromTop (6);
        auto b = row2.removeFromTop (30); renderBtn.setBounds (b.removeFromLeft (230)); b.removeFromLeft (8); renderLabel.setBounds (b);
    }
    for (auto& k : advKnobs) { k->s.setVisible (advanced); k->l.setVisible (advanced); }
    for (juce::Component* c : { (juce::Component*) &loadRef, (juce::Component*) &quality, (juce::Component*) &refLabel, (juce::Component*) &renderBtn, (juce::Component*) &renderLabel }) c->setVisible (advanced);
    path.setBounds (centre.removeFromBottom (58)); centre.removeFromBottom (6);
    galaxy.setBounds (centre);
    if (help) help->setBounds (getLocalBounds());
    if (lic) lic->setBounds (getLocalBounds());
    advBtn.setButtonText (advanced ? tr ("simple") : tr ("advanced"));
}
