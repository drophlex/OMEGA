// OMEGA — editor. Graphic system: Polaris "signal print" (see Design.h). Design: Dark Funk Durcal · Polaris Sound Lab.
#include "PluginEditor.h"

using namespace om;
using namespace psl;

namespace
{
const char* kBuyOmega  = "https://www.paypal.com/cgi-bin/webscr?cmd=_xclick&business=drophlex%40gmail.com&item_name=OMEGA%20licence%20(send%20your%20e-mail)&amount=49.00&currency_code=USD&no_shipping=1";
const char* kBuyBundle = "https://www.paypal.com/cgi-bin/webscr?cmd=_xclick&business=drophlex%40gmail.com&item_name=OMEGA%20%2B%20CONSTELLATION%20licences%20(send%20your%20e-mail)&amount=69.00&currency_code=USD&no_shipping=1";
const juce::Colour KEY = C::orange;

struct UStyle { juce::Colour col; const char* kana; const char* code; };
const UStyle kU[kNumUniverses] = {
    { C::magenta, "ネビュラ", "U-01 NBL" }, { C::cyan, "パルサー", "U-02 PLS" }, { C::yellow, "クエーサー", "U-03 QSR" },
    { C::orange, "スーパーノヴァ", "U-04 SNV" }, { C::paper, "イベントホライズン", "U-05 EVH" }, { C::green, "ピュアライト", "U-06 PRL" } };

juce::String fromU8 (const char* s) { return juce::String::fromUTF8 (s); }

juce::Path omegaGlyph (juce::Rectangle<float> r, float stroke)
{
    juce::Path p; const float cx = r.getCentreX(), w = r.getWidth(), h = r.getHeight();
    const float cy = r.getY() + h * 0.45f, rad = w * 0.36f;
    p.addCentredArc (cx, cy, rad, rad, 0.0f, juce::MathConstants<float>::pi * 1.22f, juce::MathConstants<float>::pi * 2.78f, true);
    juce::Path s; juce::PathStrokeType (stroke, juce::PathStrokeType::mitered, juce::PathStrokeType::butt).createStrokedPath (s, p);
    s.addRectangle (cx - w * 0.5f, r.getBottom() - stroke, w * 0.34f, stroke);
    s.addRectangle (cx + w * 0.16f, r.getBottom() - stroke, w * 0.34f, stroke);
    return s;
}

void paperSheet (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& code, const juce::String& title, const juce::String& kana)
{
    g.setColour (C::paper); g.fillRect (r);
    auto head = r.removeFromTop (54);
    g.setColour (KEY); g.fillRect (head);
    text (g, title.toUpperCase(), head.reduced (22, 4), display (34), C::ink);
    text (g, kana, head.reduced (22, 6), jp (15), C::ink, juce::Justification::bottomRight);
    tech (g, code, head.reduced (22, 6), C::ink, juce::Justification::topRight, 9);
    regMark (g, { r.getX() + 12, r.getBottom() - 12 }, 6, C::ink);
    regMark (g, { r.getRight() - 12, r.getBottom() - 12 }, 6, C::ink);
}

struct HelpOverlay : juce::Component
{
    void paint (juce::Graphics& g) override
    {
        g.fillAll (C::ink.withAlpha (0.93f));
        auto r = getLocalBounds().reduced (70, 46).toFloat();
        paperSheet (g, r, "OMG-HLP // 01", "OMEGA - " + juce::String (langFlag().load() ? "GUIA" : "GUIDE"), fromU8 ("ガイド"));
        r.removeFromTop (54);
        g.setColour (C::ink); g.setFont (body (14.5f));
        g.drawFittedText (tr ("help"), r.reduced (28, 22).toNearestInt(), juce::Justification::topLeft, 40, 1.0f);
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
        for (auto* t : { &email, &key }) { addAndMakeVisible (*t); t->setFont (mono (14)); }
        key.setMultiLine (true); key.setReturnKeyStartsNewLine (false);
        for (auto* b : { &buy, &bundle, &activate, &close }) addAndMakeVisible (*b);
        buy.setColour (juce::TextButton::buttonOnColourId, KEY); buy.setToggleState (true, juce::dontSendNotification);
        bundle.setColour (juce::TextButton::buttonOnColourId, C::cyan); bundle.setToggleState (true, juce::dontSendNotification);
        activate.setColour (juce::TextButton::buttonOnColourId, C::ink); activate.setColour (juce::TextButton::textColourOnId, C::paper);
        activate.setToggleState (true, juce::dontSendNotification);
        buy.onClick = [] { juce::URL (kBuyOmega).launchInDefaultBrowser(); };
        bundle.onClick = [] { juce::URL (kBuyBundle).launchInDefaultBrowser(); };
        activate.onClick = [this] { ok = OmegaProcessor::activate (email.getText(), key.getText()); status = ok ? tr ("actOk") : tr ("actBad"); repaint(); if (ok && onDone) onDone(); };
        close.onClick = [this] { setVisible (false); };
        refresh();
    }
    void refresh()
    {
        buy.setButtonText (tr ("buyOmega")); bundle.setButtonText (tr ("buyBundle")); activate.setButtonText (tr ("activate")); close.setButtonText (tr ("close"));
        email.setTextToShowWhenEmpty (tr ("email"), C::grey); key.setTextToShowWhenEmpty (tr ("key"), C::grey);
        repaint();
    }
    juce::Rectangle<int> box() const { return getLocalBounds().withSizeKeepingCentre (juce::jmin (680, getWidth() - 40), juce::jmin (560, getHeight() - 40)); }
    void paint (juce::Graphics& g) override
    {
        g.fillAll (C::ink.withAlpha (0.93f));
        auto r = box().toFloat();
        paperSheet (g, r, "OMG-LIC // 15D TRIAL", tr ("licTitle"), fromU8 ("ライセンス"));
        auto in = r.reduced (26, 0); in.removeFromTop (66);
        auto& L = OmegaProcessor::licence();
        auto st = in.removeFromTop (24);
        g.setColour (L.licensed ? C::green : (L.expired() ? C::orange : C::ink)); g.fillRect (st.withWidth (12).reduced (0, 6));
        text (g, (L.licensed ? tr ("licensed") + ": " + L.owner : (L.expired() ? tr ("expired") : tr ("trial").replace ("%d", juce::String (L.daysLeft.load())))).toUpperCase(),
              st.withTrimmedLeft (20), ui (14), C::ink);
        g.setColour (C::ink); g.setFont (body (14));
        g.drawFittedText (tr ("licText"), in.removeFromTop (120).toNearestInt(), juce::Justification::topLeft, 12, 1.0f);
        if (status.isNotEmpty()) text (g, status, box().toFloat().reduced (26, 20).removeFromBottom (64).removeFromTop (22), ui (13), ok ? juce::Colour (0xff0a7a45) : KEY);
    }
    void resized() override
    {
        auto in = box().reduced (26, 20); in.removeFromTop (66 + 24 + 120 + 6);
        auto row = in.removeFromTop (40);
        buy.setBounds (row.removeFromLeft (220)); row.removeFromLeft (10); bundle.setBounds (row.removeFromLeft (320));
        in.removeFromTop (14); email.setBounds (in.removeFromTop (34)); in.removeFromTop (8); key.setBounds (in.removeFromTop (70));
        auto b = in.removeFromBottom (40);
        close.setBounds (b.removeFromRight (120)); b.removeFromRight (10); activate.setBounds (b.removeFromRight (170));
    }
    void mouseDown (const juce::MouseEvent& e) override { if (! box().contains (e.getPosition())) setVisible (false); }
};
} // namespace

// ───────────────────────────── Galaxy: flat, technical, alive ─────────────────────────────
Galaxy::Galaxy (OmegaProcessor& pr) : p (pr)
{
    juce::Random r (21);
    for (int i = 0; i < 900; ++i)
    {
        const int arm = i % 3;
        const float rr = std::pow (r.nextFloat(), 0.75f);
        stars.push_back ({ rr, (float) arm * 2.0944f + rr * 4.4f + (r.nextFloat() - 0.5f) * (0.35f + rr * 0.5f), (r.nextFloat() - 0.5f) * 0.06f,
                           r.nextFloat(), r.nextFloat() * 6.28f, arm });
    }
    for (int i = 0; i < 140; ++i) bg.push_back ({ r.nextFloat(), r.nextFloat(), r.nextFloat(), r.nextFloat() * 6.28f });
    setOpaque (true);
}

void Galaxy::setAmount (float v) { if (auto* prm = p.apvts.getParameter ("amount")) prm->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, v)); }
void Galaxy::mouseDown (const juce::MouseEvent& e) { if (auto* prm = p.apvts.getParameter ("amount")) { prm->beginChangeGesture(); dragAmount = prm->getValue(); } dragStart = (float) e.position.y; }
void Galaxy::mouseDrag (const juce::MouseEvent& e) { setAmount (dragAmount + (dragStart - (float) e.position.y) / (e.mods.isShiftDown() ? 900.0f : 250.0f)); }
void Galaxy::mouseUp (const juce::MouseEvent&) { if (auto* prm = p.apvts.getParameter ("amount")) prm->endChangeGesture(); }
void Galaxy::mouseDoubleClick (const juce::MouseEvent&) { setAmount (0.5f); }
void Galaxy::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w) { if (auto* prm = p.apvts.getParameter ("amount")) setAmount (prm->getValue() + w.deltaY * 0.15f); }

void Galaxy::tick (float dt)
{
    time += dt;
    const float e = sat01 ((p.meterOut.shortTerm.load() + 40.0f) / 34.0f);
    energy += 0.08f * (e - energy);
    gr += 0.2f * (-p.engine.grLimit.load() - gr);
    sat += 0.1f * (sat01 (p.engine.satAct.load() * 6.0f) - sat);
    width += 0.05f * (p.engine.widthNow.load() - width);
    rot += dt * (0.04f + 0.5f * energy) * (p.playing.load() ? 1.0f : 0.3f);
    tint = kU[juce::jlimit (0, kNumUniverses - 1, (int) p.apvts.getRawParameterValue ("universe")->load())].col;
    repaint();
}

void Galaxy::paint (juce::Graphics& g)
{
    const auto R0 = getLocalBounds().toFloat();
    const float W = R0.getWidth(), H = R0.getHeight();
    g.fillAll (C::ink);
    dotGrid (g, R0, 16.0f, C::ink3);
    g.setColour (C::rule);
    for (float x = 0; x < W; x += 80) { g.fillRect (x, 0.0f, 1.0f, 6.0f); g.fillRect (x, H - 6.0f, 1.0f, 6.0f); tech (g, juce::String ((int) x).paddedLeft ('0', 4), { x + 3, H - 14, 40, 10 }, C::rule, juce::Justification::centredLeft, 8); }
    for (float y = 0; y < H; y += 80) { g.fillRect (0.0f, y, 6.0f, 1.0f); g.fillRect (W - 6.0f, y, 6.0f, 1.0f); }
    for (auto& s : bg)
    {
        const bool blink = std::sin (time * (0.6f + s.s) + s.ph) > 0.2f;
        g.setColour (C::paper.withAlpha (blink ? 0.55f : 0.18f));
        const float x = s.x * W, y = s.y * H;
        if (s.s > 0.85f) { g.fillRect (x - 3, y, 7.0f, 1.0f); g.fillRect (x, y - 3, 1.0f, 7.0f); } else g.fillRect (x, y, 1.0f, 1.0f);
    }

    const int ui_ = juce::jlimit (0, kNumUniverses - 1, (int) p.apvts.getRawParameterValue ("universe")->load());
    const auto& U = kUniverses[ui_];
    const bool es = langFlag().load() == 1;
    const float amount = p.apvts.getRawParameterValue ("amount")->load();

    {
        const float base = H - 20.0f, maxH = H * 0.13f, bw = W / 64.0f;
        for (int j = 0; j < 64; ++j)
        {
            const float v = sat01 ((p.spectrumLine[(size_t) j] + 70.0f) / 60.0f);
            const float hh = v * maxH, x = bw * (float) j + bw * 0.5f;
            g.setColour (C::paper.withAlpha (0.35f));
            g.fillRect (x - 0.5f, base - hh, 1.0f, hh);
            g.setColour (tint); g.fillRect (x - 2.0f, base - hh - 1.0f, 4.0f, 2.0f);
        }
        if (p.hasRef)
        {
            float rm = 0, mm = 0; for (int b = 1; b < 9; ++b) { rm += p.refBands[(size_t) b]; mm += p.specOutLong[(size_t) b]; } rm /= 8; mm /= 8;
            g.setColour (C::yellow);
            for (int b = 0; b < 10; ++b)
            {
                const float x = W * (std::log2 (kOct[b] / 30.0f) / 9.4f);
                const float lvl = p.refBands[(size_t) b] - rm + mm + 3.0f * std::log2 (kOct[b] / 1000.0f);
                const float y = base - sat01 ((lvl + 70.0f) / 60.0f) * maxH;
                g.fillRect (x - 12, y - 1, 24.0f, 3.0f);
            }
            tech (g, "REF // " + p.refName, { 12, base - maxH - 16, W * 0.6f, 12 }, C::yellow);
        }
        g.setColour (C::rule); g.fillRect (0.0f, base + 1, W, 1.0f);
    }

    const float cx = W * 0.58f, cy = H * 0.44f;
    const float R = juce::jmin (W * 0.28f, H * 0.34f);
    const float tilt = 0.40f;
    for (int b = 0; b < 3; ++b)
    {
        const float grb = sat01 (-p.engine.grBand[b].load() / 8.0f);
        const float rr = R * (0.62f + 0.2f * (float) b);
        const juce::Colour oc = b == 0 ? C::orange : (b == 1 ? C::yellow : C::cyan);
        juce::Path e; e.addEllipse (cx - rr, cy - rr * tilt, rr * 2, rr * 2 * tilt);
        juce::Path dashed; const float d[] = { 3, 5 }; juce::PathStrokeType (1.0f).createDashedStroke (dashed, e, d, 2);
        g.setColour (C::paper.withAlpha (0.28f)); g.fillPath (dashed);
        juce::Path seg; seg.addCentredArc (cx, cy, rr, rr * tilt, 0, juce::MathConstants<float>::halfPi, juce::MathConstants<float>::halfPi + 0.05f + grb * 2.6f, true);
        g.setColour (oc); g.strokePath (seg, juce::PathStrokeType (4.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::butt));
        static const char* nm[3] = { "LO", "MID", "HI" };
        const float ly = cy + rr * tilt * 0.5f + 4.0f;
        g.setColour (oc); g.fillRect (cx + rr * 0.86f, ly + 5, 10.0f, 2.0f);
        tech (g, juce::String (nm[b]) + " " + juce::String (p.engine.grBand[b].load(), 1), { cx + rr * 0.86f + 14, ly, 80, 12 }, oc);
    }
    for (auto& s : stars)
    {
        const float spread = 1.0f + (width - 1.0f) * 1.8f;
        const float ang = s.a + rot * (1.2f - s.r * 0.7f);
        const float rr = s.r * R * (0.95f + 0.1f * energy);
        const float x = cx + std::cos (ang) * rr * spread, y = cy + std::sin (ang) * rr * tilt + s.z * R;
        const bool on = std::sin (time * 1.7f + s.tw) > -0.6f + 0.8f * (1.0f - energy);
        if (! on) continue;
        const float sz = s.r < 0.2f ? 3.0f : (s.z > 0.025f ? 2.0f : 1.5f);
        g.setColour (s.arm == 0 ? tint : (s.r < 0.25f ? C::paper : C::paper.withAlpha (0.75f)));
        if (s.arm == 1 && s.tw > 6.0f) { g.fillRect (x - 3, y, 7.0f, 1.0f); g.fillRect (x, y - 3, 1.0f, 7.0f); }
        else g.fillRect (x, y, sz, sz);
    }
    const float core = R * 0.15f;
    g.setColour (C::paper); g.fillEllipse (cx - core, cy - core * 0.62f, core * 2, core * 1.24f);
    const int rings = 1 + (int) (sat * 4.0f);
    for (int i = 1; i <= rings; ++i) { const float rr = core * (1.0f + 0.28f * (float) i); g.setColour (C::paper.withAlpha (0.5f / (float) i)); g.drawEllipse (cx - rr, cy - rr * 0.62f, rr * 2, rr * 1.24f, 1.0f); }
    const float eh = core * (0.35f + 0.13f * juce::jmin (gr, 8.0f));
    g.setColour (C::ink); g.fillEllipse (cx - eh, cy - eh * 0.62f, eh * 2, eh * 1.24f);
    if (gr > 0.05f) { g.setColour (KEY); g.drawEllipse (cx - eh - 2, cy - eh * 0.62f - 2, eh * 2 + 4, eh * 1.24f + 4, 2.0f); }

    {
        const float rr = R * 1.18f, ry = rr * 0.56f;
        for (int i = 0; i <= 20; ++i)
        {
            const float a = -2.4f + 4.8f * (float) i / 20.0f;
            const float sx = std::sin (a), sy = -std::cos (a);
            const float l = i % 5 == 0 ? 10.0f : 5.0f;
            g.setColour (i % 5 == 0 ? C::paper.withAlpha (0.7f) : C::rule);
            g.drawLine (cx + sx * rr, cy + sy * ry, cx + sx * (rr + l), cy + sy * (ry + l * 0.56f), 1.0f);
            if (i % 5 == 0) tech (g, juce::String (i * 5), { cx + sx * (rr + 22) - 14, cy + sy * (ry + 14) - 6, 28, 12 }, C::grey, juce::Justification::centred, 8);
        }
        juce::Path val; val.addCentredArc (cx, cy, rr - 4, ry - 4, 0, -2.4f, -2.4f + 4.8f * amount, true);
        g.setColour (tint); g.strokePath (val, juce::PathStrokeType (6.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::butt));
    }

    {
        sectionTab (g, { 16, 8, W - 32, 16 }, "03", es ? "UNIVERSO" : "UNIVERSE", fromU8 ("宇宙"), KEY);
        auto tb = juce::Rectangle<float> (16, 32, W * 0.36f, 150);
        tech (g, fromU8 (kU[ui_].code) + " // " + fromU8 (es ? U.tagEs : U.tagEn).toUpperCase(), tb.removeFromTop (14), tint);
        text (g, fromU8 (es ? U.es : U.en).toUpperCase(), tb.removeFromTop (52), display (54), C::paper);
        text (g, fromU8 (kU[ui_].kana), tb.removeFromTop (22), jp (15), C::grey);
        auto am = tb.removeFromTop (46);
        g.setColour (tint); g.fillRect (am.withWidth (6));
        text (g, juce::String (juce::roundToInt (amount * 100)).paddedLeft ('0', 3), am.withTrimmedLeft (14).withWidth (90), display (44), tint);
        tech (g, "% OMEGA", am.withTrimmedLeft (104).withTrimmedTop (24), C::grey);
    }
    tech (g, tr ("dragHint"), { W - 420, 32, 404, 12 }, C::grey, juce::Justification::centredRight);
    barcode (g, { W - 120, 48, 104, 14 }, 7 + ui_, C::paper.withAlpha (0.6f));
    regMark (g, { W - 12, H - 30 }, 6, C::rule);
    g.setColour (C::rule); g.drawRect (R0, 1.0f);
}

// ───────────────────────────── meters & panels ─────────────────────────────
void Meter::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (C::ink2); g.fillRect (r);
    g.setColour (C::rule); g.drawRect (r, 1.0f);
    tech (g, label, r.removeFromTop (16).reduced (4, 0), C::grey, juce::Justification::centred, 8.5f);
    const float v = value ? value() : -100.0f;
    auto num = r.removeFromBottom (24);
    text (g, v > -60 ? juce::String (v, 1) : juce::String ("--.-"), num, display (20), C::paper, juce::Justification::centred);
    auto bar = r.reduced (6, 4);
    auto scale = bar.removeFromRight (20);
    auto yOf = [&] (float lufs) { return bar.getBottom() - sat01 ((lufs + 40.0f) / 40.0f) * bar.getHeight(); };
    if (zoneHi > zoneLo) { g.setColour (C::green.withAlpha (0.25f)); g.fillRect (bar.getX(), yOf (zoneHi), bar.getWidth(), yOf (zoneLo) - yOf (zoneHi)); }
    segBar (g, bar.reduced (3, 0), sat01 ((v + 40.0f) / 40.0f), col, C::ink3, 30);
    if (target > -60) { g.setColour (C::yellow); g.fillRect (bar.getX() - 2, yOf (target) - 1, bar.getWidth() + 4, 3.0f); }
    for (int l : { -36, -24, -14, -6 }) tech (g, juce::String (-l), { scale.getX(), yOf ((float) l) - 5, 20, 10 }, C::grey, juce::Justification::centredRight, 8);
}

void Compass::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (C::ink2); g.fillRect (r); g.setColour (C::rule); g.drawRect (r, 1.0f);
    const float c = r.getCentreX(), base = r.getBottom() - 22, rad = juce::jmin (r.getWidth() * 0.40f, r.getHeight() - 34);
    const juce::Colour seg[3] = { C::orange, C::green, C::cyan };
    for (int i = 0; i < 3; ++i)
    {
        juce::Path a; const float a0 = -1.3f + (float) i * 0.8667f;
        a.addCentredArc (c, base, rad, rad, 0, a0, a0 + 0.8667f, true);
        g.setColour (seg[i]); g.strokePath (a, juce::PathStrokeType (8.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::butt));
    }
    for (int i = 0; i <= 12; ++i)
    {
        const float a = -1.3f + 2.6f * (float) i / 12.0f;
        const float sx = std::sin (a), sy = -std::cos (a);
        const float l = i % 6 == 0 ? 22.0f : 16.0f;
        g.setColour (C::paper.withAlpha (0.5f)); g.drawLine (c + sx * (rad - 12), base + sy * (rad - 12), c + sx * (rad - l), base + sy * (rad - l), 1.0f);
    }
    const float t = juce::jlimit (-1.0f, 1.0f, (p.toneSlope + 2.6f) / 2.4f);
    needle += 0.1f * (t - needle);
    const float ang = needle * 1.3f;
    g.setColour (C::paper); g.drawLine (c, base, c + std::sin (ang) * rad * 0.92f, base - std::cos (ang) * rad * 0.92f, 2.0f);
    g.fillRect (c - 4, base - 4, 8.0f, 8.0f);
    tech (g, tr ("dark"), { r.getX() + 8, base + 6, 80, 12 }, C::orange);
    tech (g, tr ("bright"), { r.getRight() - 88, base + 6, 80, 12 }, C::cyan, juce::Justification::centredRight);
    const bool ok = std::abs (needle) < 0.34f;
    text (g, (ok ? tr ("balanced") : (needle < 0 ? tr ("dark") : tr ("bright"))).toUpperCase(), { c - 60, base - rad * 0.58f, 120, 18 }, ui (14), ok ? C::green : C::paper, juce::Justification::centred);
    tech (g, juce::String (p.toneSlope, 1) + " DB/OCT", { c - 60, base - rad * 0.58f + 17, 120, 12 }, C::grey, juce::Justification::centred, 8);
}

void Path::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    auto& E = p.engine;
    struct St { const char* n; float act; juce::Colour c; juce::String v; };
    const float in = p.apvts.getRawParameterValue ("input")->load();
    const St st[] = {
        { "IN", sat01 (std::abs (in) / 12.0f), C::paper, juce::String (in, 1) },
        { "EQ", 0.5f, C::yellow, "" },
        { "TAME", sat01 (-E.tameAct.load() / 4.0f), C::magenta, juce::String (E.tameAct.load(), 1) },
        { "PUNCH", sat01 (E.punchAct.load() / 3.0f), C::orange, "+" + juce::String (E.punchAct.load(), 1) },
        { "GLUE", sat01 (-(E.grBand[0].load() + E.grBand[1].load() + E.grBand[2].load()) / 9.0f), C::yellow,
                  juce::String (E.grBand[0].load(), 1) + "/" + juce::String (E.grBand[1].load(), 1) + "/" + juce::String (E.grBand[2].load(), 1) },
        { "WIDTH", sat01 (std::abs (E.widthNow.load() - 1.0f) * 4.0f), C::cyan, juce::String (juce::roundToInt (E.widthNow.load() * 100)) + "%" },
        { "HARM", sat01 (E.satAct.load() * 8.0f), C::orange, "" },
        { "CLIP", sat01 (E.clipAct.load() / 1.5f), C::magenta, juce::String (E.clipAct.load(), 1) },
        { "LIMIT", sat01 (-E.grLimit.load() / 6.0f), C::green, juce::String (E.grLimit.load(), 1) },
        { "OUT", 0.5f, C::paper, juce::String (p.truePeakDb.load(), 1) + "TP" },
    };
    const int n = (int) (sizeof (st) / sizeof (st[0]));
    sectionTab (g, r.removeFromTop (16), "04", tr ("chain"), fromU8 ("信号"), KEY);
    r.removeFromTop (4);
    const float w = r.getWidth() / (float) n;
    for (int i = 0; i < n; ++i)
    {
        auto b = juce::Rectangle<float> (r.getX() + w * (float) i, r.getY(), w - 3, r.getHeight());
        g.setColour (C::ink2); g.fillRect (b);
        g.setColour (C::rule); g.drawRect (b, 1.0f);
        auto topr = b.removeFromTop (13);
        g.setColour (st[i].c.withAlpha (0.25f + 0.75f * st[i].act)); g.fillRect (topr);
        tech (g, juce::String (i + 1).paddedLeft ('0', 2), topr.reduced (3, 0), C::ink, juce::Justification::centredLeft, 8);
        text (g, st[i].n, b.removeFromTop (b.getHeight() * 0.5f).reduced (4, 0), ui (13), C::paper, juce::Justification::bottomLeft);
        tech (g, st[i].v, b.reduced (4, 0).withTrimmedBottom (6), C::grey, juce::Justification::topLeft, 8.5f);
        g.setColour (st[i].c); g.fillRect (b.getX() + 1, b.getBottom() - 4, (b.getWidth() - 2) * st[i].act, 3.0f);
    }
}

void Readout::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    if (++slow >= 10) { slow = 0; integ = p.getIntegrated(); range = p.getRange(); }
    g.setColour (C::ink2); g.fillRect (r); g.setColour (C::rule); g.drawRect (r, 1.0f);
    const float st = p.meterOut.shortTerm.load();
    auto top = r.removeFromTop (68).reduced (10, 4);
    text (g, st > -60 ? juce::String (st, 1) : juce::String ("--.-"), top.removeFromLeft (top.getWidth() * 0.66f), display (60), C::paper);
    tech (g, "LUFS", top.removeFromTop (top.getHeight() * 0.5f), KEY, juce::Justification::bottomLeft, 12);
    tech (g, tr ("short"), top, C::grey, juce::Justification::topLeft, 8);
    auto line = [&] (const juce::String& k, const juce::String& v, juce::Colour c)
    {
        auto l = r.removeFromTop (18).reduced (10, 0);
        const float kw = juce::GlyphArrangement::getStringWidth (mono (10), k.toUpperCase());
        tech (g, k, l, C::grey, juce::Justification::centredLeft, 10);
        text (g, v, l, mono (11, true), c, juce::Justification::centredRight);
        g.setColour (C::rule); for (float x = l.getX() + kw + 6; x < l.getRight() - 96; x += 4) g.fillRect (x, l.getCentreY() + 2, 1.0f, 1.0f);
    };
    const float m = p.meterOut.momentary.load(), tp = p.truePeakMax.load();
    line (tr ("momentary"), m > -60 ? juce::String (m, 1) + " LUFS" : "-", C::paper);
    line (tr ("integrated"), integ > -60 ? juce::String (integ, 1) + " LUFS" : "-", C::yellow);
    line (tr ("truepeak"), tp > -60 ? juce::String (tp, 2) + " dBTP" : "-", tp > p.apvts.getRawParameterValue ("ceiling")->load() + 0.05f ? KEY : C::green);
    line (tr ("range"), juce::String (range, 1) + " LU", C::paper);
    line ("PLR", integ > -60 ? juce::String (tp - integ, 1) + " dB" : "-", C::paper);
}

// ───────────────────────────── editor ─────────────────────────────
OmegaEditor::K* OmegaEditor::knob (std::vector<std::unique_ptr<K>>& v, const char* id, const char* key, juce::Colour c, const char*)
{
    auto k = std::make_unique<K>();
    k->key = key;
    k->s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 15);
    k->s.setColour (juce::Slider::rotarySliderFillColourId, c);
    k->s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    k->a = std::make_unique<SA> (proc.apvts, id, k->s);
    k->l.setJustificationType (juce::Justification::centred);
    k->l.setFont (mono (10)); k->l.setColour (juce::Label::textColourId, C::grey);
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
    inMeter.zoneLo = -21; inMeter.zoneHi = -13; inMeter.col = C::cyan;
    outMeter.value = [this] { return proc.meterOut.shortTerm.load(); };
    outMeter.zoneLo = 0; outMeter.zoneHi = 0; outMeter.col = KEY;

    auto tb = [this] (juce::TextButton& b, juce::Colour on, bool toggle = false) { b.setColour (juce::TextButton::buttonOnColourId, on); b.setClickingTogglesState (toggle); addAndMakeVisible (b); };
    for (int i = 0; i < kNumUniverses; ++i)
    {
        tb (uni[i], C::ink);
        uni[i].setColour (juce::TextButton::buttonColourId, KEY);
        uni[i].setColour (juce::TextButton::textColourOffId, C::ink);
        uni[i].setColour (juce::TextButton::textColourOnId, kU[i].col);
        uni[i].onClick = [this, i] { if (auto* prm = proc.apvts.getParameter ("universe")) { prm->beginChangeGesture(); prm->setValueNotifyingHost (prm->convertTo0to1 ((float) i)); prm->endChangeGesture(); } };
    }
    for (auto* b : { &snapA, &snapB, &undoBtn, &redoBtn, &langBtn, &helpBtn })
    {
        tb (*b, C::ink); b->setColour (juce::TextButton::buttonColourId, KEY);
        b->setColour (juce::TextButton::textColourOffId, C::ink); b->setColour (juce::TextButton::textColourOnId, C::paper);
    }
    snapA.setButtonText ("A"); snapB.setButtonText ("B");
    snapA.onClick = [this] { if (proc.currentSnapshot != 0) { proc.storeSnapshot (1); proc.recallSnapshot (0); proc.currentSnapshot = 0; } };
    snapB.onClick = [this] { if (proc.currentSnapshot != 1) { proc.storeSnapshot (0); proc.recallSnapshot (1); proc.currentSnapshot = 1; } };
    undoBtn.onClick = [this] { proc.undo.undo(); }; redoBtn.onClick = [this] { proc.undo.redo(); };
    tb (licBtn, C::yellow); licBtn.setColour (juce::TextButton::buttonColourId, C::ink); licBtn.setColour (juce::TextButton::textColourOffId, C::yellow);
    licBtn.onClick = [this] { showLicence(); };
    langBtn.onClick = [this] { langFlag() = langFlag().load() ? 0 : 1; };
    helpBtn.setButtonText ("?"); helpBtn.onClick = [this] { help->setVisible (true); help->toFront (false); };
    tb (autoIn, C::cyan); autoIn.onClick = [this] { proc.startAutoInput(); };
    tb (learnLoud, KEY); learnLoud.onClick = [this] { proc.startLoudnessMatch(); };
    tb (balanceBtn, C::green); balanceBtn.onClick = [this] { proc.balanceTone(); };
    tb (compareBtn, C::cyan, true); tb (deltaBtn, C::magenta, true);
    cmpAtt = std::make_unique<BA> (proc.apvts, "compare", compareBtn);
    delAtt = std::make_unique<BA> (proc.apvts, "delta", deltaBtn);
    tb (advBtn, C::paper, true); advBtn.onClick = [this] { advanced = advBtn.getToggleState(); resized(); repaint(); };
    tb (loadRef, C::yellow);
    loadRef.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> (tr ("loadRef"), juce::File::getSpecialLocation (juce::File::userMusicDirectory), "*.wav;*.aif;*.aiff;*.flac;*.mp3;*.ogg;*.m4a");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) proc.loadReference (fc.getResult()); });
    };
    tb (renderBtn, C::cyan);
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
    for (auto* l : { &refLabel, &renderLabel, &streamLabel }) { addAndMakeVisible (*l); l->setFont (mono (10.5f)); l->setColour (juce::Label::textColourId, C::grey); }
    renderLabel.addMouseListener (this, false);

    knob (mainKnobs, "input", "inGain", C::cyan);
    knob (mainKnobs, "output", "outGain", C::paper);
    knob (mainKnobs, "ceiling", "ceiling", C::green);
    knob (mainKnobs, "drive", "drive", KEY);
    knob (advKnobs, "low", "eqLow", C::orange);
    knob (advKnobs, "mid", "eqMid", C::yellow);
    knob (advKnobs, "high", "eqHigh", C::cyan);
    knob (advKnobs, "air", "eqAir", C::paper);
    knob (advKnobs, "tame", "tame", C::magenta);
    knob (advKnobs, "punch", "punch", C::orange);
    knob (advKnobs, "glue", "glue", C::yellow);
    knob (advKnobs, "harmonics", "harmonics", C::orange);
    knob (advKnobs, "width", "width", C::cyan);
    knob (advKnobs, "clip", "clip", C::magenta);
    knob (advKnobs, "bassmono", "bassMono", C::green);
    knob (advKnobs, "match", "match", C::yellow);

    help = std::make_unique<HelpOverlay>(); addChildComponent (*help);
    auto lp = std::make_unique<LicencePanel>(); lp->onDone = [this] { refreshTexts(); }; lic = std::move (lp); addChildComponent (*lic);
    if (OmegaProcessor::licence().expired()) juce::MessageManager::callAsync ([sp = juce::Component::SafePointer<OmegaEditor> (this)] { if (sp) sp->showLicence(); });

    refreshTexts();
    setResizable (true, true); setResizeLimits (1060, 700, 2200, 1400);
    setSize (1240, 800);
    startTimerHz (30);
}

OmegaEditor::~OmegaEditor() { stopTimer(); setLookAndFeel (nullptr); }

void OmegaEditor::showLicence() { static_cast<LicencePanel*> (lic.get())->refresh(); lic->setBounds (getLocalBounds()); lic->setVisible (true); lic->toFront (true); }
void OmegaEditor::filesDropped (const juce::StringArray& files, int, int) { if (! files.isEmpty()) proc.loadReference (juce::File (files[0])); }

void OmegaEditor::refreshTexts()
{
    lastLang = langFlag().load();
    const bool es = lastLang == 1;
    for (int i = 0; i < kNumUniverses; ++i) uni[i].setButtonText (fromU8 (es ? kUniverses[i].es : kUniverses[i].en));
    undoBtn.setButtonText (tr ("undo")); redoBtn.setButtonText (tr ("redo")); langBtn.setButtonText (tr ("lang"));
    autoIn.setButtonText (tr ("autoInput")); learnLoud.setButtonText (tr ("learnLoud")); balanceBtn.setButtonText (tr ("balance"));
    compareBtn.setButtonText (tr ("compare")); deltaBtn.setButtonText (tr ("delta"));
    advBtn.setButtonText (advanced ? tr ("simple") : tr ("advanced"));
    loadRef.setButtonText (tr ("loadRef")); renderBtn.setButtonText (tr ("surround"));
    for (auto* v : { &mainKnobs, &advKnobs }) for (auto& k : *v) k->l.setText (tr (k->key).toUpperCase(), juce::dontSendNotification);
    inMeter.label = "LUFS-M"; outMeter.label = "LUFS-S";
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
    autoIn.setButtonText (proc.isAutoInput() ? tr ("learning") : tr ("autoInput")); autoIn.setToggleState (proc.isAutoInput(), juce::dontSendNotification);
    learnLoud.setButtonText (proc.isLoudMatch() ? tr ("learning") : tr ("learnLoud")); learnLoud.setToggleState (proc.isLoudMatch(), juce::dontSendNotification);
    const int ti = (int) proc.apvts.getRawParameterValue ("target")->load();
    outMeter.target = ti > 0 ? OmegaProcessor::targetLufs (ti) : -100.0f;
    const float integ = readout.integ;
    if (integ > -60)
    {
        const float over = integ - (-14.0f);
        streamLabel.setText ((over > 0.5f ? tr ("stream").replace ("%s", juce::String (over, 1)) : tr ("streamOk")).toUpperCase(), juce::dontSendNotification);
        streamLabel.setColour (juce::Label::textColourId, over > 0.5f ? C::yellow : C::green);
    }
    else streamLabel.setText ({}, juce::dontSendNotification);
    refLabel.setText (proc.hasRef ? "REF // " + proc.refName + "  " + juce::String (proc.refLufs, 1) + " LUFS" : tr ("noRef").toUpperCase(), juce::dontSendNotification);
    const float rp = proc.renderProgress.load();
    renderLabel.setText (rp < 0 ? juce::String() : (rp > 1.5f ? tr ("rendered") + juce::File (proc.renderResult).getFileName() : tr ("rendering") + " " + juce::String (juce::roundToInt (rp * 100)) + "%"), juce::dontSendNotification);
    auto& L = OmegaProcessor::licence();
    licBtn.setVisible (! L.licensed.load());
    licBtn.setButtonText (L.expired() ? tr ("buy") : tr ("trial").replace ("%d", juce::String (L.daysLeft.load())));
    licBtn.setToggleState (L.expired(), juce::dontSendNotification);
    for (auto* c : std::initializer_list<juce::Component*> { &inMeter, &outMeter, &compass, &path, &readout }) c->repaint();
}

void OmegaEditor::paint (juce::Graphics& g)
{
    const auto R = getLocalBounds().toFloat();
    g.fillAll (C::ink);
    auto top = R.withHeight (64.0f);
    g.setColour (KEY); g.fillRect (top);
    g.setColour (C::ink); g.fillPath (omegaGlyph ({ 14, 12, 38, 40 }, 7.0f));
    text (g, "OMEGA", { 60, 4, 190, 54 }, display (54), C::ink);
    const float tx = 64.0f + juce::GlyphArrangement::getStringWidth (display (54), "OMEGA") + 10.0f;
    text (g, fromU8 ("オメガ"), { tx, 8, 80, 22 }, jp (15), C::ink);
    tech (g, "MSTR.PRCSSR", { tx, 30, 130, 12 }, C::ink, juce::Justification::centredLeft, 8.5f);
    tech (g, "PSL // MID.YUC // V1.1", { tx, 42, 140, 12 }, C::ink, juce::Justification::centredLeft, 8.5f);
    g.setColour (C::ink); g.fillRect (0.0f, 64.0f, R.getWidth(), 2.0f);
    auto L = R.withTrimmedTop (66).withWidth (240).reduced (12, 10);
    sectionTab (g, L.withHeight (16), "01", tr ("input"), fromU8 ("入力"), KEY);
    sectionTab (g, { L.getX(), (float) compass.getY() - 24, L.getWidth(), 16 }, "02", tr ("tone"), fromU8 ("周波数"), KEY);
    auto Rr = R.withTrimmedTop (66).withLeft (R.getRight() - 272).reduced (12, 10);
    sectionTab (g, Rr.withHeight (16), "05", tr ("output"), fromU8 ("音量"), KEY);
    if (advanced) sectionTab (g, { (float) galaxy.getX(), (float) path.getBottom() + 10, (float) galaxy.getWidth(), 16 }, "06", tr ("advanced"), fromU8 ("設定"), KEY);
    auto foot = R.withTop (R.getBottom() - 26);
    g.setColour (C::ink2); g.fillRect (foot);
    g.setColour (C::rule); g.fillRect (foot.withHeight (1));
    tech (g, tr ("footer"), foot.reduced (14, 0), C::grey, juce::Justification::centredLeft, 9);
    barcode (g, { foot.getRight() - 160, foot.getY() + 7, 120, 12 }, 99, C::grey);
    hatch (g, { foot.getRight() - 30, foot.getY() + 4, 18, 18 }, KEY, 4);
}

void OmegaEditor::mouseDown (const juce::MouseEvent& e)
{
    if (e.eventComponent == &renderLabel && proc.renderProgress.load() > 1.5f) juce::File (proc.renderResult).revealToUser();
}

void OmegaEditor::resized()
{
    auto r = getLocalBounds();
    auto top = r.removeFromTop (66).withTrimmedBottom (2).reduced (10, 14);
    top.removeFromLeft (336);
    helpBtn.setBounds (top.removeFromRight (34)); top.removeFromRight (4);
    langBtn.setBounds (top.removeFromRight (40)); top.removeFromRight (4);
    licBtn.setBounds (top.removeFromRight (176)); top.removeFromRight (10);
    redoBtn.setBounds (top.removeFromRight (62)); top.removeFromRight (4);
    undoBtn.setBounds (top.removeFromRight (70)); top.removeFromRight (10);
    snapB.setBounds (top.removeFromRight (32)); top.removeFromRight (4);
    snapA.setBounds (top.removeFromRight (32)); top.removeFromRight (10);
    const int uw = top.getWidth() / kNumUniverses;
    for (int i = 0; i < kNumUniverses; ++i) uni[i].setBounds (top.removeFromLeft (uw).reduced (2, 0));

    r.removeFromBottom (26);
    auto bottom = r.removeFromBottom (56).reduced (12, 8);
    advBtn.setBounds (bottom.removeFromRight (140)); bottom.removeFromRight (10);
    compareBtn.setBounds (bottom.removeFromLeft (280)); bottom.removeFromLeft (6);
    deltaBtn.setBounds (bottom.removeFromLeft (180)); bottom.removeFromLeft (14);
    streamLabel.setBounds (bottom);

    auto left = r.removeFromLeft (240).reduced (12, 10);
    left.removeFromTop (24);
    auto lk = left.removeFromTop (140);
    mainKnobs[0]->l.setBounds (lk.withWidth (110).removeFromTop (14)); mainKnobs[0]->s.setBounds (lk.withWidth (110).withTrimmedTop (14));
    inMeter.setBounds (lk.withTrimmedLeft (118));
    left.removeFromTop (8);
    autoIn.setBounds (left.removeFromTop (32)); left.removeFromTop (32);
    compass.setBounds (left.removeFromTop (140)); left.removeFromTop (6);
    balanceBtn.setBounds (left.removeFromTop (30));

    auto right = r.removeFromRight (272).reduced (12, 10);
    right.removeFromTop (24);
    readout.setBounds (right.removeFromTop (166)); right.removeFromTop (8);
    target.setBounds (right.removeFromTop (30)); right.removeFromTop (6);
    learnLoud.setBounds (right.removeFromTop (32)); right.removeFromTop (8);
    auto kr = right.removeFromTop (104);
    const int kw = kr.getWidth() / 3;
    for (int i : { 1, 2, 3 }) { auto c = kr.removeFromLeft (kw); mainKnobs[(size_t) i]->l.setBounds (c.removeFromTop (14)); mainKnobs[(size_t) i]->s.setBounds (c); }
    right.removeFromTop (6);
    outMeter.setBounds (right.withSizeKeepingCentre (90, right.getHeight()));

    auto centre = r.reduced (4, 10);
    if (advanced)
    {
        auto adv = centre.removeFromBottom (220);
        adv.removeFromTop (28);
        auto row1 = adv.removeFromTop (100);
        const int n1 = 8, w1 = row1.getWidth() / n1;
        for (int i = 0; i < n1; ++i) { auto c = row1.removeFromLeft (w1); advKnobs[(size_t) i]->l.setBounds (c.removeFromTop (14)); advKnobs[(size_t) i]->s.setBounds (c); }
        auto row2 = adv.removeFromTop (88);
        auto kk = row2.removeFromLeft (w1 * 4);
        for (int i = 8; i < 12; ++i) { auto c = kk.removeFromLeft (w1); advKnobs[(size_t) i]->l.setBounds (c.removeFromTop (14)); advKnobs[(size_t) i]->s.setBounds (c); }
        row2.removeFromLeft (10);
        auto a = row2.removeFromTop (30); loadRef.setBounds (a.removeFromLeft (200)); a.removeFromLeft (8); quality.setBounds (a.removeFromRight (74)); refLabel.setBounds (a);
        row2.removeFromTop (6);
        auto b = row2.removeFromTop (30); renderBtn.setBounds (b.removeFromLeft (270)); b.removeFromLeft (8); renderLabel.setBounds (b);
    }
    for (auto& k : advKnobs) { k->s.setVisible (advanced); k->l.setVisible (advanced); }
    for (juce::Component* c : { (juce::Component*) &loadRef, (juce::Component*) &quality, (juce::Component*) &refLabel, (juce::Component*) &renderBtn, (juce::Component*) &renderLabel }) c->setVisible (advanced);
    path.setBounds (centre.removeFromBottom (72)); centre.removeFromBottom (8);
    galaxy.setBounds (centre);
    if (help) help->setBounds (getLocalBounds());
    if (lic) lic->setBounds (getLocalBounds());
    advBtn.setButtonText (advanced ? tr ("simple") : tr ("advanced"));
}
