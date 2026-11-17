// Dark Funk Durcal Synt — the four instruments: drum kit, acid line, bass, lead.
#pragma once
#include "Common.h"

namespace dfd {

// ============================================================================ DRUMS
enum DrumLane { KICK, SNARE, CLAP, CHAT, OHAT, TOM, RIM, BELL, NLANES };
static const char* const kLaneNames[NLANES] = {"KICK", "SNARE", "CLAP", "CL HAT", "OP HAT", "TOM", "RIM", "BELL"};
enum DrumKit { KIT_909, KIT_808, KIT_WARP, KIT_BREAK, KIT_TOY, NKITS };
static const char* const kKitNames[NKITS] = {"GRIT 909", "DEEP 808", "WARPED", "LIVE BREAK", "TOY BOX"};

struct DrumParams { int kit = 0; float pitch = 0, decay = 0.5f, crush = 0, kickTune = 0.5f, kickDecay = 0.5f, snareTone = 0.5f, hatDecay = 0.5f, vol = 0.8f; };

struct DrumVoice {
    int lane = 0; float sr = 48000;
    Env amp, pitchEnv, noiseEnv; Osc o1, o2; Osc metal[6]; Svf f1, f2; Rng rng{1234};
    float vel = 1, basePitch = 60, burst = 0; int burstN = 0; bool on = false;
    float crushHold = 0; int crushCount = 0;
    void trigger(int ln, float v, const DrumParams& p, float sampleRate, float micro) {
        lane = ln; sr = sampleRate; vel = v; on = true; o1.ph = o2.ph = 0; burstN = 0; burst = 0;
        const float k = (float)p.kit, dec = 0.35f + p.decay * 1.3f, pm = std::pow(2.0f, p.pitch / 12.0f + micro);
        switch (lane) {
            case KICK: {
                float d = (p.kit == KIT_808 ? 0.9f : p.kit == KIT_TOY ? 0.12f : p.kit == KIT_WARP ? 0.22f : 0.38f) * (0.4f + p.kickDecay * 1.4f);
                amp.setTimes(0.0008f, d, sr); amp.trigger(1);
                pitchEnv.setTimes(0.0005f, p.kit == KIT_808 ? 0.09f : 0.05f, sr); pitchEnv.trigger(1);
                basePitch = (p.kit == KIT_808 ? 40.0f : p.kit == KIT_TOY ? 70.0f : 46.0f) * (0.7f + p.kickTune * 0.6f) * pm;
                noiseEnv.setTimes(0.0002f, 0.004f, sr); noiseEnv.trigger(p.kit == KIT_BREAK ? 0.6f : 0.35f);
                break; }
            case SNARE: {
                amp.setTimes(0.0005f, (p.kit == KIT_BREAK ? 0.18f : p.kit == KIT_TOY ? 0.06f : 0.12f) * dec, sr); amp.trigger(1);
                noiseEnv.setTimes(0.0005f, (p.kit == KIT_808 ? 0.16f : p.kit == KIT_BREAK ? 0.26f : 0.2f) * dec, sr); noiseEnv.trigger(1);
                basePitch = (160.0f + p.snareTone * 90.0f) * pm; pitchEnv.setTimes(0.0005f, 0.03f, sr); pitchEnv.trigger(1);
                f1.set(p.kit == KIT_WARP ? 3200.0f : 1800.0f + p.snareTone * 2500.0f, 0.8f, sr);
                break; }
            case CLAP: {
                amp.setTimes(0.0005f, 0.22f * dec, sr); amp.trigger(1); burstN = 3; burst = 0;
                f1.set(1100.0f * pm, 1.6f, sr); break; }
            case CHAT: case OHAT: {
                float d = lane == CHAT ? 0.035f + p.hatDecay * 0.06f : 0.18f + p.hatDecay * 0.5f;
                if (p.kit == KIT_TOY) d *= 0.6f;
                amp.setTimes(0.0004f, d * dec, sr); amp.trigger(1);
                static const float R[6] = {205.3f, 304.4f, 369.6f, 522.7f, 540.0f, 800.0f};
                for (int i = 0; i < 6; ++i) metal[i].ph = rng.uni();
                basePitch = (p.kit == KIT_808 ? 1.0f : p.kit == KIT_WARP ? 1.6f : 1.25f) * pm; (void)R;
                f1.set(p.kit == KIT_TOY ? 6500.0f : 8000.0f, 0.9f, sr); f2.set(7000.0f, 0.7f, sr);
                break; }
            case TOM: {
                amp.setTimes(0.0005f, 0.32f * dec, sr); amp.trigger(1);
                pitchEnv.setTimes(0.0005f, 0.12f, sr); pitchEnv.trigger(1); basePitch = (p.kit == KIT_808 ? 90.0f : 120.0f) * pm; break; }
            case RIM: {
                amp.setTimes(0.0002f, 0.03f, sr); amp.trigger(1); basePitch = 1700.0f * pm; f1.set(1700.0f * pm, 4.0f, sr); break; }
            case BELL: {
                amp.setTimes(0.0004f, (p.kit == KIT_TOY ? 0.6f : 0.25f) * dec, sr); amp.trigger(1);
                basePitch = (p.kit == KIT_TOY ? 1320.0f : 540.0f) * pm; f1.set(basePitch * 1.4f, 2.0f, sr); break; }
        }
        (void)k;
    }
    float tick(const DrumParams& p) {
        if (!on) return 0;
        float out = 0, a = amp.tick();
        switch (lane) {
            case KICK: {
                float pe = pitchEnv.tick(), f = basePitch * (1.0f + pe * (p.kit == KIT_WARP ? 7.0f : 4.0f));
                float s = o1.sine(f / sr);
                float click = noiseEnv.tick() * rng.bi();
                out = softclip((s * 1.6f + click) * (p.kit == KIT_WARP || p.kit == KIT_909 ? 1.8f : 1.2f)) * a; break; }
            case SNARE: {
                float pe = pitchEnv.tick(), tone = o1.sine(basePitch * (1.0f + pe * 0.6f) / sr) * 0.6f + o2.tri(basePitch * 1.48f / sr) * 0.25f;
                float n = f1.hp(rng.bi()) * noiseEnv.tick();
                out = (tone * a + n * (p.kit == KIT_BREAK ? 1.1f : 0.9f)); if (p.kit == KIT_WARP) out = softclip(out * 2.2f); break; }
            case CLAP: {
                burst += 1.0f / sr;
                float e;
                if (burstN > 0) { float ph = std::fmod(burst, 0.011f) / 0.011f; e = std::exp(-ph * 6.0f); if (burst > 0.011f * 3) burstN = 0; }
                else e = a;
                out = f1.bp(rng.bi()) * e * 2.2f; break; }
            case CHAT: case OHAT: {
                static const float R[6] = {205.3f, 304.4f, 369.6f, 522.7f, 540.0f, 800.0f};
                float m = 0; for (int i = 0; i < 6; ++i) m += metal[i].sqr(R[i] * basePitch / sr);
                m = f2.hp(f1.bp(m * 0.25f + rng.bi() * 0.15f));
                out = m * a * 1.6f; break; }
            case TOM: { float pe = pitchEnv.tick(); out = o1.sine(basePitch * (1.0f + pe) / sr) * a; break; }
            case RIM: { out = f1.bp(o1.sqr(basePitch / sr) + rng.bi() * 0.4f) * a * 2.5f; break; }
            case BELL: { out = f1.bp(o1.sqr(basePitch / sr) * 0.5f + o2.sqr(basePitch * 1.5f / sr) * 0.5f) * a * 1.4f; break; }
        }
        // bit/rate crush (the WARPED kit lives on it)
        float cr = p.crush + (p.kit == KIT_WARP ? 0.35f : p.kit == KIT_TOY ? 0.25f : 0);
        if (cr > 0.01f) {
            int hold = 1 + (int)(cr * cr * 24.0f);
            if (++crushCount >= hold) { crushCount = 0; float q = std::pow(2.0f, 12.0f - cr * 9.0f); crushHold = std::round(out * q) / q; }
            out = crushHold;
        }
        if (!amp.active() && burstN == 0) on = false;
        return out * vel;
    }
};

struct DrumKitSynth {
    DrumVoice v[NLANES]; float sr = 48000;
    void trigger(int lane, float vel, const DrumParams& p, float micro = 0) {
        v[lane].trigger(lane, vel, p, sr, micro);
        if (lane == CHAT) v[OHAT].on = false;  // closed hat chokes open
    }
    float tick(const DrumParams& p) { float s = 0; for (auto& x : v) s += x.tick(p); return s * 0.55f * p.vol; }
};

// ============================================================================ ACID (303-style)
struct AcidParams { float cutoff = 0.35f, reso = 0.7f, envmod = 0.6f, decay = 0.35f, accent = 0.6f, slide = 0.5f, wave = 0, drive = 0.3f, tune = 0, vol = 0.8f; };

struct AcidVoice {
    float sr = 48000, freq = 110, target = 110, gate = 0, accentAmt = 0, accentCharge = 0;
    Osc osc; Env fenv, aenv; float s1 = 0, s2 = 0, s3 = 0, s4 = 0; OnePole hpf, dc;
    bool sliding = false;
    void note(float n, bool accent, bool slide, const AcidParams& p) {
        target = mtof(n + p.tune * 12.0f);
        if (!slide || gate == 0) { freq = target; fenv.setTimes(0.002f, 0.05f + p.decay * 1.6f, sr); fenv.trigger(1); aenv.setTimes(0.002f, 0.6f, sr); aenv.trigger(1); }
        sliding = slide && gate > 0; gate = 1;
        if (accent) { accentCharge = std::min(1.0f, accentCharge * 0.6f + 0.6f); accentAmt = p.accent; fenv.setTimes(0.002f, 0.09f + p.decay * 0.4f, sr); fenv.trigger(1); }
        else { accentAmt = 0; accentCharge *= 0.6f; }
    }
    void release() { gate = 0; }
    float tick(const AcidParams& p) {
        float glide = sliding ? (0.02f + p.slide * 0.12f) : 0.0015f;
        freq += (target - freq) * (1.0f - std::exp(-1.0f / (glide * sr)));
        float dt = freq / sr;
        // saw and square from the same phase, crossfaded by WAVE
        float ph = osc.ph;
        float saw = 2.0f * ph - 1.0f - polyblep(ph, dt);
        float sq = (ph < 0.5f ? 1.0f : -1.0f) + polyblep(ph, dt) - polyblep(std::fmod(ph + 0.5f, 1.0f), dt);
        osc.adv(dt);
        float raw = lerp(saw, sq * 0.8f, clampf(p.wave, 0.0f, 1.0f));
        float fe = fenv.tick();
        float cut = 30.0f * std::pow(2.0f, (p.cutoff * 9.0f) + fe * p.envmod * 5.5f + accentAmt * accentCharge * fe * 2.5f);
        cut = clampf(cut, 25.0f, sr * 0.42f);
        float g = std::tan(kPi * cut / sr), G = g / (1.0f + g);
        float res = p.reso * (3.95f + accentAmt * 0.3f);
        // 4-pole ladder with soft feedback (diode-ish squelch)
        float fb = softclip(s4 * res);
        float x = softclip((raw - fb) * 1.2f);
        s1 += G * (x - s1); s2 += G * (s1 - s2); s3 += G * (s2 - s3); s4 += G * (s3 - s4);
        float out = s4 * (1.0f + res * 0.35f);
        hpf.set(40.0f, sr); out = hpf.hp(out);
        float a = aenv.tick() * (gate > 0 ? 1.0f : 0.0f);
        if (gate == 0) { aenvRel = aenvRel * 0.995f; a = aenvRel; } else aenvRel = std::max(a, aenvRel * 0.999f);
        out *= a * (1.0f + accentAmt * 0.8f);
        if (p.drive > 0.01f) out = softclip(out * (1.0f + p.drive * 7.0f)) / (1.0f + p.drive * 1.5f);
        return out * p.vol * 0.9f;
    }
    float aenvRel = 0;
};

// ============================================================================ BASS
enum BassMode { BASS_SUB, BASS_REESE, BASS_WAH, BASS_SLAP, BASS_FM, NBASS };
static const char* const kBassNames[NBASS] = {"SUB", "REESE", "RUBBER WAH", "SLAP", "FM BASS"};
struct BassParams { int mode = 0; float cutoff = 0.5f, wah = 0.5f, glide = 0.2f, drive = 0.2f, tune = 0, detune = 0.3f, vol = 0.8f; };

struct BassVoice {
    float sr = 48000, freq = 55, target = 55; bool gate = false; Osc o1, o2, o3, mod; Svf flt; Env fe, ae, pe; float rel = 0;
    void note(float n, float vel, bool legato, const BassParams& p) {
        target = mtof(n + p.tune * 12.0f);
        if (!legato || !gate) {
            freq = target * (p.mode == BASS_SLAP ? 1.0f : 1.0f);
            fe.setTimes(0.002f, p.mode == BASS_SLAP ? 0.12f : p.mode == BASS_WAH ? 0.28f : 0.35f, sr); fe.trigger(vel);
            pe.setTimes(0.0005f, p.mode == BASS_WAH ? 0.09f : 0.03f, sr); pe.trigger(1);
        }
        gate = true; rel = 1;
    }
    void release() { gate = false; }
    float tick(const BassParams& p) {
        float gl = 0.002f + p.glide * 0.15f;
        freq += (target - freq) * (1.0f - std::exp(-1.0f / (gl * sr)));
        float pev = pe.tick(), fev = fe.tick();
        float f = freq * (p.mode == BASS_WAH ? (1.0f - pev * 0.09f) : p.mode == BASS_SLAP ? (1.0f + pev * 0.05f) : 1.0f);
        float dt = f / sr, s = 0;
        switch (p.mode) {
            case BASS_SUB: s = o1.sine(dt) * 0.9f + o2.tri(dt * 2.0f) * 0.12f; break;
            case BASS_REESE: { float d = 1.0f + p.detune * 0.012f; s = (o1.saw(dt * d) + o2.saw(dt / d)) * 0.5f + o3.sine(dt * 0.5f) * 0.5f;
                flt.set(120.0f + p.cutoff * 1800.0f, 0.9f, sr); s = flt.lp(s) + o3.sine(0) * 0; break; }
            case BASS_WAH: { s = o1.saw(dt) * 0.7f + o2.sqr(dt * 0.5f, 0.35f) * 0.4f;
                float c = 150.0f * std::pow(2.0f, p.cutoff * 3.0f + fev * p.wah * 4.0f);
                flt.set(c, 2.5f + p.wah * 4.0f, sr); float b, h; float lpv = flt.tick(s, b, h); s = lpv * 0.6f + b * 1.1f; break; }
            case BASS_SLAP: { s = o1.saw(dt) * 0.6f + o2.sqr(dt, 0.25f) * 0.3f;
                flt.set(90.0f * std::pow(2.0f, p.cutoff * 2.5f + fev * 5.0f), 1.2f, sr); s = flt.lp(s) + o3.sine(dt) * 0.4f; break; }
            case BASS_FM: { float m = mod.sine(dt * 2.0f) * (1.0f + fev * 4.0f * p.wah); o1.ph += m * 0.02f; o1.adv(0); s = o1.sine(dt); break; }
        }
        float a = gate ? 1.0f : (rel *= std::exp(-1.0f / (0.06f * sr)));
        if (gate) rel = 1;
        s *= a;
        if (p.drive > 0.01f) s = softclip(s * (1.0f + p.drive * 5.0f)) / (1.0f + p.drive);
        return s * p.vol;
    }
};

// ============================================================================ LEAD (poly)
enum LeadMode { LEAD_EP, LEAD_PAD, LEAD_VOX, LEAD_TOY, LEAD_BRAIN, LEAD_STAB, LEAD_BELL, NLEAD };
static const char* const kLeadNames[NLEAD] = {"VELVET EP", "ANALOG HAZE", "VOX FORMANT", "MUSIC BOX", "BRAIN SQUARE", "RAVE STAB", "GLASS BELL"};
struct LeadParams { int mode = 0; float bright = 0.5f, attack = 0.05f, release = 0.4f, vowel = 0.3f, vibrato = 0.2f, detune = 0.3f, vol = 0.7f; float bend = 0; };

struct LeadVoice {
    float sr = 48000, freq = 440, target = 440, note = 60, vel = 1, age = 0; bool on = false;
    Osc o[4]; Osc lfo; Adsr env; Env idx; Svf flt, fm1, fm2, fm3; Rng rng{77}; float crushH = 0; int crushC = 0;
    void start(float n, float v, const LeadParams& p) {
        note = n; vel = v; target = mtof(n); on = true; age = 0;
        freq = p.mode == LEAD_VOX ? target * 0.94f : target;  // vocal scoop
        env.sr = sr;
        switch (p.mode) {
            case LEAD_EP: env.a = 0.002f; env.d = 1.2f; env.s = 0.25f; env.r = 0.35f + p.release; break;
            case LEAD_PAD: env.a = 0.25f + p.attack * 2.0f; env.d = 0.8f; env.s = 0.85f; env.r = 0.8f + p.release * 2; break;
            case LEAD_VOX: env.a = 0.03f + p.attack; env.d = 0.3f; env.s = 0.85f; env.r = 0.15f + p.release; break;
            case LEAD_TOY: env.a = 0.001f; env.d = 0.9f; env.s = 0.0f; env.r = 0.4f; break;
            case LEAD_BRAIN: env.a = 0.003f; env.d = 0.25f; env.s = 0.6f; env.r = 0.08f + p.release * 0.5f; break;
            case LEAD_STAB: env.a = 0.001f; env.d = 0.22f; env.s = 0.0f; env.r = 0.15f; break;
            case LEAD_BELL: env.a = 0.001f; env.d = 1.6f; env.s = 0.0f; env.r = 0.8f; break;
        }
        env.on(); idx.setTimes(0.001f, p.mode == LEAD_BELL ? 1.2f : 0.6f, sr); idx.trigger(1);
        for (auto& x : o) x.ph = rng.uni();
    }
    void stop() { env.off(); }
    float tick(const LeadParams& p) {
        if (!on) return 0;
        age += 1.0f / sr;
        freq += (target - freq) * (1.0f - std::exp(-1.0f / (0.045f * sr)));
        float vib = lfo.sine(5.2f / sr) * p.vibrato * (age > 0.25f ? 1.0f : age * 4.0f);
        float f = freq * std::pow(2.0f, (vib * 0.6f + p.bend * 2.0f) / 12.0f), dt = f / sr;
        float e = env.tick(), s = 0;
        switch (p.mode) {
            case LEAD_EP: { // FM tine: carrier 1:1, modulator 1:1 with decaying index + 14:1 "tine" click
                float ix = idx.tick();
                float m = o[1].sine(dt) * (0.8f + p.bright * 2.5f) * (0.3f + ix);
                float tine = o[2].sine(dt * 14.0f) * ix * 0.15f * p.bright;
                o[0].ph += m * dt * 0.5f; if (o[0].ph >= 1) o[0].ph -= 1; if (o[0].ph < 0) o[0].ph += 1;
                s = o[0].sine(dt) * 0.8f + tine;
                s *= 1.0f + 0.25f * std::sin(age * kTwoPi * 4.5f) * p.vibrato; break; }
            case LEAD_PAD: { float d = 1.0f + p.detune * 0.01f;
                s = (o[0].saw(dt * d) + o[1].saw(dt / d) + o[2].saw(dt * 0.5f) * 0.5f) * 0.4f;
                flt.set(300.0f + p.bright * 4000.0f + 600.0f * std::sin(age * 0.7f), 0.8f, sr); s = flt.lp(s); break; }
            case LEAD_VOX: { // glottal-ish pulse into three formant band-passes, vowel morphs with p.vowel and the note's life
                static const float F[5][3] = {{730, 1090, 2440}, {530, 1840, 2480}, {270, 2290, 3010}, {570, 840, 2410}, {300, 870, 2240}};
                float vw = clampf(p.vowel + 0.18f * std::sin(age * kTwoPi * 0.8f) + (age < 0.12f ? (0.12f - age) * 2.0f : 0), 0.0f, 0.999f) * 4.0f;
                int i0 = (int)vw; float t = vw - i0; int i1 = std::min(i0 + 1, 4);
                float src = o[0].saw(dt) * 0.6f + o[1].sqr(dt, 0.15f) * 0.4f + rng.bi() * 0.04f;
                fm1.set(lerp(F[i0][0], F[i1][0], t), 9.0f, sr); fm2.set(lerp(F[i0][1], F[i1][1], t), 11.0f, sr); fm3.set(lerp(F[i0][2], F[i1][2], t), 13.0f, sr);
                s = fm1.bp(src) * 1.4f + fm2.bp(src) * 1.0f + fm3.bp(src) * (0.4f + p.bright * 0.6f); s *= 1.6f; break; }
            case LEAD_TOY: { float ix = idx.tick(); float m = o[1].sine(dt * 3.5f) * ix * 1.2f; o[0].ph += m * dt; if (o[0].ph >= 1) o[0].ph -= 1; if (o[0].ph < 0) o[0].ph += 1;
                s = o[0].sine(dt) * 0.8f + o[2].sine(dt * 4.0f * (1.0f + p.detune * 0.003f)) * 0.2f * ix; break; }
            case LEAD_BRAIN: { float pw = 0.5f + 0.35f * std::sin(age * kTwoPi * (0.5f + p.vibrato * 6.0f));
                s = o[0].sqr(dt, pw) * 0.6f + o[1].sqr(dt * 2.0f * (1.0f + p.detune * 0.004f), 0.5f) * 0.25f;
                flt.set(400.0f + p.bright * 6000.0f, 1.4f, sr); s = flt.lp(s);
                if (++crushC >= 3) { crushC = 0; crushH = std::round(s * 24.0f) / 24.0f; } s = lerp(s, crushH, 0.5f); break; }
            case LEAD_STAB: { float d = 1.0f + p.detune * 0.012f;
                s = (o[0].saw(dt) + o[1].saw(dt * d) + o[2].saw(dt / d) + o[3].saw(dt * 2.0f) * 0.4f) * 0.3f;
                float ix = idx.tick(); flt.set(500.0f + (p.bright * 5000.0f) * (0.3f + ix), 1.2f, sr); s = flt.lp(s); break; }
            case LEAD_BELL: { float ix = idx.tick(); float m = o[1].sine(dt * 3.5f) * ix * (1.0f + p.bright * 3.0f);
                o[0].ph += m * dt; if (o[0].ph >= 1) o[0].ph -= 1; if (o[0].ph < 0) o[0].ph += 1; s = o[0].sine(dt) * 0.8f; break; }
        }
        if (!env.active()) on = false;
        return s * e * vel;
    }
};

struct LeadSynth {
    static constexpr int N = 8; LeadVoice v[N]; float sr = 48000; int rr = 0;
    void noteOn(float n, float vel, const LeadParams& p) {
        int pick = -1; for (int i = 0; i < N; ++i) if (!v[i].on) { pick = i; break; }
        if (pick < 0) { pick = rr; rr = (rr + 1) % N; }
        v[pick].sr = sr; v[pick].start(n, vel, p);
    }
    void noteOff(float n) { for (auto& x : v) if (x.on && std::fabs(x.note - n) < 0.01f && x.env.st != 4) x.stop(); }
    void allOff() { for (auto& x : v) x.stop(); }
    float tick(const LeadParams& p) { float s = 0; for (auto& x : v) s += x.tick(p); return s * 0.32f * p.vol; }
};

} // namespace dfd
