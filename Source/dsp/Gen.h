// Dark Funk Durcal Synt — patterns, scales, the 15 styles and every generator (summon, dice, melody, next phrase).
#pragma once
#include "Voices.h"

namespace dfd {

enum { TR_DRUMS, TR_ACID, TR_BASS, TR_LEAD, NTRACKS };
static const char* const kTrackNames[NTRACKS] = {"DRUMS", "ACID", "BASS", "LEAD"};
constexpr int MAXSTEPS = 64;

struct Step {
    uint8_t n = 0; uint8_t notes[4] = {0, 0, 0, 0};
    uint8_t gate = 0;          // 0 rest, 1 note, 2 tie (held from the previous step)
    bool acc = false, slide = false;
    uint8_t vel = 100, prob = 100, ratchet = 1;
    uint16_t hits = 0, accHits = 0;   // drums: one bit per lane
};
struct Track { int len = 16; Step s[MAXSTEPS]; void clear() { for (auto& x : s) x = Step(); } };
struct Pattern {
    Track t[NTRACKS]; int key = 9; int mood = 1; float bpm = 128; float swing = 0;
    void clear() { for (auto& x : t) { x.clear(); x.len = 16; } }
};

// ---------------------------------------------------------------- moods = scales a beginner can't get wrong
enum Mood { M_DARK, M_ACID, M_FUNKY, M_MELAN, M_DREAMY, M_ALIEN, M_RAVE, M_HYPNO, M_SOUL, NMOODS };
static const char* const kMoodNames[NMOODS] = {"DARK", "ACID", "FUNKY", "MELANCHOLY", "DREAMY", "ALIEN", "RAVE", "HYPNO", "SOUL"};
static const int kScales[NMOODS][8] = {
    {0, 1, 3, 5, 7, 8, 10, -1},   // phrygian
    {0, 3, 5, 7, 10, -1},          // minor pentatonic
    {0, 2, 3, 5, 7, 9, 10, -1},   // dorian
    {0, 2, 3, 5, 7, 8, 10, -1},   // aeolian
    {0, 2, 4, 6, 7, 9, 11, -1},   // lydian
    {0, 2, 4, 6, 8, 10, -1},      // whole tone
    {0, 2, 3, 5, 7, 8, 10, -1},   // minor
    {0, 3, 5, 7, 10, -1},          // pentatonic (hypnotic)
    {0, 2, 3, 5, 7, 9, 10, -1},   // dorian, played with 7ths and 9ths
};
static const char* const kKeyNames[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
inline int scaleLen(int mood) { int n = 0; while (n < 8 && kScales[mood][n] >= 0) ++n; return n; }
// degree (any integer) -> MIDI note around a base octave
inline int degNote(int mood, int key, int deg, int baseOct) {
    int L = scaleLen(mood); int o = (deg >= 0 ? deg / L : -((-deg + L - 1) / L)); int d = deg - o * L;
    return 12 * (baseOct + o) + key + kScales[mood][d];
}
inline int snapToScale(int note, int mood, int key) {
    int best = note, bd = 99;
    for (int c = note - 6; c <= note + 6; ++c) { int pc = ((c - key) % 12 + 12) % 12; for (int i = 0; i < scaleLen(mood); ++i) if (kScales[mood][i] == pc && std::abs(c - note) < bd) { bd = std::abs(c - note); best = c; } }
    return best;
}

// ---------------------------------------------------------------- the styles
enum DrumTpl { DT_FOUR, DT_ELECTRO, DT_AMEN, DT_TWOSTEP, DT_BRAIN, DT_HALFFUNK, DT_BOOMBAP, DT_BIGBEAT, DT_AMBIENT, DT_SLAP };
enum BassType { BT_ROOT, BT_OFFBEAT, BT_FUNK, BT_LONG, BT_BUSY };
enum LeadType { LT_MELODY, LT_CHORDS, LT_ARP, LT_STABS };

struct Style {
    const char* name; const char* full; int bpmLo, bpmHi; float swing; int mood; int kit; int tpl;
    float chop, ghost, ratchet, hat; float acidDens, acidSlide, acidAcc; int bassMode, bassType; int leadMode, leadType; float leadDens;
    float cut, res, env, dec, drive;   // the acid's sound
};
static const Style kStyles[] = {
    // name     full               bpm       swing mood      kit        tpl           chop ghost ratch hat   acidD slide acc   bass        btype      lead        ltype      ldens  cut   res   env   dec   drive
    {"ACID",   "ACID",            124, 134, 0.00f, M_ACID,  KIT_909,   DT_FOUR,      0.0f, 0.1f, 0.05f, 0.6f, 0.80f, 0.30f, 0.35f, BASS_SUB,   BT_ROOT,   LEAD_BELL,  LT_MELODY, 0.25f, 0.30f, 0.80f, 0.70f, 0.35f, 0.35f},
    {"DRILL",  "DRILL N BASS",    165, 180, 0.05f, M_MELAN, KIT_BREAK, DT_AMEN,      0.6f, 0.5f, 0.45f, 0.8f, 0.55f, 0.25f, 0.30f, BASS_SLAP,  BT_BUSY,   LEAD_TOY,   LT_MELODY, 0.45f, 0.40f, 0.70f, 0.60f, 0.30f, 0.20f},
    {"JUNGLE", "JUNGLE",          160, 172, 0.08f, M_DARK,  KIT_BREAK, DT_AMEN,      0.4f, 0.4f, 0.20f, 0.6f, 0.40f, 0.40f, 0.30f, BASS_SUB,   BT_LONG,   LEAD_VOX,   LT_CHORDS, 0.30f, 0.35f, 0.75f, 0.60f, 0.40f, 0.30f},
    {"DNB",    "ROLLING DNB",     170, 176, 0.00f, M_DARK,  KIT_BREAK, DT_TWOSTEP,   0.2f, 0.3f, 0.10f, 0.7f, 0.45f, 0.30f, 0.30f, BASS_REESE, BT_LONG,   LEAD_PAD,   LT_CHORDS, 0.25f, 0.30f, 0.75f, 0.65f, 0.35f, 0.30f},
    {"BRKCOR", "BREAKCORE",       180, 200, 0.00f, M_ALIEN, KIT_WARP,  DT_AMEN,      0.9f, 0.3f, 0.70f, 0.9f, 0.60f, 0.20f, 0.50f, BASS_FM,    BT_BUSY,   LEAD_BRAIN, LT_MELODY, 0.40f, 0.45f, 0.85f, 0.80f, 0.25f, 0.60f},
    {"BIGBT",  "BIG BEAT",        128, 138, 0.06f, M_RAVE,  KIT_909,   DT_BIGBEAT,   0.2f, 0.2f, 0.10f, 0.5f, 0.55f, 0.30f, 0.50f, BASS_WAH,   BT_OFFBEAT,LEAD_STAB,  LT_STABS,  0.30f, 0.35f, 0.85f, 0.75f, 0.40f, 0.70f},
    {"RAVE",   "RAVE HARDCORE",   138, 150, 0.00f, M_RAVE,  KIT_909,   DT_AMEN,      0.3f, 0.2f, 0.15f, 0.7f, 0.50f, 0.20f, 0.50f, BASS_REESE, BT_OFFBEAT,LEAD_STAB,  LT_STABS,  0.35f, 0.40f, 0.80f, 0.80f, 0.30f, 0.50f},
    {"BRAIN",  "BRAINDANCE",      110, 140, 0.10f, M_DREAMY,KIT_WARP,  DT_BRAIN,     0.5f, 0.4f, 0.50f, 0.7f, 0.60f, 0.35f, 0.40f, BASS_FM,    BT_BUSY,   LEAD_BRAIN, LT_MELODY, 0.50f, 0.40f, 0.75f, 0.70f, 0.30f, 0.30f},
    {"ELECTR", "DARK ELECTRO",    126, 136, 0.00f, M_DARK,  KIT_808,   DT_ELECTRO,   0.0f, 0.1f, 0.10f, 0.8f, 0.60f, 0.25f, 0.40f, BASS_FM,    BT_OFFBEAT,LEAD_BRAIN, LT_MELODY, 0.30f, 0.30f, 0.85f, 0.75f, 0.30f, 0.40f},
    {"AMBIEN", "AMBIENT IDM",      86, 104, 0.12f, M_DREAMY,KIT_TOY,   DT_AMBIENT,   0.1f, 0.3f, 0.30f, 0.3f, 0.25f, 0.50f, 0.20f, BASS_SUB,   BT_LONG,   LEAD_PAD,   LT_CHORDS, 0.20f, 0.25f, 0.60f, 0.40f, 0.50f, 0.10f},
    {"FUNK",   "BROKEN FUNK",      98, 116, 0.18f, M_FUNKY, KIT_BREAK, DT_BOOMBAP,   0.1f, 0.6f, 0.10f, 0.7f, 0.40f, 0.40f, 0.30f, BASS_SLAP,  BT_FUNK,   LEAD_EP,    LT_CHORDS, 0.35f, 0.35f, 0.70f, 0.55f, 0.40f, 0.20f},
    // new in v2
    {"WARPFK", "WARPED FUNK",     100, 108, 0.22f, M_SOUL,  KIT_WARP,  DT_HALFFUNK,  0.3f, 0.6f, 0.35f, 0.7f, 0.35f, 0.60f, 0.40f, BASS_WAH,   BT_FUNK,   LEAD_VOX,   LT_MELODY, 0.40f, 0.30f, 0.70f, 0.60f, 0.45f, 0.30f},
    {"SLAPDN", "SLAP DNB",        168, 178, 0.04f, M_FUNKY, KIT_BREAK, DT_SLAP,      0.5f, 0.6f, 0.40f, 0.8f, 0.40f, 0.30f, 0.30f, BASS_SLAP,  BT_BUSY,   LEAD_EP,    LT_CHORDS, 0.35f, 0.40f, 0.70f, 0.60f, 0.30f, 0.20f},
    {"TOYBOX", "MUSIC BOX IDM",   112, 132, 0.08f, M_MELAN, KIT_TOY,   DT_BRAIN,     0.4f, 0.4f, 0.55f, 0.6f, 0.30f, 0.40f, 0.30f, BASS_SUB,   BT_ROOT,   LEAD_TOY,   LT_ARP,    0.60f, 0.30f, 0.65f, 0.55f, 0.40f, 0.10f},
    {"SPECTR", "SPECTRAL STEP",   134, 144, 0.10f, M_HYPNO, KIT_808,   DT_TWOSTEP,   0.3f, 0.3f, 0.25f, 0.6f, 0.50f, 0.45f, 0.45f, BASS_FM,    BT_OFFBEAT,LEAD_VOX,   LT_STABS,  0.30f, 0.35f, 0.80f, 0.70f, 0.35f, 0.35f},
};
constexpr int NSTYLES = (int)(sizeof(kStyles) / sizeof(kStyles[0]));

// drum templates: 32 steps (2 bars), lanes K S C CH OH; 'x' hit, 'X' accent, 'g' ghost
struct Tpl { const char* lane[5]; };
static const Tpl kTpl[] = {
    /* FOUR */    {{"x...x...x...x...x...x...x...x...", "....x.......x.......x.......x...", "................................", "..x...x...x...x...x...x...x...x.", "..x...x...x...x...x...x...x...x."}},
    /* ELECTRO */ {{"x......x..x.....x......x..x..x..", "....x.......x.......x.......x...", "............x...............x...", "x.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xx", "................................"}},
    /* AMEN */    {{"x.x.......xx....x.x.......x.....", "....X..x.x..X..x....X..x.x....X.", "................................", "x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.", "..............x...............x."}},
    /* TWOSTEP */ {{"x.........x.....x.........x..x..", "....X.......X.......X..g....X...", "................................", "x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.xx", "......x...............x........."}},
    /* BRAIN */   {{"x..x..x...x.x...x.....x..x..x...", "....x..g.x..x.g.....x.g...x.x..g", "...........x...............x....", "xx.xx.x.xx.xx.x.xx.xx.xx.x.xxx.x", "................................"}},
    /* HALFFUNK */{{"x......x..x.....x.....x...x.....", "........X..g...g........X..g.g..", "........x...............x.......", "x.xgx.xgx.xgx.xgx.xgx.xgx.xgx.xg", "..............x...............x."}},
    /* BOOMBAP */ {{"x......x..x.....x.....x.x.......", "....X..g....X..g....X..g....X.g.", "................................", "x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.", "......x...............x........."}},
    /* BIGBEAT */ {{"x.....x...x.....x.....x...x..x..", "....X.......X.......X.......X...", "....x.......x.......x.......x...", "x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.", "..x...x...x...x...x...x...x...x."}},
    /* AMBIENT */ {{"x...........x...........x.......", "........x...............x.......", "................................", "..x...x...x...x...x...x...x...x.", "................................"}},
    /* SLAP */    {{"x.x...x...x..x..x.x.....x.x..x..", "....X..g.g..X..g....X.gg.g..X..g", "................................", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", "......x.......x.......x........."}},
};

struct Gen {
    Rng rng;
    // ------------------------------------------------------------ helpers
    static void setNote(Step& s, int note, int vel = 100, bool acc = false, bool slide = false) {
        s.gate = 1; s.n = 1; s.notes[0] = (uint8_t)clampf((float)note, 12, 120); s.vel = (uint8_t)vel; s.acc = acc; s.slide = slide;
    }
    static void addNote(Step& s, int note) { if (s.n < 4) s.notes[s.n++] = (uint8_t)clampf((float)note, 12, 120); s.gate = 1; }

    // ------------------------------------------------------------ DRUMS
    void drums(Track& t, const Style& a, const Style& b, float fusion, float chaos) {
        t.clear(); t.len = 32;
        const Style& base = rng.uni() < fusion ? b : a;
        const Tpl& T = kTpl[base.tpl];
        float ghost = lerp(a.ghost, b.ghost, fusion), rat = lerp(a.ratchet, b.ratchet, fusion) * (0.4f + chaos), chop = lerp(a.chop, b.chop, fusion) * (0.3f + chaos);
        float hat = lerp(a.hat, b.hat, fusion);
        static const int laneOf[5] = {KICK, SNARE, CLAP, CHAT, OHAT};
        for (int L = 0; L < 5; ++L)
            for (int i = 0; i < 32; ++i) {
                char c = T.lane[L][i];
                if (c == '.') continue;
                if (c == 'g' && !rng.chance(0.4f + ghost * 0.6f)) continue;
                if (L == 3 && !rng.chance(0.35f + hat * 0.65f)) continue;
                Step& s = t.s[i]; s.gate = 1; s.hits |= (uint16_t)(1u << laneOf[L]);
                if (c == 'X') s.accHits |= (uint16_t)(1u << laneOf[L]);
                s.vel = c == 'g' ? 45 : 100;
            }
        // ghost snares between, and toms/rims/bells for flavour
        for (int i = 0; i < 32; ++i) {
            if (rng.chance(ghost * 0.12f)) { t.s[i].gate = 1; t.s[i].hits |= 1u << RIM; t.s[i].vel = std::min<int>(t.s[i].vel, 60); }
            if (rng.chance(ghost * 0.05f)) { t.s[i].gate = 1; t.s[i].hits |= 1u << TOM; }
            if (base.kit == KIT_TOY && rng.chance(0.06f)) { t.s[i].gate = 1; t.s[i].hits |= 1u << BELL; }
        }
        if (chop > 0.05f) breakChop(t, chop);
        for (int i = 0; i < 32; ++i) if (t.s[i].gate && rng.chance(rat * 0.18f)) t.s[i].ratchet = (uint8_t)(2 + rng.range(3));
        // last beat fill
        if (rng.chance(0.3f + chaos * 0.5f)) for (int i = 28; i < 32; ++i) { t.s[i].gate = 1; t.s[i].hits |= 1u << SNARE; t.s[i].vel = (uint8_t)(60 + (i - 28) * 15); if (rng.chance(chaos)) t.s[i].ratchet = 2; }
    }
    // the cut-up: copy random 2- or 4-step slices over other places, like re-sequenced breaks
    void breakChop(Track& t, float amount) {
        Step copy[MAXSTEPS]; std::copy(t.s, t.s + MAXSTEPS, copy);
        int cuts = 1 + (int)(amount * 8.0f);
        for (int c = 0; c < cuts; ++c) {
            int len = rng.chance(0.5f) ? 2 : 4, dst = rng.range(t.len / len) * len, src = rng.range(t.len / len) * len;
            bool reverse = rng.chance(amount * 0.3f);
            for (int k = 0; k < len; ++k) t.s[(dst + k) % t.len] = copy[(src + (reverse ? len - 1 - k : k)) % t.len];
        }
    }
    void dice_rolls(Track& t) { for (int i = 0; i < t.len; ++i) if (t.s[i].gate && rng.chance(0.18f)) t.s[i].ratchet = (uint8_t)(2 + rng.range(3)); }
    void dice_ghosts(Track& t) { for (int i = 0; i < t.len; ++i) if (rng.chance(0.15f)) { t.s[i].gate = 1; t.s[i].hits |= 1u << (rng.chance(0.6f) ? SNARE : RIM); if (!(t.s[i].hits & (1u << KICK))) t.s[i].vel = 40; } }
    void dice_hats(Track& t) { for (int i = 0; i < t.len; ++i) { t.s[i].hits &= (uint16_t)~(1u << CHAT); if (rng.chance(i % 2 == 0 ? 0.85f : 0.4f)) { t.s[i].gate = 1; t.s[i].hits |= 1u << CHAT; } } }

    // ------------------------------------------------------------ ACID
    void acid(Track& t, int mood, int key, float dens, float slide, float acc, float chaos) {
        t.clear(); t.len = 16;
        int L = scaleLen(mood);
        int motif[4]; for (int& m : motif) m = rng.chance(0.45f) ? 0 : rng.range(L + 2) - 1;
        for (int i = 0; i < 16; ++i) {
            float p = dens * (i % 4 == 0 ? 1.15f : i % 2 == 0 ? 0.95f : 0.8f);
            if (!rng.chance(p)) { if (i > 0 && t.s[i - 1].gate && rng.chance(0.25f)) t.s[i].gate = 2; continue; }
            int deg = rng.chance(0.55f) ? motif[(i / 2) % 4] : rng.range(L + 3) - 1;
            int oct = rng.chance(0.18f + chaos * 0.2f) ? 1 : rng.chance(0.08f) ? -1 : 0;
            setNote(t.s[i], degNote(mood, key, deg, 2) + oct * 12, 100, rng.chance(acc), rng.chance(slide));
        }
        if (!t.s[0].gate) setNote(t.s[0], degNote(mood, key, 0, 2));
    }
    void dice_acidNotes(Track& t, int mood, int key) { int L = scaleLen(mood); for (int i = 0; i < t.len; ++i) if (t.s[i].gate == 1 && rng.chance(0.4f)) t.s[i].notes[0] = (uint8_t)degNote(mood, key, rng.range(L + 2) - 1, 2); }
    void dice_accents(Track& t) { for (int i = 0; i < t.len; ++i) if (t.s[i].gate == 1) t.s[i].acc = rng.chance(0.3f); }
    void dice_slides(Track& t) { for (int i = 0; i < t.len; ++i) if (t.s[i].gate == 1) t.s[i].slide = rng.chance(0.3f); }
    void dice_octaves(Track& t) { for (int i = 0; i < t.len; ++i) if (t.s[i].gate == 1 && rng.chance(0.25f)) { int n = t.s[i].notes[0] + (rng.chance(0.7f) ? 12 : -12); t.s[i].notes[0] = (uint8_t)clampf((float)n, 24, 84); } }

    // ------------------------------------------------------------ BASS
    void bass(Track& t, int type, int mood, int key, const Track& drums) {
        t.clear(); t.len = 32; int L = scaleLen(mood);
        int prog[4] = {0, 0, rng.chance(0.5f) ? 5 % L : 3 % L, rng.chance(0.5f) ? 4 % L : L - 1};
        for (int i = 0; i < 32; ++i) {
            int bar = i / 8 % 4, root = degNote(mood, key, prog[bar], 1);
            bool kick = drums.s[i % drums.len].hits & (1u << KICK);
            switch (type) {
                case BT_ROOT: if (i % 4 == 0 || (kick && rng.chance(0.5f))) setNote(t.s[i], root); else if (t.s[i - 1].gate && rng.chance(0.6f)) t.s[i].gate = 2; break;
                case BT_OFFBEAT: if (i % 4 == 2) setNote(t.s[i], root + (rng.chance(0.2f) ? 12 : 0)); break;
                case BT_FUNK: {   // syncopated, octave pops, chromatic approach, legato slides
                    static const char* R = "x..x..x.x.x..x.xx..x..x.x..x.x.x";
                    if (R[i] == 'x') { int n = root; if (rng.chance(0.25f)) n += 12; if (rng.chance(0.15f)) n = degNote(mood, key, prog[bar] + (rng.chance(0.5f) ? 2 : 4), 1);
                        setNote(t.s[i], n, rng.chance(0.3f) ? 120 : 95, rng.chance(0.25f), rng.chance(0.35f));
                        if (i + 1 < 32 && rng.chance(0.4f)) t.s[i + 1].gate = 2; }
                    if (i % 8 == 7 && rng.chance(0.5f)) setNote(t.s[i], root - 1 + (rng.chance(0.5f) ? 2 : 0), 80, false, true);   // approach note
                    break; }
                case BT_LONG: if (i % 8 == 0) setNote(t.s[i], root); else t.s[i].gate = (i % 8 == 7 && rng.chance(0.5f)) ? 0 : 2; break;
                case BT_BUSY: if (rng.chance(0.55f) || i % 4 == 0) { int n = degNote(mood, key, prog[bar] + (rng.chance(0.6f) ? 0 : rng.range(L)), 1) + (rng.chance(0.2f) ? 12 : 0); setNote(t.s[i], n, 100, rng.chance(0.2f), rng.chance(0.25f)); } break;
            }
        }
        (void)L;
    }

    // ------------------------------------------------------------ LEAD: melodies a beginner can't get wrong
    enum Shape { SH_WAVE, SH_RISE, SH_FALL, SH_QA, SH_ARP, SH_HOOK, SH_FEW, SH_BUSY, NSHAPES };
    void melody(Track& t, int mood, int key, int shape, float dens) {
        t.clear(); t.len = 32; int L = scaleLen(mood);
        float d = shape == SH_FEW ? 0.2f : shape == SH_BUSY ? 0.75f : dens;
        int deg = 0; int hook[8]; for (int& h : hook) h = rng.range(L + 1);
        for (int i = 0; i < 32; ++i) {
            float ph = i / 32.0f;
            bool strong = i % 4 == 0;
            if (!rng.chance(strong ? d + 0.25f : d * 0.7f)) { if (i && t.s[i - 1].gate && rng.chance(0.5f)) t.s[i].gate = 2; continue; }
            switch (shape) {
                case SH_WAVE: deg = (int)std::lround(std::sin(ph * kTwoPi * 2) * L * 0.7f) + 2; break;
                case SH_RISE: deg = (int)(ph * L * 1.5f) + rng.range(2); break;
                case SH_FALL: deg = (int)((1 - ph) * L * 1.5f) + rng.range(2) - 1; break;
                case SH_QA: deg = (i < 16 ? (i / 2) % 4 + 2 : 6 - (i - 16) / 3) + rng.range(2); break;
                case SH_ARP: { static const int A[4] = {0, 2, 4, 7}; deg = A[i % 4] + (i / 8 % 2) * 2; break; }
                case SH_HOOK: deg = hook[i % 8] + (i >= 16 && i % 8 >= 6 ? 1 : 0); break;
                default: deg += rng.range(5) - 2; deg = std::max(-2, std::min(deg, L + 3)); break;
            }
            setNote(t.s[i], degNote(mood, key, deg, 4), strong ? 105 : 85);
        }
        // land the phrase: last note on the root or the fifth
        for (int i = 31; i >= 24; --i) if (t.s[i].gate == 1) { t.s[i].notes[0] = (uint8_t)degNote(mood, key, rng.chance(0.6f) ? L : (L >= 5 ? 4 : 2), 4); break; }
    }
    void chords(Track& t, int mood, int key, bool stabs, bool rich) {
        t.clear(); t.len = 32; int L = scaleLen(mood);
        static const int P[4][4] = {{0, 5, 3, 4}, {0, 3, 5, 4}, {0, 6, 5, 4}, {0, 2, 3, 0}};
        const int* prog = P[rng.range(4)];
        for (int bar = 0; bar < 4; ++bar) {
            int root = prog[bar] % std::max(L, 1);
            int at[4] = {0, 3, 6, 11}; int hits = stabs ? 2 + rng.range(3) : 1;
            for (int h = 0; h < hits; ++h) {
                int i = bar * 8 + (stabs ? at[h] % 8 : 0); if (i >= 32) break;
                Step& s = t.s[i]; s.gate = 1; s.n = 0;
                addNote(s, degNote(mood, key, root, 3)); addNote(s, degNote(mood, key, root + 2, 3)); addNote(s, degNote(mood, key, root + 4, 3));
                if (rich) addNote(s, degNote(mood, key, root + 6, 3));  // 7th (dorian soul voicings)
                if (!stabs) for (int k = 1; k < 8 && i + k < 32; ++k) t.s[i + k].gate = 2;
            }
        }
    }
    void arp(Track& t, int mood, int key) { melody(t, mood, key, SH_ARP, 0.9f); }

    // NEXT PHRASE: three continuations of the current lead that always fit
    // 0 ANSWER (same rhythm, contour mirrored, lands on the root)  1 VARIATION (same rhythm, a few notes moved)
    // 2 TENSION (busier toward the end, climbing to the fifth)
    void nextPhrase(const Track& cur, Track out[3], int mood, int key) {
        int L = scaleLen(mood);
        int lo = 127, hi = 0; for (int i = 0; i < cur.len; ++i) if (cur.s[i].gate == 1) { lo = std::min<int>(lo, cur.s[i].notes[0]); hi = std::max<int>(hi, cur.s[i].notes[0]); }
        if (hi < lo) { lo = hi = degNote(mood, key, 0, 4); }
        int mid = (lo + hi) / 2;
        for (int v = 0; v < 3; ++v) { out[v] = cur; }
        for (int i = 0; i < cur.len; ++i) {
            const Step& s = cur.s[i]; if (s.gate != 1) continue;
            int n = s.notes[0];
            out[0].s[i].notes[0] = (uint8_t)snapToScale(2 * mid - n, mood, key);
            if (rng.chance(0.35f)) out[1].s[i].notes[0] = (uint8_t)snapToScale(n + (rng.chance(0.5f) ? 2 : -2), mood, key);
            if (i >= cur.len / 2) out[2].s[i].notes[0] = (uint8_t)snapToScale(n + (i - cur.len / 2) / 3, mood, key);
        }
        for (int i = cur.len * 3 / 4; i < cur.len; ++i) if (out[2].s[i].gate == 0 && rng.chance(0.5f)) setNote(out[2].s[i], snapToScale(hi + 2, mood, key), 90);
        // endings
        for (int v = 0; v < 3; ++v) for (int i = cur.len - 1; i >= 0; --i) if (out[v].s[i].gate == 1) {
            out[v].s[i].notes[0] = (uint8_t)degNote(mood, key, v == 2 ? (L >= 5 ? 4 : 2) : 0, (mid >= 60 ? 4 : 3) + (v == 2 ? 0 : 0)); break; }
    }
};

} // namespace dfd
