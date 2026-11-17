// Dark Funk Durcal Synt — the engine: sequencer synced to the host, the four instruments, effects and the live tools.
// No JUCE in here: the same code renders in the plugin and in the offline tests.
#pragma once
#include "Fx.h"
#include "Gen.h"
#include <atomic>
#include <string>

namespace dfd {

struct Params {
    DrumParams drum; AcidParams acid; BassParams bass; LeadParams lead; FxParams fx;
    float vol[NTRACKS] = {0.85f, 0.8f, 0.8f, 0.7f};
    float echoSend[NTRACKS] = {0.05f, 0.3f, 0.05f, 0.35f};
    float revSend[NTRACKS] = {0.1f, 0.15f, 0.05f, 0.4f};
    float swing = 0; float stutterRate = 0.125f;   // beats
    int styleA = 0, styleB = 2; float fusion = 0.3f, chaos = 0.4f; int mood = 1, key = 9; float density = 0.5f;
    int playTarget = TR_LEAD; bool scaleLock = true; bool keyTranspose = false;
};

enum Cmd : int {
    C_NONE, C_SUMMON, C_DICE_DRUMS, C_DICE_ACID, C_DICE_MELODY, C_DICE_SOUND, C_DICE_ALL, C_MAKE_MELODY,
    C_NEXT_GEN, C_NEXT_PICK, C_NEXT_KEEP, C_UNDO, C_REDO, C_SCENE_QUEUE, C_SCENE_STORE, C_MUTE_TOGGLE,
    C_FILL, C_STUTTER, C_BUILD, C_SET_STEP, C_CLEAR_TRACK, C_PLAY_TOGGLE, C_PAD
};
struct Command { int type = C_NONE; int a = 0, b = 0, c = 0; };

class Engine {
public:
    // ---------------------------------------------------------------- set-up
    void prepare(float sampleRate) {
        sr = sampleRate; kit.sr = sr; acidV.sr = sr; bassV.sr = sr; lead.sr = sr;
        echo.prepare(sr); rev.prepare(sr); stut.prepare(sr); master.prepare(sr);
        if (!seeded) { summonNow(); seeded = true; }
    }
    Params p;               // copied in by the host wrapper every block
    Pattern pat;            // the current pattern (all four tracks)
    Pattern scenes[8]; bool sceneUsed[8] = {false};
    Track nextCand[3]; int nextPick = -1; bool nextReady = false;
    bool mute[NTRACKS] = {false, false, false, false};
    std::atomic<int> uiStep{0}; std::atomic<float> intensity{0}; std::atomic<float> level{0};
    std::atomic<int> queuedScene{-1}; std::atomic<int> currentScene{0};
    bool internalPlay = false;   // standalone / no host transport
    std::atomic<const char*> lastAction{"SUMMONED"};   // literals only: read by the UI thread
    Gen gen; uint32_t seedCounter = 1;

    // UI / MIDI thread -> audio thread (single producer per side is fine: the wrapper serialises)
    void post(const Command& c) { int w = qw.load(); int nw = (w + 1) % QN; if (nw == qr.load()) return; q[w] = c; qw.store(nw); }

    // MIDI from the host, in the audio thread
    void noteOn(int ch, int note, float vel) {
        if ((ch == 10 || ch == 11) && note >= 36 && note <= 43) { pad(note - 36, true); return; }
        if (p.keyTranspose) { transpose = note - 60; return; }
        int n = p.scaleLock ? snapToScale(note, p.mood, p.key) : note;
        heldNote = n;
        switch (p.playTarget) {
            case TR_ACID: acidV.note((float)n, vel > 0.85f, liveHeld > 0, p.acid); break;
            case TR_BASS: bassV.note((float)n, vel, liveHeld > 0, p.bass); break;
            case TR_DRUMS: kit.trigger(note % NLANES, vel, p.drum); break;
            default: lead.noteOn((float)n, vel, p.lead); break;
        }
        ++liveHeld; liveNotes[note & 127] = (uint8_t)n;
    }
    void noteOff(int ch, int note) {
        if ((ch == 10 || ch == 11) && note >= 36 && note <= 43) { pad(note - 36, false); return; }
        if (p.keyTranspose) return;
        liveHeld = std::max(0, liveHeld - 1);
        int n = liveNotes[note & 127];
        if (p.playTarget == TR_LEAD) lead.noteOff((float)n);
        else if (liveHeld == 0) { acidV.release(); bassV.release(); }
    }
    void pitchBend(float v) { p.lead.bend = v; bendNow = v; }
    float bendNow = 0;

    // the 8 performance pads
    static constexpr const char* kPadNames[8] = {"MUTE DRUMS", "MUTE ACID", "MUTE BASS", "MUTE LEAD", "FILL", "STUTTER", "BUILD/DROP", "NEXT PHRASE"};
    void pad(int i, bool down) {
        if (i < 4) { if (down) mute[i] = !mute[i]; return; }
        if (i == 4) fill = down;
        if (i == 5) { if (down) stut.engage(p.stutterRate, bpm); else stut.release(); }
        if (i == 6) { if (down) { build = true; buildT = 0; } else if (build) { build = false; dropT = 1.0f; } }
        if (i == 7 && down) { if (!nextReady) doNextGen(); else { nextPick = (nextPick + 1) % 3; } }
        padState[i] = down;
    }
    bool padState[8] = {false};

    // ---------------------------------------------------------------- audio
    void process(float* outL, float* outR, int n, bool hostPlaying, double ppqStart, double hostBpm) {
        drainCommands();
        bpm = (float)(hostBpm > 1 ? hostBpm : pat.bpm);
        bool playing = hostPlaying || internalPlay;
        double ppq = hostPlaying ? ppqStart : internalPpq;
        const double ppqPerSample = bpm / 60.0 / sr;
        float peak = 0;
        for (int i = 0; i < n; ++i) {
            if (playing) tickSequencer(ppq);
            else if (wasPlaying) { acidV.release(); bassV.release(); lead.allOff(); lastStep = -1; }
            wasPlaying = playing;
            if (playing) ppq += ppqPerSample;
            float l, r; renderSample(l, r);
            outL[i] = l; outR[i] = r; peak = std::max(peak, std::fabs(l));
        }
        if (!hostPlaying && internalPlay) internalPpq = ppq;
        if (!playing) internalPpq = 0;
        level.store(peak);
        updateIntensity(peak, playing);
    }

private:
    static constexpr int QN = 64; Command q[QN]; std::atomic<int> qw{0}, qr{0};
    float sr = 48000, bpm = 120; bool seeded = false, wasPlaying = false; double internalPpq = 0;
    DrumKitSynth kit; AcidVoice acidV; BassVoice bassV; LeadSynth lead;
    TapeEcho echo; Reverb rev; Stutter stut; Master master;
    long lastStep = -1; int transpose = 0; int liveHeld = 0; int heldNote = 60; uint8_t liveNotes[128] = {0};
    bool fill = false, build = false; float buildT = 0, dropT = 0; Rng fxRng{99}; Svf riser;
    // per-step state
    double stepStartPpq = 0; int ratchetLeft[NTRACKS] = {0}; double ratchetNext[NTRACKS] = {0}; double ratchetEvery[NTRACKS] = {0};
    bool acidGateOn = false, bassGateOn = false; double acidOffAt = 1e18, bassOffAt = 1e18; std::vector<std::pair<double, float>> leadOffs;
    Pattern undoStack[16]; int undoTop = 0, undoCount = 0; Pattern redoPat; bool canRedo = false;

    void pushUndo() { undoStack[undoTop] = pat; undoTop = (undoTop + 1) % 16; undoCount = std::min(undoCount + 1, 16); canRedo = false; }

    void drainCommands() {
        while (qr.load() != qw.load()) { Command c = q[qr.load()]; qr.store((qr.load() + 1) % QN); exec(c); }
    }
    void exec(const Command& c) {
        if (c.type == C_SUMMON && c.a) seedCounter = (uint32_t)c.a;   // presets summon a fixed groove
        gen.rng = Rng(0x9E3779B9u * (++seedCounter) ^ (uint32_t)c.type);
        const Style& A = kStyles[p.styleA % NSTYLES]; (void)A;
        switch (c.type) {
            case C_SUMMON: pushUndo(); summonNow(); lastAction = "SUMMONED"; break;
            case C_DICE_DRUMS: { pushUndo(); int k = gen.rng.range(5); Track& t = pat.t[TR_DRUMS];
                if (k == 0) gen.drums(t, kStyles[p.styleA], kStyles[p.styleB], p.fusion, p.chaos); else if (k == 1) gen.breakChop(t, 0.6f);
                else if (k == 2) gen.dice_rolls(t); else if (k == 3) gen.dice_ghosts(t); else gen.dice_hats(t);
                static const char* N[5] = {"NEW GROOVE", "BREAK CHOP", "ROLLS", "GHOSTS", "HATS"}; lastAction = N[k]; break; }
            case C_DICE_ACID: { pushUndo(); int k = gen.rng.range(5); Track& t = pat.t[TR_ACID];
                if (k == 0) gen.acid(t, p.mood, p.key, 0.6f, 0.3f, 0.35f, p.chaos); else if (k == 1) gen.dice_acidNotes(t, p.mood, p.key);
                else if (k == 2) gen.dice_accents(t); else if (k == 3) gen.dice_slides(t); else gen.dice_octaves(t);
                static const char* N[5] = {"NEW LINE", "NOTES", "ACCENTS", "SLIDES", "OCTAVES"}; lastAction = N[k]; break; }
            case C_DICE_MELODY: { pushUndo(); int k = gen.rng.range(3);
                if (k == 0) gen.melody(pat.t[TR_LEAD], p.mood, p.key, gen.rng.range(Gen::NSHAPES), p.density);
                else if (k == 1) gen.chords(pat.t[TR_LEAD], p.mood, p.key, gen.rng.chance(0.5f), p.mood == M_SOUL || p.mood == M_FUNKY);
                else gen.bass(pat.t[TR_BASS], kStyles[p.styleA].bassType, p.mood, p.key, pat.t[TR_DRUMS]);
                static const char* N[3] = {"NEW MELODY", "NEW CHORDS", "NEW BASS"}; lastAction = N[k]; break; }
            case C_DICE_SOUND: { int k = gen.rng.range(4);
                if (k == 0) soundPick = 1 + gen.rng.range(NKITS); else if (k == 1) soundPick = 100 + gen.rng.range(NBASS); else if (k == 2) soundPick = 200 + gen.rng.range(NLEAD); else soundPick = 300 + gen.rng.range(100);
                lastAction = "NEW SOUND"; break; }
            case C_DICE_ALL: pushUndo(); for (int k = 0; k < 3; ++k) exec({C_DICE_DRUMS + k}); lastAction = "ALL DICE"; break;
            case C_MAKE_MELODY: pushUndo(); gen.melody(pat.t[TR_LEAD], p.mood, p.key, c.a, p.density); lastAction = "MELODY"; break;
            case C_NEXT_GEN: doNextGen(); break;
            case C_NEXT_PICK: if (nextReady) { nextPick = c.a; { static const char* T[3] = {"TRY 1", "TRY 2", "TRY 3"}; lastAction = T[c.a % 3]; } } break;
            case C_NEXT_KEEP: if (nextReady && nextPick >= 0) { pushUndo(); pat.t[TR_LEAD] = nextCand[nextPick]; nextReady = false; nextPick = -1; lastAction = "KEPT"; } break;
            case C_UNDO: if (undoCount > 0) { redoPat = pat; canRedo = true; undoTop = (undoTop + 15) % 16; --undoCount; pat = undoStack[undoTop]; lastAction = "UNDO"; } break;
            case C_REDO: if (canRedo) { pushUndo(); pat = redoPat; canRedo = false; lastAction = "REDO"; } break;
            case C_SCENE_STORE: if (c.a >= 0 && c.a < 8) { scenes[c.a] = pat; sceneUsed[c.a] = true; currentScene.store(c.a); { static const char* T[8] = {"STORED A", "STORED B", "STORED C", "STORED D", "STORED E", "STORED F", "STORED G", "STORED H"}; lastAction = T[c.a]; } } break;
            case C_SCENE_QUEUE: if (c.a >= 0 && c.a < 8) { if (!sceneUsed[c.a]) { scenes[c.a] = pat; sceneUsed[c.a] = true; } queuedScene.store(c.a); { static const char* T[8] = {"NEXT BAR: A", "NEXT BAR: B", "NEXT BAR: C", "NEXT BAR: D", "NEXT BAR: E", "NEXT BAR: F", "NEXT BAR: G", "NEXT BAR: H"}; lastAction = T[c.a]; } } break;
            case C_MUTE_TOGGLE: if (c.a >= 0 && c.a < NTRACKS) mute[c.a] = !mute[c.a]; break;
            case C_FILL: fill = c.a != 0; break;
            case C_STUTTER: if (c.a) stut.engage(p.stutterRate, bpm); else stut.release(); break;
            case C_BUILD: pad(6, c.a != 0); break;
            case C_SET_STEP: { pushUndo(); Track& t = pat.t[c.a]; Step& s = t.s[c.b % MAXSTEPS];
                if (c.a == TR_DRUMS) { s.hits ^= (uint16_t)(1u << c.c); s.gate = s.hits ? 1 : 0; }
                else { if (s.gate == 1) s.gate = 0; else Gen::setNote(s, degNote(p.mood, p.key, c.c, c.a == TR_LEAD ? 4 : c.a == TR_BASS ? 1 : 2)); }
                break; }
            case C_CLEAR_TRACK: pushUndo(); pat.t[c.a].clear(); break;
            case C_PLAY_TOGGLE: internalPlay = !internalPlay; break;
            case C_PAD: pad(c.a, c.b != 0); break;
        }
    }
public:
    int soundPick = 0;   // the wrapper applies sound changes (it owns the parameters), then clears this
private:
    void doNextGen() { gen.nextPhrase(pat.t[TR_LEAD], nextCand, p.mood, p.key); nextReady = true; nextPick = 0; lastAction = "3 NEXT PHRASES"; }

public:
    void summonNow() {
        const Style& a = kStyles[p.styleA % NSTYLES]; const Style& b = kStyles[p.styleB % NSTYLES];
        const Style& m = gen.rng.uni() < p.fusion ? b : a;
        p.mood = m.mood;
        pat.bpm = (float)(int)lerp((float)lerp((float)a.bpmLo, (float)b.bpmLo, p.fusion), (float)lerp((float)a.bpmHi, (float)b.bpmHi, p.fusion), gen.rng.uni());
        pat.swing = lerp(a.swing, b.swing, p.fusion);
        gen.drums(pat.t[TR_DRUMS], a, b, p.fusion, p.chaos);
        gen.acid(pat.t[TR_ACID], p.mood, p.key, lerp(a.acidDens, b.acidDens, p.fusion), lerp(a.acidSlide, b.acidSlide, p.fusion), lerp(a.acidAcc, b.acidAcc, p.fusion), p.chaos);
        gen.bass(pat.t[TR_BASS], (gen.rng.uni() < p.fusion ? b : a).bassType, p.mood, p.key, pat.t[TR_DRUMS]);
        const Style& ls = gen.rng.uni() < p.fusion ? b : a;
        if (ls.leadType == LT_CHORDS) gen.chords(pat.t[TR_LEAD], p.mood, p.key, false, p.mood == M_SOUL || p.mood == M_FUNKY);
        else if (ls.leadType == LT_STABS) gen.chords(pat.t[TR_LEAD], p.mood, p.key, true, false);
        else if (ls.leadType == LT_ARP) gen.arp(pat.t[TR_LEAD], p.mood, p.key);
        else gen.melody(pat.t[TR_LEAD], p.mood, p.key, gen.rng.range(Gen::NSHAPES), ls.leadDens);
        summonedStyle = &m; summonedLead = &ls; summoned = true;
    }
    const Style* summonedStyle = nullptr; const Style* summonedLead = nullptr; bool summoned = false;  // the wrapper applies the style's sounds
    bool isPadDown(int i) const { return padState[i]; }
    bool isFill() const { return fill; } bool isBuild() const { return build; }
    float curBpm() const { return bpm; }

private:
    // ---------------------------------------------------------------- sequencer
    void tickSequencer(double ppq) {
        double sw = (double)std::max(p.swing, pat.swing) * 0.33;
        long step16 = (long)std::floor(ppq * 4.0);
        double frac = ppq * 4.0 - step16;
        bool odd = step16 & 1;
        bool due = !odd ? true : frac >= sw;
        if (step16 != lastStep && due) {
            lastStep = step16;
            if (step16 % 16 == 0) { int qs = queuedScene.load(); if (qs >= 0 && sceneUsed[qs]) { pat = scenes[qs]; currentScene.store(qs); queuedScene.store(-1); } }
            stepStartPpq = ppq; triggerStep(step16, ppq);
            uiStep.store((int)(step16 % 64));
        }
        // ratchets
        for (int t = 0; t < NTRACKS; ++t) if (ratchetLeft[t] > 0 && ppq >= ratchetNext[t]) { --ratchetLeft[t]; ratchetNext[t] += ratchetEvery[t]; retrigger(t); }
        // gates
        if (acidGateOn && ppq >= acidOffAt) { acidV.release(); acidGateOn = false; }
        if (bassGateOn && ppq >= bassOffAt) { bassV.release(); bassGateOn = false; }
        for (size_t k = 0; k < leadOffs.size();) { if (ppq >= leadOffs[k].first) { lead.noteOff(leadOffs[k].second); leadOffs[k] = leadOffs.back(); leadOffs.pop_back(); } else ++k; }
        if (build) buildT += (float)(1.0 / sr);
    }
    Step lastFired[NTRACKS];
    void triggerStep(long step16, double ppq) {
        const double stepLen = 0.25;
        for (int t = 0; t < NTRACKS; ++t) {
            const Track& tr = pat.t[t]; int len = std::max(1, tr.len);
            int i = (int)(step16 % len);
            Step s = tr.s[i];
            if (t == TR_LEAD && nextReady && nextPick >= 0) s = nextCand[nextPick].s[i % std::max(1, nextCand[nextPick].len)];
            if (t == TR_DRUMS && (fill || build)) s = fillStep(step16);
            if (mute[t] || (dropT > 0 && t != TR_LEAD && dropT > 0.5f)) { if (t == TR_ACID && s.gate != 2) { acidV.release(); acidGateOn = false; } continue; }
            if (s.gate == 0) { if (t == TR_ACID) { acidV.release(); acidGateOn = false; } if (t == TR_BASS) { bassV.release(); bassGateOn = false; } continue; }
            if (s.gate == 2) { // tie: extend the previous note
                if (t == TR_ACID) acidOffAt = ppq + stepLen * 0.55; if (t == TR_BASS) bassOffAt = ppq + stepLen * 0.9;
                for (auto& o : leadOffs) o.first = std::max(o.first, ppq + stepLen * 0.95);
                continue;
            }
            if (s.prob < 100 && gen.rng.range(100) >= s.prob) continue;
            lastFired[t] = s;
            const Step& nxt = tr.s[(i + 1) % len];
            fire(t, s, nxt, ppq, stepLen);
            if (s.ratchet > 1) { ratchetLeft[t] = s.ratchet - 1; ratchetEvery[t] = stepLen / s.ratchet; ratchetNext[t] = ppq + ratchetEvery[t]; }
        }
    }
    void fire(int t, const Step& s, const Step& nxt, double ppq, double stepLen) {
        float vel = s.vel / 127.0f;
        switch (t) {
            case TR_DRUMS:
                for (int L = 0; L < NLANES; ++L) if (s.hits >> L & 1) {
                    float v = (s.accHits >> L & 1) ? 1.0f : vel * 0.85f;
                    kit.trigger(L, v, p.drum, (gen.rng.uni() - 0.5f) * 0.02f);
                }
                break;
            case TR_ACID: {
                bool slideIn = lastAcidSlide; lastAcidSlide = s.slide;
                acidV.note((float)(s.notes[0] + transpose), s.acc, slideIn, p.acid); acidGateOn = true;
                acidOffAt = ppq + stepLen * ((s.slide || nxt.gate == 2) ? 1.05 : 0.55); break; }
            case TR_BASS:
                bassV.note((float)(s.notes[0] + transpose), vel, s.slide, p.bass); bassGateOn = true;
                bassOffAt = ppq + stepLen * ((s.slide || nxt.gate == 2) ? 1.05 : 0.8); break;
            case TR_LEAD:
                for (int k = 0; k < s.n; ++k) { lead.noteOn((float)(s.notes[k] + transpose), vel, p.lead); leadOffs.push_back({ppq + stepLen * (nxt.gate == 2 ? 1.0 : 0.9), (float)(s.notes[k] + transpose)}); }
                break;
        }
    }
    bool lastAcidSlide = false;
    void retrigger(int t) { const Step& s = lastFired[t]; if (t == TR_DRUMS) { for (int L = 0; L < NLANES; ++L) if (s.hits >> L & 1) kit.trigger(L, s.vel / 127.0f * 0.8f, p.drum); } else if (t == TR_ACID) acidV.note((float)(s.notes[0] + transpose), s.acc, false, p.acid); }
    Step fillStep(long step16) {
        Step s; s.gate = 1; s.vel = 110; int pos = (int)(step16 % 16);
        if (build) { s.hits = 1u << SNARE; s.ratchet = (uint8_t)(buildT < 2 ? 1 : buildT < 4 ? 2 : 4); s.vel = (uint8_t)std::min(127.0f, 70.0f + buildT * 12.0f); if (pos % 4 == 0) s.hits |= 1u << KICK; return s; }
        s.hits = (pos % 2 == 0) ? (1u << SNARE) : (1u << TOM); if (pos % 4 == 0) s.hits |= 1u << KICK; s.ratchet = (uint8_t)(pos >= 12 ? 3 : 1); return s;
    }

    // ---------------------------------------------------------------- mixing
    void renderSample(float& L, float& R) {
        float d = kit.tick(p.drum), a = acidV.tick(p.acid), b = bassV.tick(p.bass), l = lead.tick(p.lead);
        float tr[NTRACKS] = {d * p.vol[0], a * p.vol[1], b * p.vol[2], l * p.vol[3]};
        float eIn = 0, rIn = 0;
        for (int t = 0; t < NTRACKS; ++t) { eIn += tr[t] * p.echoSend[t]; rIn += tr[t] * p.revSend[t]; }
        float el, er, rl, rr; echo.process(eIn, eIn, el, er, p.fx, bpm); rev.process(rIn, rIn, rl, rr, p.fx);
        float dry = tr[0] + tr[1] + tr[2] + tr[3];
        L = dry + el * p.fx.echoMix * 1.6f + rl * p.fx.revMix * 1.4f;
        R = dry + er * p.fx.echoMix * 1.6f + rr * p.fx.revMix * 1.4f;
        // build: high-pass rises, a noise riser; drop: a sub thump and the lows come back
        FxParams fx = p.fx;
        if (build) { float t = std::min(buildT / 8.0f, 1.0f); fx.filter = std::max(fx.filter, t * 0.75f);
            riser.set(400.0f + t * 7000.0f, 2.0f, sr); float nz = riser.bp(fxRng.bi()) * t * 0.25f; L += nz; R += nz; }
        if (dropT > 0) { dropT -= 1.0f / (sr * 60.0f / bpm * 4.0f) * 2.0f; }
        stut.process(L, R);
        master.process(L, R, fx);
    }
    void updateIntensity(float peak, bool playing) {
        float tempo = clampf((bpm - 95.0f) / 85.0f, 0, 1), dens = 0;
        for (int i = 0; i < 16; ++i) for (int t = 0; t < NTRACKS; ++t) dens += pat.t[t].s[i].gate == 1 ? 1.0f : 0.0f;
        dens = clampf(dens / 40.0f, 0, 1);
        float target = playing ? clampf(peak * 1.2f, 0, 1) * 0.3f + tempo * 0.28f + dens * 0.2f + p.acid.drive * 0.12f + p.acid.reso * 0.1f + (build ? 0.3f : 0) : 0.08f;
        float cur = intensity.load(); intensity.store(cur + (target - cur) * 0.05f);
    }
};

} // namespace dfd
