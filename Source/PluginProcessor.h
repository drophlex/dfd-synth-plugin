// Dark Funk Durcal Synt — the plugin (VST3 / AU / standalone). (c) Polaris Sound Lab, Mérida, Yucatán, México.
#pragma once
#include <JuceHeader.h>
#include "ParamTable.h"
#include "Presets.h"

class DfdProcessor : public juce::AudioProcessor, private juce::Timer {
public:
    DfdProcessor();
    ~DfdProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Dark Funk Durcal Synt"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return dfd::NPRESETS; }
    int getCurrentProgram() override { return currentPreset; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // for the editor
    juce::AudioProcessorValueTreeState apvts;
    dfd::Engine engine;
    void post(int type, int a = 0, int b = 0, int c = 0) { engine.post({type, a, b, c}); }
    juce::File writeMidiFile();          // the current pattern as a .mid (drag it into Ableton)
    void setParam(const char* id, float plainValue);
    float getParam(const char* id) const;
    int currentPreset = 0;
    juce::String presetName() const { return juce::String(dfd::kPresets[currentPreset].name); }

private:
    void timerCallback() override;       // message thread: reflects what the engine summoned into the parameters
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    std::array<std::atomic<float>*, dfd::NPARAMS> raw{};
    float modWheel = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DfdProcessor)
};
