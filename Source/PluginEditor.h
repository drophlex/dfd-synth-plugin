#pragma once
#include "PluginProcessor.h"

namespace dfdui {
const juce::Colour BG{0xff0e0c0b}, PANEL{0xff171412}, LINE{0xff3a332c}, BONE{0xffebe2cd}, DIM{0xff9c917c}, BLOOD{0xffc2412f}, EMBER{0xffe8613f};
struct Fonts { juce::Typeface::Ptr cinzel, barlow, barlowBold; static Fonts& get(); juce::Font title(float h); juce::Font text(float h, bool bold = false); };
class LnF : public juce::LookAndFeel_V4 {
public:
    LnF();
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool over, bool down) override;
    void drawComboBox(juce::Graphics&, int w, int h, bool down, int bx, int by, int bw, int bh, juce::ComboBox&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    juce::Font getTextButtonFont(juce::TextButton&, int h) override;
    juce::Label* createSliderTextBox(juce::Slider&) override;
};
}

class EyeView;
class GridView;
class PadView;
class DragMidiButton;
class HelpPanel;

class DfdEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit DfdEditor(DfdProcessor&);
    ~DfdEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void showBank(int b);
    void buildPresetMenu();

    DfdProcessor& proc;
    dfdui::LnF lnf;
    struct Content : juce::Component { DfdEditor* ed = nullptr; void paint(juce::Graphics&) override; } content;
    static constexpr int W = 1200, H = 760;

    // header
    juce::ComboBox presetBox; juce::TextButton prevPreset{"<"}, nextPreset{">"}, undoBtn{"UNDO"}, redoBtn{"REDO"}, playBtn{"PLAY"};
    std::unique_ptr<DragMidiButton> dragMidi;
    // ritual
    juce::ComboBox styleA, styleB, moodBox, keyBox;
    juce::Slider fusion, chaos;
    juce::TextButton summon{"SUMMON"}, diceDrums{"DRUMS"}, diceAcid{"ACID"}, diceMel{"MELODY"}, diceSound{"SOUND"}, diceAll{"ALL"};
    juce::ComboBox shapeBox; juce::TextButton makeBtn{"MAKE MELODY"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> aStyleA, aStyleB, aMood, aKey;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> aFusion, aChaos;
    // centre
    std::unique_ptr<EyeView> eye; std::unique_ptr<GridView> grid;
    // live
    std::unique_ptr<PadView> pads;
    juce::TextButton scene[8]; juce::TextButton nextGen{"3 NEXT"}, next1{"1"}, next2{"2"}, next3{"3"}, keep{"KEEP"};
    juce::ComboBox targetBox; juce::ToggleButton scaleLock{"SCALE LOCK"}, keyTrans{"KEYS TRANSPOSE"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> aTarget;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> aLock;
    // knob banks (same order as the KeyLab / Ableton banks)
    juce::TextButton bankBtn[dfd::NBANKS];
    juce::Slider knob[8]; juce::Label knobLabel[8];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> aKnob[8];
    int bank = 0;
    juce::String status;
    // help (?) and a small donation button in the corner
    juce::TextButton helpBtn{"?"}, donateBtn{juce::String::fromUTF8("\xe2\x99\xa5")};
    std::unique_ptr<HelpPanel> help;
    void showHelp(bool show);
    void applyLanguage();
    juce::TextButton langBtn{"ES"};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DfdEditor)
};
