#include "PluginEditor.h"
#include "BinaryData.h"

using namespace dfdui;
using namespace dfd;

// ================================================================ fonts and look
Fonts& Fonts::get() {
    static Fonts f;
    if (!f.cinzel) {
        f.cinzel = juce::Typeface::createSystemTypefaceFor(BinaryData::Cinzel_ttf, BinaryData::Cinzel_ttfSize);
        f.barlow = juce::Typeface::createSystemTypefaceFor(BinaryData::BarlowCondensedSemiBold_ttf, BinaryData::BarlowCondensedSemiBold_ttfSize);
        f.barlowBold = juce::Typeface::createSystemTypefaceFor(BinaryData::BarlowCondensedBold_ttf, BinaryData::BarlowCondensedBold_ttfSize);
    }
    return f;
}
juce::Font Fonts::title(float h) { return juce::Font(juce::FontOptions(cinzel).withHeight(h)); }
juce::Font Fonts::text(float h, bool bold) { return juce::Font(juce::FontOptions(bold ? barlowBold : barlow).withHeight(h)); }

LnF::LnF() {
    setColour(juce::ComboBox::backgroundColourId, PANEL); setColour(juce::ComboBox::textColourId, BONE);
    setColour(juce::ComboBox::outlineColourId, LINE); setColour(juce::ComboBox::arrowColourId, BLOOD);
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff15110f)); setColour(juce::PopupMenu::textColourId, BONE);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, BLOOD.withAlpha(0.6f)); setColour(juce::PopupMenu::headerTextColourId, BLOOD);
    setColour(juce::TextButton::textColourOffId, BONE); setColour(juce::TextButton::textColourOnId, BG);
    setColour(juce::Slider::textBoxTextColourId, BONE); setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::ToggleButton::textColourId, BONE); setColour(juce::ToggleButton::tickColourId, BLOOD);
    setColour(juce::Label::textColourId, BONE);
}
void LnF::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s) {
    auto r = juce::Rectangle<float>((float)x, (float)y, (float)w, (float)h).reduced(4);
    float d = juce::jmin(r.getWidth(), r.getHeight()); auto c = r.withSizeKeepingCentre(d, d);
    float ang = a0 + pos * (a1 - a0);
    juce::Path track; track.addCentredArc(c.getCentreX(), c.getCentreY(), d * 0.46f, d * 0.46f, 0, a0, a1, true);
    g.setColour(LINE); g.strokePath(track, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    bool bip = s.getMinimum() < 0;
    juce::Path val; val.addCentredArc(c.getCentreX(), c.getCentreY(), d * 0.46f, d * 0.46f, 0, bip ? (a0 + a1) * 0.5f : a0, ang, true);
    g.setColour(s.isMouseOverOrDragging() ? EMBER : BLOOD); g.strokePath(val, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(juce::Colour(0xff1d1916)); g.fillEllipse(c.reduced(d * 0.16f));
    g.setColour(BONE.withAlpha(0.25f)); g.drawEllipse(c.reduced(d * 0.16f), 1.0f);
    juce::Point<float> p1 = c.getCentre().getPointOnCircumference(d * 0.12f, ang), p2 = c.getCentre().getPointOnCircumference(d * 0.32f, ang);
    g.setColour(BONE); g.drawLine({p1, p2}, 2.2f);
}
void LnF::drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) {
    auto r = b.getLocalBounds().toFloat().reduced(1.0f);
    bool on = b.getToggleState();
    juce::Colour fill = on ? BLOOD : (down ? BLOOD.withAlpha(0.5f) : juce::Colour(0xff1d1916));
    if (b.getProperties()["big"]) fill = down ? EMBER : (over ? BLOOD.brighter(0.15f) : BLOOD);
    g.setColour(fill); g.fillRoundedRectangle(r, 4.0f);
    g.setColour(over ? EMBER : LINE); g.drawRoundedRectangle(r, 4.0f, 1.0f);
}
void LnF::drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool) {
    g.setFont(b.getProperties()["big"] ? Fonts::get().title(22) : Fonts::get().text(15, true));
    g.setColour(b.getToggleState() || b.getProperties()["big"] ? juce::Colour(0xfff3ebd8) : BONE);
    g.drawFittedText(b.getButtonText(), b.getLocalBounds().reduced(3), juce::Justification::centred, 2);
}
void LnF::drawComboBox(juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&) {
    auto r = juce::Rectangle<float>(0, 0, (float)w, (float)h).reduced(0.5f);
    g.setColour(PANEL); g.fillRoundedRectangle(r, 3); g.setColour(LINE); g.drawRoundedRectangle(r, 3, 1);
    juce::Path arrow; float ax = (float)w - 14, ay = h * 0.5f; arrow.addTriangle(ax - 4, ay - 2, ax + 4, ay - 2, ax, ay + 3);
    g.setColour(BLOOD); g.fillPath(arrow);
}
juce::Font LnF::getComboBoxFont(juce::ComboBox&) { return Fonts::get().text(16, true); }
juce::Font LnF::getPopupMenuFont() { return Fonts::get().text(17); }
juce::Font LnF::getTextButtonFont(juce::TextButton&, int) { return Fonts::get().text(15, true); }
juce::Label* LnF::createSliderTextBox(juce::Slider& s) {
    auto* l = LookAndFeel_V4::createSliderTextBox(s); l->setFont(Fonts::get().text(14)); l->setColour(juce::Label::textColourId, DIM);
    l->setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack); l->setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack); return l;
}

// ================================================================ the eye: calm in ambient, furious when it burns
class EyeView : public juce::Component {
public:
    explicit EyeView(DfdProcessor& p) : proc(p) {}
    void paint(juce::Graphics& g) override {
        auto b = getLocalBounds().toFloat(); auto c = b.getCentre();
        float ins = proc.engine.intensity.load(), lv = juce::jmin(1.0f, proc.engine.level.load() * 1.5f);
        double t = juce::Time::getMillisecondCounterHiRes() / 1000.0;
        float r = juce::jmin(b.getWidth(), b.getHeight()) * 0.42f;
        float jx = 0, jy = 0;
        if (ins > 0.72f) { juce::Random rnd((juce::int64)(t * 30)); float a = (ins - 0.72f) * r * 0.06f; jx = rnd.nextFloat() * a * 2 - a; jy = rnd.nextFloat() * a * 2 - a; }
        c += {jx, jy};
        // glow
        juce::ColourGradient glow(BLOOD.withAlpha(0.18f + ins * 0.3f), c.x, c.y, BLOOD.withAlpha(0.0f), c.x + r * 1.5f, c.y, true);
        g.setGradientFill(glow); g.fillEllipse(juce::Rectangle<float>(r * 3, r * 3).withCentre(c));
        // rays (spin faster with intensity)
        float spin = (float)std::fmod(t * (0.1 + ins * 1.4), juce::MathConstants<double>::twoPi);
        g.setColour(BONE.withAlpha(0.85f));
        for (int k = 0; k < 18; ++k) {
            float a = spin + k * juce::MathConstants<float>::twoPi / 18.0f, len = r * (0.18f + 0.1f * (k % 2)) * (1.0f + lv * 0.4f);
            auto p1 = c.getPointOnCircumference(r * 1.12f, a), p2 = c.getPointOnCircumference(r * 1.12f + len, a);
            g.setColour(ins > 0.8f && k % 3 == 0 ? BLOOD : BONE.withAlpha(0.8f)); g.drawLine({p1, p2}, k % 2 ? 1.5f : 2.5f);
        }
        juce::Path ring; ring.addEllipse(juce::Rectangle<float>(r * 2.1f, r * 2.1f).withCentre(c));
        float dash[] = {10.0f, 7.0f}; juce::Path dashed; juce::PathStrokeType(2.5f).createDashedStroke(dashed, ring, dash, 2);
        g.setColour(ins > 0.78f ? BLOOD : BONE); g.fillPath(dashed);
        // the triangle
        juce::Path tri; tri.addTriangle(c.x, c.y - r * 0.95f, c.x + r * 0.83f, c.y + r * 0.5f, c.x - r * 0.83f, c.y + r * 0.5f);
        g.setColour(BONE); g.strokePath(tri, juce::PathStrokeType(2.5f, juce::PathStrokeType::mitered));
        // lids: calm = heavy, furious = narrowed; blinks
        float open = 1.0f;
        double blinkEvery = ins < 0.27f ? 7.9 : ins > 0.7f ? 2.6 : 5.3;
        if (std::fmod(t, blinkEvery) < 0.14) open = 0.08f;
        if (ins < 0.27f) open *= 0.55f + ins; else if (ins > 0.66f) open *= 1.0f - (ins - 0.66f) * 0.9f;
        float W = r * 0.62f, Hh = r * 0.36f * open;
        juce::Path eyeShape; eyeShape.startNewSubPath(c.x - W, c.y); eyeShape.quadraticTo(c.x, c.y - Hh * 2, c.x + W, c.y); eyeShape.quadraticTo(c.x, c.y + Hh * 2, c.x - W, c.y); eyeShape.closeSubPath();
        g.setColour(BG); g.fillPath(eyeShape);
        {
            juce::Graphics::ScopedSaveState ss(g); g.reduceClipRegion(eyeShape);
            float gx = (float)std::sin(t / (ins < 0.27f ? 4.0 : ins > 0.66f ? 0.7 : 1.6)) * r * 0.17f;
            float gy = (float)std::sin(t / 2.3 + 1.0) * r * 0.06f;
            auto ic = c + juce::Point<float>(gx, gy); float ir = r * 0.24f;
            g.setColour(ins > 0.78f ? BLOOD : juce::Colour(0xff3a2a22)); g.fillEllipse(juce::Rectangle<float>(ir * 2, ir * 2).withCentre(ic));
            g.setColour(BONE); g.drawEllipse(juce::Rectangle<float>(ir * 2, ir * 2).withCentre(ic), 2.0f);
            float pr = r * (0.1f + lv * 0.06f) * (ins > 0.66f ? 0.65f : ins < 0.27f ? 1.25f : 1.0f);
            g.fillEllipse(juce::Rectangle<float>(pr * 2, pr * 2).withCentre(ic));
            g.setColour(BG); g.fillEllipse(juce::Rectangle<float>(pr * 0.7f, pr * 0.7f).withCentre(ic));
            g.setColour(juce::Colours::white); g.fillEllipse(juce::Rectangle<float>(pr * 0.35f, pr * 0.35f).withCentre(ic + juce::Point<float>(-pr * 0.6f, -pr * 0.6f)));
            if (ins > 0.47f) { g.setColour(BLOOD); int v = 1 + (int)((ins - 0.47f) * 8); for (int k = 0; k < v; ++k) { float sx = k % 2 ? 1.0f : -1.0f, yy = (k - v / 2) * Hh / (v + 1);
                g.drawLine(c.x + sx * W * 0.97f, c.y + yy * 0.3f, c.x + sx * W * 0.62f, c.y + yy, 1.2f); } }
        }
        g.setColour(BONE); g.strokePath(eyeShape, juce::PathStrokeType(2.6f));
        if (ins > 0.55f) { float lift = Hh + r * 0.12f, drop = (ins - 0.55f) * r * 0.35f; g.setColour(ins > 0.78f ? BLOOD : BONE);
            g.drawLine(c.x - W, c.y - lift - drop, c.x - W * 0.25f, c.y - lift + drop, 2.6f); g.drawLine(c.x + W, c.y - lift - drop, c.x + W * 0.25f, c.y - lift + drop, 2.6f); }
        // the tear
        juce::Path tear; tear.startNewSubPath(c.x, c.y + r * 0.6f); tear.quadraticTo(c.x - r * 0.06f, c.y + r * 0.72f, c.x, c.y + r * 0.78f); tear.quadraticTo(c.x + r * 0.06f, c.y + r * 0.72f, c.x, c.y + r * 0.6f);
        g.setColour(BONE); g.fillPath(tear);
    }
private:
    DfdProcessor& proc;
};

// ================================================================ the four lanes: click to write, the playhead runs
class GridView : public juce::Component {
public:
    explicit GridView(DfdProcessor& p) : proc(p) {}
    int drumLane = KICK;
    void paint(juce::Graphics& g) override {
        auto& pat = proc.engine.pat; int play = proc.engine.uiStep.load();
        g.setColour(PANEL); g.fillRoundedRectangle(getLocalBounds().toFloat(), 4);
        for (int t = 0; t < NTRACKS; ++t) {
            auto row = rowRect(t); const auto& tr = pat.t[t]; int len = juce::jmax(1, tr.len);
            g.setColour(proc.engine.mute[t] ? DIM.withAlpha(0.6f) : BLOOD); g.setFont(Fonts::get().text(15, true));
            g.drawText(kTrackNames[t], juce::Rectangle<int>(4, (int)row.getY(), 54, (int)row.getHeight()), juce::Justification::centredLeft);
            float cw = (row.getWidth()) / 32.0f;
            for (int i = 0; i < 32; ++i) {
                auto cell = juce::Rectangle<float>(row.getX() + i * cw, row.getY(), cw, row.getHeight()).reduced(1.2f);
                bool inLen = i < len; int si = i % len; const auto& s = tr.s[si];
                g.setColour(i % 4 == 0 ? juce::Colour(0xff221d19) : juce::Colour(0xff1a1613)); g.fillRect(cell);
                if (!inLen) { g.setColour(BG.withAlpha(0.6f)); g.fillRect(cell); }
                if (t == TR_DRUMS) {
                    float lh = cell.getHeight() / NLANES;
                    for (int L = 0; L < NLANES; ++L) if (s.hits >> L & 1) {
                        g.setColour((s.accHits >> L & 1) ? EMBER : (L == drumLane ? BONE : BONE.withAlpha(0.55f)));
                        g.fillRect(cell.getX() + 1, cell.getY() + L * lh + 0.5f, cell.getWidth() - 2, juce::jmax(1.0f, lh - 1.0f));
                    }
                    if (s.ratchet > 1) { g.setColour(BLOOD); g.fillRect(cell.removeFromBottom(2.0f)); }
                } else if (s.gate == 1) {
                    int lo = 127, hi = 0; for (int k = 0; k < len; ++k) if (tr.s[k].gate == 1) { lo = juce::jmin(lo, (int)tr.s[k].notes[0]); hi = juce::jmax(hi, (int)tr.s[k].notes[0]); }
                    float y = hi > lo ? 1.0f - (s.notes[0] - lo) / (float)(hi - lo) : 0.5f;
                    auto dot = juce::Rectangle<float>(cell.getX() + 1, cell.getY() + y * (cell.getHeight() - 6), cell.getWidth() - 2, 6);
                    g.setColour(s.acc ? EMBER : BONE); g.fillRoundedRectangle(dot, 2);
                    if (s.n > 1) { g.setColour(BONE.withAlpha(0.4f)); g.fillRect(cell.getX() + 1, cell.getY(), cell.getWidth() - 2, cell.getHeight()); }
                    if (s.slide) { g.setColour(BLOOD); g.drawLine(dot.getRight(), dot.getCentreY(), dot.getRight() + cw * 0.5f, dot.getCentreY(), 2); }
                } else if (s.gate == 2) { g.setColour(BONE.withAlpha(0.35f)); g.fillRect(cell.withHeight(3.0f).withY(cell.getCentreY())); }
                if (inLen && (play % len) == i && proc.engine.uiStep.load() >= 0) { g.setColour(BLOOD.withAlpha(0.35f)); g.fillRect(cell); }
            }
        }
        if (proc.engine.nextReady) { g.setColour(EMBER); g.setFont(Fonts::get().text(14, true));
            g.drawText("LEAD: TRYING NEXT " + juce::String(proc.engine.nextPick + 1) + "  (KEEP to keep it)", getLocalBounds().removeFromBottom(16).reduced(60, 0), juce::Justification::centredRight); }
    }
    void mouseDown(const juce::MouseEvent& e) override {
        for (int t = 0; t < NTRACKS; ++t) {
            auto row = rowRect(t); if (!row.contains(e.position)) continue;
            int i = (int)((e.position.x - row.getX()) / (row.getWidth() / 32.0f));
            if (t == TR_DRUMS) { int L = juce::jlimit(0, NLANES - 1, (int)((e.position.y - row.getY()) / (row.getHeight() / NLANES))); drumLane = L; proc.post(C_SET_STEP, t, i, L); }
            else { float y = 1.0f - (e.position.y - row.getY()) / row.getHeight(); proc.post(C_SET_STEP, t, i, (int)(y * 7)); }
            repaint(); return;
        }
    }
private:
    juce::Rectangle<float> rowRect(int t) const {
        auto b = getLocalBounds().toFloat().reduced(6).withTrimmedLeft(56).withTrimmedBottom(16);
        float hs[NTRACKS] = {0.34f, 0.22f, 0.22f, 0.22f}; float y = b.getY();
        for (int k = 0; k < t; ++k) y += b.getHeight() * hs[k];
        return {b.getX(), y + 2, b.getWidth(), b.getHeight() * hs[t] - 4};
    }
    DfdProcessor& proc;
};

// ================================================================ the 8 performance pads (KeyLab pads 1-8 do the same)
class PadView : public juce::Component {
public:
    explicit PadView(DfdProcessor& p) : proc(p) {}
    void paint(juce::Graphics& g) override {
        for (int i = 0; i < 8; ++i) {
            auto r = padRect(i);
            bool lit = i < 4 ? proc.engine.mute[i] : proc.engine.isPadDown(i) || (i == 7 && proc.engine.nextReady);
            g.setColour(lit ? BLOOD : juce::Colour(0xff1d1916)); g.fillRoundedRectangle(r, 6);
            g.setColour(lit ? EMBER : LINE); g.drawRoundedRectangle(r, 6, 1.5f);
            g.setColour(lit ? juce::Colour(0xfff3ebd8) : BONE); g.setFont(Fonts::get().text(15, true));
            g.drawFittedText(Engine::kPadNames[i], r.toNearestInt().reduced(4), juce::Justification::centred, 2);
            g.setColour(DIM); g.setFont(Fonts::get().text(11)); g.drawText("PAD " + juce::String(i + 1), r.toNearestInt().reduced(5, 3), juce::Justification::topLeft);
        }
    }
    void mouseDown(const juce::MouseEvent& e) override { held = hit(e.position); if (held >= 0) proc.post(C_PAD, held, 1); repaint(); }
    void mouseUp(const juce::MouseEvent&) override { if (held >= 0) proc.post(C_PAD, held, 0); held = -1; repaint(); }
private:
    int hit(juce::Point<float> p) const { for (int i = 0; i < 8; ++i) if (padRect(i).contains(p)) return i; return -1; }
    juce::Rectangle<float> padRect(int i) const { float w = getWidth() / 4.0f, h = getHeight() / 2.0f; return {(i % 4) * w + 3, (i / 4) * h + 3, w - 6, h - 6}; }
    DfdProcessor& proc; int held = -1;
};

// ================================================================ drag the pattern into Ableton as MIDI
class DragMidiButton : public juce::Component {
public:
    explicit DragMidiButton(DfdProcessor& p) : proc(p) { setMouseCursor(juce::MouseCursor::DraggingHandCursor); }
    void paint(juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat().reduced(1); g.setColour(juce::Colour(0xff1d1916)); g.fillRoundedRectangle(r, 4);
        g.setColour(EMBER); g.drawRoundedRectangle(r, 4, 1.2f); g.setFont(Fonts::get().text(15, true)); g.setColour(BONE);
        g.drawText(juce::String::fromUTF8("\xe2\x87\xa2 DRAG MIDI"), getLocalBounds(), juce::Justification::centred);
    }
    void mouseDrag(const juce::MouseEvent& e) override {
        if (dragging || e.getDistanceFromDragStart() < 6) return;
        dragging = true; auto f = proc.writeMidiFile();
        juce::DragAndDropContainer::performExternalDragDropOfFiles({f.getFullPathName()}, false, this, [this] { dragging = false; });
    }
    void mouseUp(const juce::MouseEvent&) override { dragging = false; }
private:
    DfdProcessor& proc; bool dragging = false;
};

// ================================================================ the editor
void DfdEditor::Content::paint(juce::Graphics& g) {
    g.fillAll(BG);
    juce::ColourGradient top(juce::Colour(0xff2a1d17), 600, -60, BG.withAlpha(0.0f), 600, 420, true); g.setGradientFill(top); g.fillRect(getLocalBounds());
    g.setColour(LINE); g.drawRect(getLocalBounds().reduced(6), 1); g.setColour(BLOOD.withAlpha(0.25f)); g.drawRect(getLocalBounds().reduced(10), 1);
    g.setColour(BONE); g.setFont(Fonts::get().title(26)); g.drawText("DARK FUNK DURCAL SYNT", 24, 16, 420, 34, juce::Justification::centredLeft);
    g.setColour(BLOOD); g.setFont(Fonts::get().text(13, true)); g.drawText(juce::String::fromUTF8("ACID  \xc2\xb7  BREAKS  \xc2\xb7  RITUALS  \xc2\xb7  WARPED FUNK"), 26, 46, 420, 16, juce::Justification::centredLeft);
    auto panel = [&](juce::Rectangle<int> r, const char* title) {
        g.setColour(PANEL); g.fillRoundedRectangle(r.toFloat(), 5); g.setColour(LINE); g.drawRoundedRectangle(r.toFloat(), 5, 1);
        g.setColour(BLOOD); g.fillRect(r.getX() + 1, r.getY(), r.getWidth() - 2, 2);
        g.setColour(BONE); g.setFont(Fonts::get().title(15)); g.drawText(title, r.getX() + 12, r.getY() + 8, r.getWidth() - 20, 18, juce::Justification::centredLeft);
    };
    panel({20, 76, 270, 420}, "RITUAL");
    panel({910, 76, 270, 420}, "LIVE");
    g.setColour(DIM); g.setFont(Fonts::get().text(13, true));
    g.drawText("FUSION A/B", 40, 174, 100, 16, juce::Justification::centred); g.drawText("CHAOS", 170, 174, 100, 16, juce::Justification::centred);
    g.drawText("KEYS PLAY", 922, 322, 120, 14, juce::Justification::centredLeft);
    g.setColour(DIM); g.setFont(Fonts::get().text(13));
    g.drawFittedText(juce::String::fromUTF8("KEYLAB ESSENTIAL: pads 1\xe2\x80\x938 = these pads \xc2\xb7 knobs = the SOUND page shown below "
                     "(Part \xe2\x97\x80 \xe2\x96\xb6 changes page) \xc2\xb7 faders = levels, filter, echo, reverb, drive \xc2\xb7 master fader = master. "
                     "Shift-click a scene to store it."), 922, 404, 250, 84, juce::Justification::topLeft, 6);
    panel({20, 508, 1160, 200}, "SOUND");
    g.setColour(DIM); g.setFont(Fonts::get().text(13));
    g.drawText(juce::String::fromUTF8("CREATED AT POLARIS SOUND LAB \xc2\xb7 M\xc3\x89RIDA, YUCAT\xc3\x81N, M\xc3\x89XICO \xc2\xb7 @polaris.sound.lab"), 20, 722, 760, 18, juce::Justification::centredLeft);
    if (ed) { g.setColour(EMBER); g.setFont(Fonts::get().text(16, true)); g.drawText(ed->status, 780, 722, 400, 18, juce::Justification::centredRight); }
}

DfdEditor::DfdEditor(DfdProcessor& p) : AudioProcessorEditor(&p), proc(p) {
    setLookAndFeel(&lnf); content.ed = this; addAndMakeVisible(content); content.setSize(W, H);
    auto& A = proc.apvts;
    auto add = [&](juce::Component& c) { content.addAndMakeVisible(c); };
    // header
    buildPresetMenu(); add(presetBox); add(prevPreset); add(nextPreset); add(undoBtn); add(redoBtn); add(playBtn);
    presetBox.onChange = [this] { int id = presetBox.getSelectedId(); if (id > 0) proc.setCurrentProgram(id - 1); };
    prevPreset.onClick = [this] { proc.setCurrentProgram((proc.currentPreset + NPRESETS - 1) % NPRESETS); presetBox.setSelectedId(proc.currentPreset + 1, juce::dontSendNotification); };
    nextPreset.onClick = [this] { proc.setCurrentProgram((proc.currentPreset + 1) % NPRESETS); presetBox.setSelectedId(proc.currentPreset + 1, juce::dontSendNotification); };
    undoBtn.onClick = [this] { proc.post(C_UNDO); }; redoBtn.onClick = [this] { proc.post(C_REDO); };
    playBtn.setClickingTogglesState(true); playBtn.onClick = [this] { proc.post(C_PLAY_TOGGLE); };
    playBtn.setTooltip("Plays without the DAW's transport (the DAW's PLAY always drives it too)");
    dragMidi = std::make_unique<DragMidiButton>(proc); add(*dragMidi);
    // ritual
    for (auto* c : {&styleA, &styleB}) { for (int i = 0; i < NSTYLES; ++i) c->addItem(kStyleNames[i], i + 1); add(*c); }
    for (int i = 0; i < NMOODS; ++i) moodBox.addItem(kMoodNames[i], i + 1); add(moodBox);
    for (int i = 0; i < 12; ++i) keyBox.addItem(kKeyNames[i], i + 1); add(keyBox);
    aStyleA = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(A, "styleA", styleA);
    aStyleB = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(A, "styleB", styleB);
    aMood = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(A, "mood", moodBox);
    aKey = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(A, "key", keyBox);
    auto quiet = [](juce::Slider& s) { s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack); s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack); s.setColour(juce::Slider::textBoxTextColourId, DIM); };
    quiet(fusion); quiet(chaos);
    for (auto* s : {&fusion, &chaos}) { s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 16); add(*s); }
    aFusion = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(A, "fusion", fusion);
    aChaos = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(A, "chaos", chaos);
    summon.getProperties().set("big", true); add(summon); summon.onClick = [this] { proc.post(C_SUMMON); };
    add(diceDrums); add(diceAcid); add(diceMel); add(diceSound); add(diceAll);
    diceDrums.onClick = [this] { proc.post(C_DICE_DRUMS); }; diceAcid.onClick = [this] { proc.post(C_DICE_ACID); };
    diceMel.onClick = [this] { proc.post(C_DICE_MELODY); }; diceSound.onClick = [this] { proc.post(C_DICE_SOUND); }; diceAll.onClick = [this] { proc.post(C_DICE_ALL); };
    static const char* shapes[Gen::NSHAPES] = {"WAVE", "RISE", "FALL", "QUESTION & ANSWER", "ARPEGGIO", "HOOK", "FEW NOTES", "BUSY"};
    for (int i = 0; i < Gen::NSHAPES; ++i) shapeBox.addItem(shapes[i], i + 1); shapeBox.setSelectedId(6); add(shapeBox);
    add(makeBtn); makeBtn.onClick = [this] { proc.post(C_MAKE_MELODY, shapeBox.getSelectedId() - 1); };
    // centre
    eye = std::make_unique<EyeView>(proc); add(*eye);
    grid = std::make_unique<GridView>(proc); add(*grid);
    // live
    pads = std::make_unique<PadView>(proc); add(*pads);
    for (int i = 0; i < 8; ++i) {
        scene[i].setButtonText(juce::String::charToString((juce::juce_wchar)('A' + i))); add(scene[i]);
        scene[i].setTooltip("Click: play this scene from the next bar. Shift-click: store the current pattern here.");
        scene[i].onClick = [this, i] { if (juce::ModifierKeys::currentModifiers.isShiftDown()) proc.post(C_SCENE_STORE, i); else proc.post(C_SCENE_QUEUE, i); };
    }
    add(nextGen); add(next1); add(next2); add(next3); add(keep);
    nextGen.onClick = [this] { proc.post(C_NEXT_GEN); };
    next1.onClick = [this] { proc.post(C_NEXT_PICK, 0); }; next2.onClick = [this] { proc.post(C_NEXT_PICK, 1); }; next3.onClick = [this] { proc.post(C_NEXT_PICK, 2); };
    keep.onClick = [this] { proc.post(C_NEXT_KEEP); };
    for (int i = 0; i < 4; ++i) targetBox.addItem(kTargetNames[i], i + 1); add(targetBox);
    aTarget = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(A, "playTarget", targetBox);
    add(scaleLock); aLock = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(A, "scaleLock", scaleLock);
    add(keyTrans); keyTrans.onClick = [this] { proc.engine.p.keyTranspose = keyTrans.getToggleState(); };
    // banks
    for (int b = 0; b < NBANKS; ++b) { bankBtn[b].setButtonText(juce::String(b + 1) + " " + kBankNames[b]); bankBtn[b].setRadioGroupId(77); bankBtn[b].setClickingTogglesState(true);
        add(bankBtn[b]); bankBtn[b].onClick = [this, b] { showBank(b); }; }
    for (int k = 0; k < 8; ++k) {
        knob[k].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); knob[k].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 110, 16);
        knob[k].setDoubleClickReturnValue(true, 0); quiet(knob[k]); add(knob[k]);
        knobLabel[k].setJustificationType(juce::Justification::centred); knobLabel[k].setFont(Fonts::get().text(16, true)); add(knobLabel[k]);
    }
    bankBtn[0].setToggleState(true, juce::dontSendNotification); showBank(0);
    setResizable(true, true); setResizeLimits(W * 6 / 10, H * 6 / 10, W * 2, H * 2);
    if (auto* c = getConstrainer()) c->setFixedAspectRatio((double)W / H);
    setSize(W, H);
    startTimerHz(30);
}
DfdEditor::~DfdEditor() { setLookAndFeel(nullptr); }

void DfdEditor::buildPresetMenu() {
    presetBox.clear(); juce::String cat;
    for (int i = 0; i < NPRESETS; ++i) {
        if (cat != kPresets[i].category) { cat = kPresets[i].category; presetBox.addSectionHeading(cat); }
        presetBox.addItem(kPresets[i].name, i + 1);
    }
    presetBox.setSelectedId(proc.currentPreset + 1, juce::dontSendNotification);
}

void DfdEditor::showBank(int b) {
    bank = b;
    for (int k = 0; k < 8; ++k) {
        aKnob[k].reset();
        int i = b * 8 + k; const auto& d = kParams[i];
        aKnob[k] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc.apvts, d.id, knob[k]);
        knobLabel[k].setText(juce::String(d.name).toUpperCase(), juce::dontSendNotification);
        knob[k].setDoubleClickReturnValue(true, d.def);
    }
}

void DfdEditor::paint(juce::Graphics& g) { g.fillAll(BG); }

void DfdEditor::resized() {
    float s = juce::jmin(getWidth() / (float)W, getHeight() / (float)H);
    content.setTransform(juce::AffineTransform::scale(s));
    // header
    presetBox.setBounds(480, 22, 300, 30); prevPreset.setBounds(448, 22, 30, 30); nextPreset.setBounds(782, 22, 30, 30);
    undoBtn.setBounds(826, 22, 64, 30); redoBtn.setBounds(894, 22, 64, 30); playBtn.setBounds(966, 22, 76, 30); dragMidi->setBounds(1050, 22, 130, 30);
    // ritual
    styleA.setBounds(34, 108, 242, 28); styleB.setBounds(34, 142, 242, 28);
    fusion.setBounds(40, 190, 100, 88); chaos.setBounds(170, 190, 100, 88);
    moodBox.setBounds(34, 282, 150, 28); keyBox.setBounds(190, 282, 86, 28);
    summon.setBounds(34, 318, 242, 48);
    diceDrums.setBounds(34, 374, 78, 30); diceAcid.setBounds(116, 374, 78, 30); diceMel.setBounds(198, 374, 78, 30);
    diceSound.setBounds(34, 408, 118, 30); diceAll.setBounds(158, 408, 118, 30);
    shapeBox.setBounds(34, 448, 130, 30); makeBtn.setBounds(168, 448, 108, 30);
    // centre
    eye->setBounds(300, 66, 600, 200);
    grid->setBounds(300, 270, 600, 226);
    // live
    pads->setBounds(918, 104, 254, 140);
    for (int i = 0; i < 8; ++i) scene[i].setBounds(922 + i * 31, 252, 29, 28);
    nextGen.setBounds(922, 290, 80, 30); next1.setBounds(1006, 290, 36, 30); next2.setBounds(1046, 290, 36, 30); next3.setBounds(1086, 290, 36, 30); keep.setBounds(1124, 290, 48, 30);
    targetBox.setBounds(922, 338, 250, 28);
    scaleLock.setBounds(922, 370, 120, 26); keyTrans.setBounds(1046, 370, 130, 26);
    // banks
    for (int b = 0; b < NBANKS; ++b) bankBtn[b].setBounds(110 + b * 132, 514, 128, 26);
    for (int k = 0; k < 8; ++k) { knob[k].setBounds(36 + k * 142, 566, 120, 112); knobLabel[k].setBounds(26 + k * 142, 544, 140, 20); }
}

void DfdEditor::timerCallback() {
    eye->repaint(); grid->repaint(); pads->repaint();
    for (int i = 0; i < 8; ++i) {
        bool cur = proc.engine.currentScene.load() == i, q = proc.engine.queuedScene.load() == i;
        scene[i].setToggleState(cur, juce::dontSendNotification);
        scene[i].setColour(juce::TextButton::buttonColourId, q ? EMBER : BG);
        scene[i].setAlpha(proc.engine.sceneUsed[i] || cur ? 1.0f : 0.55f);
    }
    for (int k = 0; k < 3; ++k) (k == 0 ? next1 : k == 1 ? next2 : next3).setToggleState(proc.engine.nextReady && proc.engine.nextPick == k, juce::dontSendNotification);
    juce::String st = juce::String(proc.engine.lastAction.load()) + juce::String::fromUTF8("  \xc2\xb7  ") + juce::String((int)proc.engine.curBpm()) + " BPM";
    if (st != status) { status = st; content.repaint(700, 716, 500, 30); }
    if (presetBox.getSelectedId() != proc.currentPreset + 1) presetBox.setSelectedId(proc.currentPreset + 1, juce::dontSendNotification);
}
