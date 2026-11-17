// Dark Funk Durcal Synt — factory presets. Each one is a style blend + a sound + a fixed seed, so it always summons
// the same groove (change CHAOS or press SUMMON for new ones). Levels are matched: no preset wins by being louder.
#pragma once
#include "ParamTable.h"
#include <initializer_list>
#include <utility>

namespace dfd {

struct Preset {
    const char* name; const char* category; int styleA, styleB; float fusion, chaos; uint32_t seed;
    std::initializer_list<std::pair<const char*, float>> set;
};

enum { S_ACID, S_DRILL, S_JUNGLE, S_DNB, S_BRKCOR, S_BIGBT, S_RAVE, S_BRAIN, S_ELECTR, S_AMBIEN, S_FUNK, S_WARPFK, S_SLAPDN, S_TOYBOX, S_SPECTR };

static const Preset kPresets[] = {
    // ---------------------------------------------------------------- ACID
    {"Ladder Squelch 303", "ACID", S_ACID, S_ACID, 0.0f, 0.3f, 101, {{"acidCut", 0.28f}, {"acidRes", 0.85f}, {"acidEnv", 0.75f}, {"acidDec", 0.3f}, {"acidDrive", 0.4f}}},
    {"Chicago Night Acid", "ACID", S_ACID, S_ELECTR, 0.2f, 0.4f, 102, {{"acidCut", 0.22f}, {"acidRes", 0.9f}, {"acidEnv", 0.8f}, {"echoHeads", 5}, {"echoMix", 0.4f}}},
    {"Square Rubber Acid", "ACID", S_ACID, S_BRAIN, 0.3f, 0.5f, 103, {{"acidWave", 1.0f}, {"acidCut", 0.32f}, {"acidRes", 0.8f}, {"acidSlide", 0.7f}}},
    {"Distorted Mind Acid", "ACID", S_ACID, S_BRKCOR, 0.25f, 0.6f, 104, {{"acidDrive", 0.85f}, {"acidRes", 0.88f}, {"drive", 0.4f}}},
    {"Hypnotic 1993", "ACID", S_ACID, S_SPECTR, 0.3f, 0.2f, 105, {{"mood", M_HYPNO}, {"acidCut", 0.3f}, {"echoWow", 0.5f}}},
    {"Analog Tape Acid", "ACID", S_ACID, S_AMBIEN, 0.25f, 0.3f, 106, {{"echoHeads", 7}, {"echoWow", 0.7f}, {"echoFb", 0.6f}, {"acidCut", 0.25f}}},
    // ---------------------------------------------------------------- WARPED FUNK (vocal formants, rubber wah bass, swung half-time breaks, EP chords)
    {"Rubber Lips Funk", "WARPED FUNK", S_WARPFK, S_FUNK, 0.2f, 0.4f, 201, {{"leadMode", LEAD_VOX}, {"leadVowel", 0.35f}, {"bassMode", BASS_WAH}, {"bassWah", 0.75f}, {"swing", 0.22f}}},
    {"Window Glass Vox", "WARPED FUNK", S_WARPFK, S_BRAIN, 0.3f, 0.5f, 202, {{"leadMode", LEAD_VOX}, {"leadVowel", 0.6f}, {"leadVib", 0.45f}, {"crushRate", 0.15f}}},
    {"Velvet EP Warp", "WARPED FUNK", S_WARPFK, S_FUNK, 0.5f, 0.3f, 203, {{"leadMode", LEAD_EP}, {"bassMode", BASS_WAH}, {"mood", M_SOUL}}},
    {"Talkbox Tantrum", "WARPED FUNK", S_WARPFK, S_BRKCOR, 0.25f, 0.7f, 204, {{"leadMode", LEAD_VOX}, {"leadVowel", 0.1f}, {"drumCrush", 0.3f}}},
    {"Pitch-Bent Lover", "WARPED FUNK", S_WARPFK, S_SPECTR, 0.3f, 0.4f, 205, {{"leadMode", LEAD_VOX}, {"leadVib", 0.6f}, {"echoHeads", 3}}},
    {"Slow Wah Strut", "WARPED FUNK", S_WARPFK, S_FUNK, 0.6f, 0.2f, 206, {{"bassMode", BASS_WAH}, {"bassWah", 0.9f}, {"bassGlide", 0.45f}}},
    // ---------------------------------------------------------------- BRAINDANCE / IDM
    {"Brain Square Lead", "BRAINDANCE", S_BRAIN, S_BRAIN, 0.0f, 0.5f, 301, {{"leadMode", LEAD_BRAIN}, {"crushBits", 0.2f}}},
    {"Prepared Toy Piano", "BRAINDANCE", S_TOYBOX, S_BRAIN, 0.3f, 0.4f, 302, {{"leadMode", LEAD_TOY}, {"kit", KIT_TOY}}},
    {"Music Box Melancholy", "BRAINDANCE", S_TOYBOX, S_AMBIEN, 0.4f, 0.3f, 303, {{"leadMode", LEAD_TOY}, {"mood", M_MELAN}, {"revMix", 0.4f}}},
    {"Ratchet Brain Drill", "BRAINDANCE", S_BRAIN, S_DRILL, 0.5f, 0.8f, 304, {{"kit", KIT_WARP}, {"drumCrush", 0.25f}}},
    {"Glass Bell Garden", "BRAINDANCE", S_BRAIN, S_AMBIEN, 0.5f, 0.3f, 305, {{"leadMode", LEAD_BELL}, {"echoHeads", 6}}},
    {"Crunchy Lullaby", "BRAINDANCE", S_TOYBOX, S_BRKCOR, 0.3f, 0.6f, 306, {{"crushRate", 0.3f}, {"leadMode", LEAD_TOY}}},
    // ---------------------------------------------------------------- AMBIENT
    {"Selected Haze", "AMBIENT", S_AMBIEN, S_AMBIEN, 0.0f, 0.2f, 401, {{"leadMode", LEAD_PAD}, {"revMix", 0.5f}, {"echoWow", 0.6f}}},
    {"Tape Hiss Sunrise", "AMBIENT", S_AMBIEN, S_TOYBOX, 0.3f, 0.2f, 402, {{"leadMode", LEAD_PAD}, {"echoHeads", 7}, {"echoFb", 0.7f}}},
    {"Frozen Lake Pads", "AMBIENT", S_AMBIEN, S_SPECTR, 0.3f, 0.1f, 403, {{"leadMode", LEAD_PAD}, {"mood", M_DREAMY}, {"revSize", 0.9f}}},
    {"Bell Fog", "AMBIENT", S_AMBIEN, S_BRAIN, 0.4f, 0.3f, 404, {{"leadMode", LEAD_BELL}, {"revMix", 0.45f}}},
    // ---------------------------------------------------------------- DRILL N BASS / JAZZ BREAKS
    {"Slap Bass Frenzy", "DRILL N BASS", S_SLAPDN, S_DRILL, 0.3f, 0.6f, 501, {{"bassMode", BASS_SLAP}, {"leadMode", LEAD_EP}}},
    {"Fretless Breakdown", "DRILL N BASS", S_SLAPDN, S_FUNK, 0.3f, 0.5f, 502, {{"bassMode", BASS_SLAP}, {"bassGlide", 0.4f}, {"mood", M_FUNKY}}},
    {"Machine Gun Snares", "DRILL N BASS", S_DRILL, S_BRKCOR, 0.4f, 0.9f, 503, {{"kit", KIT_BREAK}}},
    {"Jazz Chord Drill", "DRILL N BASS", S_DRILL, S_SLAPDN, 0.5f, 0.6f, 504, {{"leadMode", LEAD_EP}, {"mood", M_SOUL}}},
    {"Hyper Funk Kit", "DRILL N BASS", S_SLAPDN, S_SLAPDN, 0.0f, 0.7f, 505, {{"swing", 0.08f}}},
    // ---------------------------------------------------------------- JUNGLE / DNB
    {"Dark Amen Rollers", "JUNGLE / DNB", S_JUNGLE, S_DNB, 0.3f, 0.5f, 601, {{"bassMode", BASS_SUB}}},
    {"Reese Pressure", "JUNGLE / DNB", S_DNB, S_DNB, 0.0f, 0.4f, 602, {{"bassMode", BASS_REESE}, {"bassDetune", 0.6f}}},
    {"Ragga Vox Jungle", "JUNGLE / DNB", S_JUNGLE, S_WARPFK, 0.3f, 0.5f, 603, {{"leadMode", LEAD_VOX}}},
    {"Atmos Roll", "JUNGLE / DNB", S_DNB, S_AMBIEN, 0.35f, 0.3f, 604, {{"leadMode", LEAD_PAD}}},
    // ---------------------------------------------------------------- RAVE / BIG BEAT
    {"Punk Rave Breaks", "RAVE / BIG BEAT", S_BIGBT, S_RAVE, 0.3f, 0.6f, 701, {{"drive", 0.5f}, {"acidDrive", 0.7f}}},
    {"Hoover Stab Rave", "RAVE / BIG BEAT", S_RAVE, S_RAVE, 0.0f, 0.5f, 702, {{"leadMode", LEAD_STAB}}},
    {"Smack Big Beat", "RAVE / BIG BEAT", S_BIGBT, S_ELECTR, 0.3f, 0.5f, 703, {{"bassMode", BASS_WAH}, {"drive", 0.45f}}},
    {"Warehouse 1992", "RAVE / BIG BEAT", S_RAVE, S_JUNGLE, 0.4f, 0.6f, 704, {{"leadMode", LEAD_STAB}, {"bassMode", BASS_REESE}}},
    // ---------------------------------------------------------------- ELECTRO / BREAKCORE
    {"Detroit 808 Ghost", "ELECTRO", S_ELECTR, S_ELECTR, 0.0f, 0.3f, 801, {{"kit", KIT_808}}},
    {"Laser Grid", "ELECTRO", S_ELECTR, S_SPECTR, 0.4f, 0.4f, 802, {{"leadMode", LEAD_BRAIN}, {"echoHeads", 1}}},
    {"Shredded Amen Core", "BREAKCORE", S_BRKCOR, S_BRKCOR, 0.0f, 0.9f, 901, {{"crushBits", 0.3f}}},
    {"Cartoon Violence", "BREAKCORE", S_BRKCOR, S_TOYBOX, 0.4f, 0.9f, 902, {{"leadMode", LEAD_TOY}}},
    // ---------------------------------------------------------------- FUNK
    {"Broken Funk Pocket", "FUNK", S_FUNK, S_FUNK, 0.0f, 0.3f, 1001, {{"leadMode", LEAD_EP}, {"bassMode", BASS_SLAP}}},
    {"Clavinet Fever", "FUNK", S_FUNK, S_WARPFK, 0.3f, 0.4f, 1002, {{"leadMode", LEAD_BRAIN}, {"leadBright", 0.8f}, {"mood", M_FUNKY}}},
    {"Soul 9ths", "FUNK", S_FUNK, S_SLAPDN, 0.3f, 0.3f, 1003, {{"mood", M_SOUL}, {"leadMode", LEAD_EP}}},
    {"Spectral Step", "FUNK", S_SPECTR, S_WARPFK, 0.3f, 0.4f, 1004, {}},
};
constexpr int NPRESETS = (int)(sizeof(kPresets) / sizeof(kPresets[0]));

} // namespace dfd
