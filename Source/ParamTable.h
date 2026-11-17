// Dark Funk Durcal Synt — every automatable parameter, in KeyLab / Ableton bank order (8 per bank).
// The order IS the controller map: bank 1 = encoders 1-8 on the first page, and so on. Never reorder (saved sets
// store parameter indices); append only.
#pragma once
#include "dsp/Engine.h"

namespace dfd {

enum ParamKind { PK_FLOAT, PK_INT, PK_BOOL };
struct ParamDef {
    const char* id; const char* name; ParamKind kind; float lo, hi, def;
    const char* const* choices; int nChoices;
    void (*apply)(Params&, float);
};

static const char* const kStyleNames[NSTYLES] = {"ACID", "DRILL N BASS", "JUNGLE", "ROLLING DNB", "BREAKCORE", "BIG BEAT", "RAVE HARDCORE",
    "BRAINDANCE", "DARK ELECTRO", "AMBIENT IDM", "BROKEN FUNK", "WARPED FUNK", "SLAP DNB", "MUSIC BOX IDM", "SPECTRAL STEP"};
static const char* const kHeadNames[8] = {"DIGITAL", "H1", "H2", "H1+2", "H3", "H1+3", "H2+3", "ALL"};
static const char* const kTargetNames[4] = {"DRUMS", "ACID", "BASS", "LEAD"};
static const char* const kStutNames[4] = {"1/4", "1/8", "1/16", "1/32"};

#define F(expr) [](Params& p, float v) { expr; }
static const ParamDef kParams[] = {
    // ---- bank 1: PERFORM (the hands-on page)
    {"filter", "DJ Filter", PK_FLOAT, -1, 1, 0, nullptr, 0, F(p.fx.filter = v)},
    {"drive", "Drive", PK_FLOAT, 0, 1, 0.15f, nullptr, 0, F(p.fx.drive = v)},
    {"echoMix", "Echo", PK_FLOAT, 0, 1, 0.35f, nullptr, 0, F(p.fx.echoMix = v)},
    {"revMix", "Reverb", PK_FLOAT, 0, 1, 0.25f, nullptr, 0, F(p.fx.revMix = v)},
    {"acidCut", "Acid Cutoff", PK_FLOAT, 0, 1, 0.35f, nullptr, 0, F(p.acid.cutoff = v)},
    {"acidRes", "Acid Reso", PK_FLOAT, 0, 1, 0.7f, nullptr, 0, F(p.acid.reso = v)},
    {"acidEnv", "Acid Env", PK_FLOAT, 0, 1, 0.6f, nullptr, 0, F(p.acid.envmod = v)},
    {"acidDec", "Acid Decay", PK_FLOAT, 0, 1, 0.35f, nullptr, 0, F(p.acid.decay = v)},
    // ---- bank 2: ACID
    {"acidAcc", "Accent", PK_FLOAT, 0, 1, 0.6f, nullptr, 0, F(p.acid.accent = v)},
    {"acidSlide", "Slide Time", PK_FLOAT, 0, 1, 0.5f, nullptr, 0, F(p.acid.slide = v)},
    {"acidWave", "Saw/Square", PK_FLOAT, 0, 1, 0, nullptr, 0, F(p.acid.wave = v)},
    {"acidDrive", "Acid Drive", PK_FLOAT, 0, 1, 0.3f, nullptr, 0, F(p.acid.drive = v)},
    {"acidTune", "Acid Tune", PK_FLOAT, -1, 1, 0, nullptr, 0, F(p.acid.tune = v)},
    {"acidVol", "Acid Level", PK_FLOAT, 0, 1, 0.8f, nullptr, 0, F(p.vol[TR_ACID] = v)},
    {"acidEcho", "Acid Echo", PK_FLOAT, 0, 1, 0.3f, nullptr, 0, F(p.echoSend[TR_ACID] = v)},
    {"acidRev", "Acid Reverb", PK_FLOAT, 0, 1, 0.15f, nullptr, 0, F(p.revSend[TR_ACID] = v)},
    // ---- bank 3: DRUMS
    {"kit", "Drum Kit", PK_INT, 0, NKITS - 1, 0, kKitNames, NKITS, F(p.drum.kit = (int)v)},
    {"drumPitch", "Drum Pitch", PK_FLOAT, -12, 12, 0, nullptr, 0, F(p.drum.pitch = v)},
    {"drumDecay", "Drum Decay", PK_FLOAT, 0, 1, 0.5f, nullptr, 0, F(p.drum.decay = v)},
    {"drumCrush", "Drum Crush", PK_FLOAT, 0, 1, 0, nullptr, 0, F(p.drum.crush = v)},
    {"kickTune", "Kick Tune", PK_FLOAT, 0, 1, 0.5f, nullptr, 0, F(p.drum.kickTune = v)},
    {"kickDecay", "Kick Decay", PK_FLOAT, 0, 1, 0.5f, nullptr, 0, F(p.drum.kickDecay = v)},
    {"snareTone", "Snare Tone", PK_FLOAT, 0, 1, 0.5f, nullptr, 0, F(p.drum.snareTone = v)},
    {"hatDecay", "Hat Decay", PK_FLOAT, 0, 1, 0.5f, nullptr, 0, F(p.drum.hatDecay = v)},
    // ---- bank 4: BASS
    {"bassMode", "Bass Sound", PK_INT, 0, NBASS - 1, 0, kBassNames, NBASS, F(p.bass.mode = (int)v)},
    {"bassCut", "Bass Tone", PK_FLOAT, 0, 1, 0.5f, nullptr, 0, F(p.bass.cutoff = v)},
    {"bassWah", "Bass Wah", PK_FLOAT, 0, 1, 0.5f, nullptr, 0, F(p.bass.wah = v)},
    {"bassGlide", "Bass Glide", PK_FLOAT, 0, 1, 0.2f, nullptr, 0, F(p.bass.glide = v)},
    {"bassDrive", "Bass Drive", PK_FLOAT, 0, 1, 0.2f, nullptr, 0, F(p.bass.drive = v)},
    {"bassDetune", "Bass Detune", PK_FLOAT, 0, 1, 0.3f, nullptr, 0, F(p.bass.detune = v)},
    {"bassTune", "Bass Tune", PK_FLOAT, -1, 1, 0, nullptr, 0, F(p.bass.tune = v)},
    {"bassVol", "Bass Level", PK_FLOAT, 0, 1, 0.8f, nullptr, 0, F(p.vol[TR_BASS] = v)},
    // ---- bank 5: LEAD
    {"leadMode", "Lead Sound", PK_INT, 0, NLEAD - 1, 0, kLeadNames, NLEAD, F(p.lead.mode = (int)v)},
    {"leadBright", "Brightness", PK_FLOAT, 0, 1, 0.5f, nullptr, 0, F(p.lead.bright = v)},
    {"leadVowel", "Vowel", PK_FLOAT, 0, 1, 0.3f, nullptr, 0, F(p.lead.vowel = v)},
    {"leadVib", "Vibrato", PK_FLOAT, 0, 1, 0.2f, nullptr, 0, F(p.lead.vibrato = v)},
    {"leadAtk", "Attack", PK_FLOAT, 0, 1, 0.05f, nullptr, 0, F(p.lead.attack = v)},
    {"leadRel", "Release", PK_FLOAT, 0, 1, 0.4f, nullptr, 0, F(p.lead.release = v)},
    {"leadDetune", "Detune", PK_FLOAT, 0, 1, 0.3f, nullptr, 0, F(p.lead.detune = v)},
    {"leadVol", "Lead Level", PK_FLOAT, 0, 1, 0.7f, nullptr, 0, F(p.vol[TR_LEAD] = v)},
    // ---- bank 6: FX
    {"echoHeads", "Echo Heads", PK_INT, 0, 7, 7, kHeadNames, 8, F(p.fx.heads = (int)v)},
    {"echoTime", "Echo Time", PK_FLOAT, 0.125f, 1.0f, 0.375f, nullptr, 0, F(p.fx.echoTime = v)},
    {"echoFb", "Echo Repeats", PK_FLOAT, 0, 0.95f, 0.45f, nullptr, 0, F(p.fx.echoFb = v)},
    {"echoWow", "Tape Wow", PK_FLOAT, 0, 1, 0.3f, nullptr, 0, F(p.fx.echoWow = v)},
    {"revSize", "Reverb Size", PK_FLOAT, 0, 1, 0.6f, nullptr, 0, F(p.fx.revSize = v)},
    {"revDamp", "Reverb Damp", PK_FLOAT, 0, 1, 0.5f, nullptr, 0, F(p.fx.revDamp = v)},
    {"crushBits", "Bit Crush", PK_FLOAT, 0, 1, 0, nullptr, 0, F(p.fx.crushBits = v)},
    {"crushRate", "Rate Crush", PK_FLOAT, 0, 1, 0, nullptr, 0, F(p.fx.crushRate = v)},
    // ---- bank 7: GROOVE (the ritual)
    {"styleA", "Style A", PK_INT, 0, NSTYLES - 1, 0, kStyleNames, NSTYLES, F(p.styleA = (int)v)},
    {"styleB", "Style B", PK_INT, 0, NSTYLES - 1, 2, kStyleNames, NSTYLES, F(p.styleB = (int)v)},
    {"fusion", "Fusion", PK_FLOAT, 0, 1, 0.3f, nullptr, 0, F(p.fusion = v)},
    {"chaos", "Chaos", PK_FLOAT, 0, 1, 0.4f, nullptr, 0, F(p.chaos = v)},
    {"mood", "Feel", PK_INT, 0, NMOODS - 1, 1, kMoodNames, NMOODS, F(p.mood = (int)v)},
    {"key", "Key", PK_INT, 0, 11, 9, kKeyNames, 12, F(p.key = (int)v)},
    {"swing", "Swing", PK_FLOAT, 0, 0.6f, 0, nullptr, 0, F(p.swing = v)},
    {"density", "Density", PK_FLOAT, 0, 1, 0.5f, nullptr, 0, F(p.density = v)},
    // ---- bank 8: MIX & PLAY
    {"drumVol", "Drums Level", PK_FLOAT, 0, 1, 0.85f, nullptr, 0, F(p.vol[TR_DRUMS] = v; p.drum.vol = 1.0f)},
    {"drumEcho", "Drums Echo", PK_FLOAT, 0, 1, 0.05f, nullptr, 0, F(p.echoSend[TR_DRUMS] = v)},
    {"leadEcho", "Lead Echo", PK_FLOAT, 0, 1, 0.35f, nullptr, 0, F(p.echoSend[TR_LEAD] = v)},
    {"leadRev", "Lead Reverb", PK_FLOAT, 0, 1, 0.4f, nullptr, 0, F(p.revSend[TR_LEAD] = v)},
    {"stutRate", "Stutter Rate", PK_INT, 0, 3, 1, kStutNames, 4, F(p.stutterRate = 1.0f / (float)(1 << (int)v))},
    {"playTarget", "Keys Play", PK_INT, 0, 3, 3, kTargetNames, 4, F(p.playTarget = (int)v)},
    {"scaleLock", "Scale Lock", PK_BOOL, 0, 1, 1, nullptr, 0, F(p.scaleLock = v > 0.5f)},
    {"master", "Master", PK_FLOAT, 0, 1, 0.8f, nullptr, 0, F(p.fx.master = v)},
};
#undef F
constexpr int NPARAMS = (int)(sizeof(kParams) / sizeof(kParams[0]));
static const char* const kBankNames[] = {"PERFORM", "ACID", "DRUMS", "BASS", "LEAD", "FX", "GROOVE", "MIX & PLAY"};
constexpr int NBANKS = NPARAMS / 8;

} // namespace dfd
