#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace dfd;

juce::AudioProcessorValueTreeState::ParameterLayout DfdProcessor::layout() {
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    for (int i = 0; i < NPARAMS; ++i) {
        const auto& d = kParams[i];
        juce::ParameterID id{d.id, 1};
        if (d.kind == PK_INT && d.choices) {
            juce::StringArray c; for (int k = 0; k < d.nChoices; ++k) c.add(d.choices[k]);
            l.add(std::make_unique<juce::AudioParameterChoice>(id, d.name, c, (int)d.def));
        } else if (d.kind == PK_INT) {
            l.add(std::make_unique<juce::AudioParameterInt>(id, d.name, (int)d.lo, (int)d.hi, (int)d.def));
        } else if (d.kind == PK_BOOL) {
            l.add(std::make_unique<juce::AudioParameterBool>(id, d.name, d.def > 0.5f));
        } else {
            juce::String pid(d.id);
            auto fmt = [pid, lo = d.lo, hi = d.hi](float v, int) -> juce::String {
                if (pid == "filter") return std::abs(v) < 0.02f ? juce::String("OPEN") : (v < 0 ? "LOW-PASS " : "HIGH-PASS ") + juce::String(juce::roundToInt(std::abs(v) * 100)) + "%";
                if (pid == "acidTune" || pid == "bassTune") return (v >= 0 ? "+" : "") + juce::String(v * 12.0f, 1) + " st";
                if (pid == "drumPitch") return (v >= 0 ? "+" : "") + juce::String(v, 1) + " st";
                if (pid == "echoTime") return juce::String(v * 4.0f, 2).trimCharactersAtEnd("0").trimCharactersAtEnd(".") + "/16";
                return juce::String(juce::roundToInt((v - lo) / (hi - lo) * 100)) + "%";
            };
            l.add(std::make_unique<juce::AudioParameterFloat>(id, d.name, juce::NormalisableRange<float>(d.lo, d.hi), d.def,
                  juce::AudioParameterFloatAttributes().withStringFromValueFunction(fmt)));
        }
    }
    return l;
}

DfdProcessor::DfdProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "DFD", layout()) {
    for (int i = 0; i < NPARAMS; ++i) raw[(size_t)i] = apvts.getRawParameterValue(kParams[i].id);
    lic = dfdlic::load();
    canPlay = lic.canPlay();
    startTimerHz(30);
}

bool DfdProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo() || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono();
}

void DfdProcessor::prepareToPlay(double sampleRate, int) {
    for (int i = 0; i < NPARAMS; ++i) kParams[i].apply(engine.p, raw[(size_t)i]->load());
    engine.prepare((float)sampleRate);
}

bool DfdProcessor::tryActivate(const juce::String& email, const juce::String& code) {
    if (!dfdlic::activate(email, code)) return false;
    lic.licensed = true; lic.email = dfdlic::normaliseEmail(email);
    canPlay = true;
    return true;
}

void DfdProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    if (!canPlay.load()) { buffer.clear(); midi.clear(); return; }   // the trial is over: silent until the code is entered
    // parameters -> engine
    for (int i = 0; i < NPARAMS; ++i) kParams[i].apply(engine.p, raw[(size_t)i]->load());
    engine.p.acid.cutoff = juce::jmin(1.0f, engine.p.acid.cutoff + modWheel * 0.6f);
    engine.p.lead.bend = engine.bendNow;

    // MIDI: keys play / transpose, pads (channel 10 or 11, notes 36-43) perform, mod wheel and pitch bend
    for (const auto meta : midi) {
        const auto m = meta.getMessage();
        if (m.isNoteOn()) engine.noteOn(m.getChannel(), m.getNoteNumber(), m.getFloatVelocity());
        else if (m.isNoteOff()) engine.noteOff(m.getChannel(), m.getNoteNumber());
        else if (m.isPitchWheel()) engine.pitchBend((m.getPitchWheelValue() - 8192) / 8192.0f);
        else if (m.isController() && m.getControllerNumber() == 1) modWheel = m.getControllerValue() / 127.0f;
        else if (m.isAllNotesOff() || m.isAllSoundOff()) { for (int n = 0; n < 128; ++n) engine.noteOff(1, n); }
    }
    midi.clear();

    bool playing = false; double ppq = 0, bpm = 0;
    if (auto* ph = getPlayHead()) {
        if (auto pos = ph->getPosition()) {
            playing = pos->getIsPlaying();
            if (auto p = pos->getPpqPosition()) ppq = *p;
            if (auto b = pos->getBpm()) bpm = *b;
        }
    }
    const int n = buffer.getNumSamples();
    auto* L = buffer.getWritePointer(0);
    auto* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    if (R) engine.process(L, R, n, playing, ppq, bpm);
    else { std::vector<float> tmp((size_t)n); engine.process(L, tmp.data(), n, playing, ppq, bpm); for (int i = 0; i < n; ++i) L[i] = 0.5f * (L[i] + tmp[(size_t)i]); }
}

void DfdProcessor::setParam(const char* id, float plain) {
    if (auto* p = apvts.getParameter(id)) p->setValueNotifyingHost(p->convertTo0to1(plain));
}
float DfdProcessor::getParam(const char* id) const { if (auto* v = apvts.getRawParameterValue(id)) return v->load(); return 0; }

void DfdProcessor::timerCallback() {
    if (++licTick >= 30 * 60) { licTick = 0; canPlay = lic.canPlay(); }   // once a minute: the trial may end while open
    if (engine.summoned) {
        engine.summoned = false;
        if (const auto* st = engine.summonedStyle) {
            setParam("acidCut", st->cut); setParam("acidRes", st->res); setParam("acidEnv", st->env); setParam("acidDec", st->dec); setParam("acidDrive", st->drive);
            setParam("kit", (float)st->kit); setParam("bassMode", (float)st->bassMode); setParam("mood", (float)st->mood);
            if (engine.summonedLead) setParam("leadMode", (float)engine.summonedLead->leadMode);
            setParam("swing", st->swing);
        }
    }
    if (int s = engine.soundPick) {
        engine.soundPick = 0;
        if (s < 100) setParam("kit", (float)(s - 1));
        else if (s < 200) setParam("bassMode", (float)(s - 100));
        else if (s < 300) setParam("leadMode", (float)(s - 200));
        else { juce::Random r((juce::int64)s * 7919); setParam("acidCut", 0.15f + r.nextFloat() * 0.35f); setParam("acidRes", 0.55f + r.nextFloat() * 0.4f);
               setParam("acidEnv", 0.4f + r.nextFloat() * 0.5f); setParam("acidWave", r.nextBool() ? 0.0f : 1.0f); }
    }
}

void DfdProcessor::setCurrentProgram(int index) {
    if (index < 0 || index >= NPRESETS) return;
    currentPreset = index;
    const auto& pr = kPresets[index];
    for (int i = 0; i < NPARAMS; ++i) if (std::strcmp(kParams[i].id, "master") != 0 && std::strcmp(kParams[i].id, "key") != 0) setParam(kParams[i].id, kParams[i].def);
    setParam("styleA", (float)pr.styleA); setParam("styleB", (float)pr.styleB); setParam("fusion", pr.fusion); setParam("chaos", pr.chaos);
    // the engine reads the new styles on its next block; summon right after with the preset's own seed
    for (int i = 0; i < NPARAMS; ++i) kParams[i].apply(engine.p, raw[(size_t)i]->load());
    post(C_SUMMON, (int)pr.seed);
    // the preset's sound wins over the style's: applied after the summon has been reflected
    auto overrides = std::vector<std::pair<juce::String, float>>();
    for (auto& kv : pr.set) overrides.push_back({kv.first, kv.second});
    juce::Timer::callAfterDelay(120, [this, overrides] { for (auto& kv : overrides) setParam(kv.first.toRawUTF8(), kv.second); });
}
const juce::String DfdProcessor::getProgramName(int index) {
    return index >= 0 && index < NPRESETS ? juce::String(kPresets[index].category) + " / " + kPresets[index].name : juce::String();
}

// ---------------------------------------------------------------- state: parameters + pattern + scenes
void DfdProcessor::getStateInformation(juce::MemoryBlock& dest) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    juce::MemoryBlock pat(&engine.pat, sizeof(engine.pat)), sc(&engine.scenes, sizeof(engine.scenes));
    xml->setAttribute("pattern", pat.toBase64Encoding());
    xml->setAttribute("scenes", sc.toBase64Encoding());
    juce::String used; for (bool b : engine.sceneUsed) used << (b ? "1" : "0");
    xml->setAttribute("scenesUsed", used);
    xml->setAttribute("preset", currentPreset);
    xml->setAttribute("version", 2);
    copyXmlToBinary(*xml, dest);
}
void DfdProcessor::setStateInformation(const void* data, int size) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (!xml || !xml->hasTagName(apvts.state.getType())) return;
    apvts.replaceState(juce::ValueTree::fromXml(*xml));
    juce::MemoryBlock pat, sc;
    if (pat.fromBase64Encoding(xml->getStringAttribute("pattern")) && pat.getSize() == sizeof(engine.pat)) { std::memcpy(&engine.pat, pat.getData(), sizeof(engine.pat)); engine.seedCounter += 7; }
    if (sc.fromBase64Encoding(xml->getStringAttribute("scenes")) && sc.getSize() == sizeof(engine.scenes)) std::memcpy(&engine.scenes, sc.getData(), sizeof(engine.scenes));
    auto used = xml->getStringAttribute("scenesUsed");
    for (int i = 0; i < 8 && i < used.length(); ++i) engine.sceneUsed[i] = used[i] == '1';
    currentPreset = juce::jlimit(0, NPRESETS - 1, xml->getIntAttribute("preset", 0));
}

// ---------------------------------------------------------------- the pattern as a MIDI file (4 tracks; drums on channel 10, GM notes)
juce::File DfdProcessor::writeMidiFile() {
    static const int gm[NLANES] = {36, 38, 39, 42, 46, 45, 37, 56};
    juce::MidiFile mf; mf.setTicksPerQuarterNote(96);
    const int stepT = 24;
    juce::MidiMessageSequence tempo;
    tempo.addEvent(juce::MidiMessage::tempoMetaEvent((int)(60000000.0 / juce::jmax(40.0f, engine.pat.bpm))), 0);
    mf.addTrack(tempo);
    for (int t = 0; t < NTRACKS; ++t) {
        juce::MidiMessageSequence seq; const auto& tr = engine.pat.t[t]; int len = juce::jmax(1, tr.len);
        int ch = t == TR_DRUMS ? 10 : t + 1;
        seq.addEvent(juce::MidiMessage::textMetaEvent(3, kTrackNames[t]), 0);
        for (int i = 0; i < len; ++i) {
            const auto& s = tr.s[i]; if (s.gate != 1) continue;
            int on = i * stepT;
            if (t == TR_DRUMS) {
                for (int L = 0; L < NLANES; ++L) if (s.hits >> L & 1) {
                    auto v = (juce::uint8)((s.accHits >> L & 1) ? 120 : s.vel);
                    for (int r = 0; r < s.ratchet; ++r) { int at = on + r * stepT / s.ratchet;
                        seq.addEvent(juce::MidiMessage::noteOn(ch, gm[L], v), at); seq.addEvent(juce::MidiMessage::noteOff(ch, gm[L]), at + juce::jmax(2, stepT / s.ratchet / 2)); }
                }
            } else {
                int k = i + 1; while (k < len && tr.s[k].gate == 2) ++k;
                int off = k * stepT - (s.slide ? -2 : 4);
                for (int nn = 0; nn < juce::jmax(1, (int)s.n); ++nn) {
                    auto v = (juce::uint8)(s.acc ? 120 : s.vel);
                    seq.addEvent(juce::MidiMessage::noteOn(ch, s.notes[nn], v), on);
                    seq.addEvent(juce::MidiMessage::noteOff(ch, s.notes[nn]), off);
                }
            }
        }
        seq.updateMatchedPairs();
        mf.addTrack(seq);
    }
    auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("DarkFunkDurcal");
    dir.createDirectory();
    auto f = dir.getChildFile("DFD_" + juce::String(kStyles[engine.p.styleA % NSTYLES].name) + "_" + juce::String((int)engine.pat.bpm) + "bpm_"
                              + juce::Time::getCurrentTime().formatted("%H%M%S") + ".mid");
    juce::FileOutputStream os(f);
    if (os.openedOk()) mf.writeTo(os);
    return f;
}

juce::AudioProcessorEditor* DfdProcessor::createEditor() { return new DfdEditor(*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DfdProcessor(); }
