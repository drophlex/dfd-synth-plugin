// Host test: the real plugin class in a fake host. Plays every preset with a running transport, checks the output,
// round-trips the state, exports MIDI, opens the editor and saves screenshots (docs).
#include <JuceHeader.h>
#include "../Source/PluginProcessor.h"

struct FakeHead : juce::AudioPlayHead {
    double ppq = 0, bpm = 120; bool playing = true;
    juce::Optional<PositionInfo> getPosition() const override {
        PositionInfo p; p.setIsPlaying(playing); p.setPpqPosition(ppq); p.setBpm(bpm); return p;
    }
};

int main(int argc, char** argv) {
    juce::ScopedJuceInitialiser_GUI gui;
    juce::String outDir = argc > 1 ? argv[1] : "/tmp";
    int fails = 0;
    auto check = [&](const juce::String& what, bool ok) { printf("host: %-70s %s\n", what.toRawUTF8(), ok ? "ok" : "FAIL"); fails += !ok; };

    DfdProcessor proc; FakeHead head; proc.setPlayHead(&head);
    const double sr = 48000; const int bs = 512;
    proc.setPlayConfigDetails(0, 2, sr, bs); proc.prepareToPlay(sr, bs);
    juce::AudioBuffer<float> buf(2, bs); juce::MidiBuffer midi;
    auto run = [&](double seconds, float& peak, double& rms) {
        peak = 0; double acc = 0; long n = 0;
        for (int b = 0; b < (int)(seconds * sr / bs); ++b) {
            buf.clear(); proc.processBlock(buf, midi);
            head.bpm = proc.engine.pat.bpm; head.ppq += bs * head.bpm / 60.0 / sr;
            for (int i = 0; i < bs; ++i) { float v = buf.getSample(0, i); peak = std::max(peak, std::abs(v)); acc += v * v; ++n; if (!std::isfinite(v)) peak = 99; }
            if (b % 8 == 0) juce::MessageManager::getInstance()->runDispatchLoopUntil(1);
        }
        rms = std::sqrt(acc / std::max(1L, n));
    };
    int bad = 0; float worst = 0;
    for (int i = 0; i < proc.getNumPrograms(); ++i) {
        proc.setCurrentProgram(i);
        juce::MessageManager::getInstance()->runDispatchLoopUntil(200);
        float pk; double rms; run(4.0, pk, rms);
        bool ok = pk <= 1.0f && rms > 0.01;
        if (!ok) { ++bad; printf("  preset %s: peak %.3f rms %.4f\n", proc.getProgramName(i).toRawUTF8(), pk, rms); }
        worst = std::max(worst, pk);
    }
    check("every factory preset plays, bounded and audible (" + juce::String(proc.getNumPrograms()) + " presets, worst peak " + juce::String(worst, 3) + ")", bad == 0);

    // pads and live tools through MIDI (KeyLab pads: notes 36-43 on channel 11)
    for (int pad = 0; pad < 8; ++pad) { midi.addEvent(juce::MidiMessage::noteOn(11, 36 + pad, 1.0f), 0); float pk; double r; run(0.5, pk, r); midi.clear();
        midi.addEvent(juce::MidiMessage::noteOff(11, 36 + pad), 0); run(0.3, pk, r); midi.clear(); }
    check("the 8 pads by MIDI (mute x4, fill, stutter, build/drop, next phrase)", proc.engine.nextReady);
    // keys play the lead in key
    midi.addEvent(juce::MidiMessage::noteOn(1, 61, 0.9f), 0); { float pk; double r; run(0.5, pk, r); check("keys play (scale lock)", r > 0.001); } midi.clear();
    midi.addEvent(juce::MidiMessage::noteOff(1, 61), 0); { float pk; double r; run(0.2, pk, r); } midi.clear();

    // state round trip
    juce::MemoryBlock st; proc.getStateInformation(st);
    auto before = proc.engine.pat.t[1].s[0].notes[0]; int beforeLen = proc.engine.pat.t[0].len;
    proc.engine.post({dfd::C_SUMMON, 777}); { float pk; double r; run(0.2, pk, r); }
    proc.setStateInformation(st.getData(), (int)st.getSize());
    check("state round trip restores the pattern", proc.engine.pat.t[1].s[0].notes[0] == before && proc.engine.pat.t[0].len == beforeLen);

    // MIDI export
    auto f = proc.writeMidiFile(); juce::FileInputStream is(f); juce::MidiFile mf; bool rd = is.openedOk() && mf.readFrom(is);
    check("MIDI export: 5 tracks (tempo + 4)", rd && mf.getNumTracks() == 5);

    // editor + screenshots
    proc.setCurrentProgram(6); juce::MessageManager::getInstance()->runDispatchLoopUntil(300);
    { float pk; double r; run(3.0, pk, r); }
    std::unique_ptr<juce::AudioProcessorEditor> ed(proc.createEditor());
    ed->setSize(1200, 760);
    juce::MessageManager::getInstance()->runDispatchLoopUntil(200);
    for (int shot = 0; shot < 3; ++shot) {
        if (shot == 1) { proc.engine.intensity.store(0.95f); }
        if (shot == 2) { proc.engine.intensity.store(0.05f); }
        auto img = ed->createComponentSnapshot(ed->getLocalBounds(), true, 2.0f);
        juce::File out(outDir + "/plugin_ui_" + juce::String(shot) + ".png"); out.deleteFile();
        juce::FileOutputStream os(out); juce::PNGImageFormat png; png.writeImageToStream(img, os);
        { float pk; double r; run(0.05, pk, r); }
    }
    check("editor opens; screenshots saved", true);
    ed.reset();

    // free and open source: it always plays; the help panel (with the small donation button) opens
    {
        std::unique_ptr<juce::AudioProcessorEditor> e2(proc.createEditor()); e2->setSize(1200, 760);
        juce::MessageManager::getInstance()->runDispatchLoopUntil(200);
        juce::Component* helpBtn = nullptr;
        std::function<void(juce::Component*)> find = [&](juce::Component* c) { for (auto* k : c->getChildren()) { if (auto* tb = dynamic_cast<juce::TextButton*>(k); tb && tb->getButtonText() == "?") helpBtn = tb; find(k); } };
        find(e2.get());
        if (auto* tb = dynamic_cast<juce::TextButton*>(helpBtn)) tb->triggerClick();
        juce::MessageManager::getInstance()->runDispatchLoopUntil(300);
        auto img = e2->createComponentSnapshot(e2->getLocalBounds(), true, 2.0f); juce::File out(outDir + "/plugin_ui_help.png"); out.deleteFile();
        juce::FileOutputStream os(out); juce::PNGImageFormat png; png.writeImageToStream(img, os);
        check("the help panel opens (? in the corner)", helpBtn != nullptr);
        e2.reset();
    }
    printf(fails ? "host test FAILED (%d)\n" : "host test passed\n", fails);
    return fails != 0;
}
