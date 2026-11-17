// Dark Funk Durcal Synt — shared DSP helpers. (c) Polaris Sound Lab, Mérida, Yucatán, México. All rights reserved.
#pragma once
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <array>
#include <vector>
#include <cstring>

namespace dfd {

constexpr float kPi = 3.14159265358979f;
constexpr float kTwoPi = 6.28318530717959f;

inline float clampf(float x, float a, float b) { return x < a ? a : (x > b ? b : x); }
inline float mtof(float n) { return 440.0f * std::pow(2.0f, (n - 69.0f) / 12.0f); }
inline float dbToGain(float db) { return std::pow(10.0f, db / 20.0f); }
inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

// fast tanh-ish soft clip, smooth and bounded to ±1
inline float softclip(float x) {
    if (x > 3.0f) return 1.0f;
    if (x < -3.0f) return -1.0f;
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

// deterministic xorshift RNG (patterns must reproduce from a seed: undo, scenes, tests)
struct Rng {
    uint32_t s = 0x2545F491u;
    explicit Rng(uint32_t seed = 0x2545F491u) : s(seed ? seed : 1u) {}
    uint32_t next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s; }
    float uni() { return (next() >> 8) * (1.0f / 16777216.0f); }           // [0,1)
    int range(int n) { return n <= 0 ? 0 : (int)(next() % (uint32_t)n); }  // [0,n)
    bool chance(float p) { return uni() < p; }
    float bi() { return uni() * 2.0f - 1.0f; }
};

// polyBLEP band-limited oscillator pieces
inline float polyblep(float t, float dt) {
    if (t < dt) { t /= dt; return t + t - t * t - 1.0f; }
    if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
    return 0.0f;
}

struct Osc {
    float ph = 0.0f;
    float saw(float dt) { float v = 2.0f * ph - 1.0f - polyblep(ph, dt); adv(dt); return v; }
    float sqr(float dt, float pw = 0.5f) {
        float v = (ph < pw ? 1.0f : -1.0f) + polyblep(ph, dt) - polyblep(std::fmod(ph + 1.0f - pw, 1.0f), dt);
        adv(dt); return v;
    }
    float sine(float dt) { float v = std::sin(kTwoPi * ph); adv(dt); return v; }
    float tri(float dt) { float v = 4.0f * std::fabs(ph - 0.5f) - 1.0f; adv(dt); return v; }
    void adv(float dt) { ph += dt; if (ph >= 1.0f) ph -= 1.0f; if (ph < 0.0f) ph += 1.0f; }
};

// TPT state-variable filter (Zavalishin): stable at any cutoff, modulation-friendly
struct Svf {
    float ic1 = 0, ic2 = 0, g = 0, k = 1, a1 = 0, a2 = 0, a3 = 0;
    void set(float fc, float q, float sr) {
        fc = clampf(fc, 10.0f, sr * 0.45f);
        g = std::tan(kPi * fc / sr);
        k = 1.0f / std::max(q, 0.05f);
        a1 = 1.0f / (1.0f + g * (g + k)); a2 = g * a1; a3 = g * a2;
    }
    // returns low; band/high in refs
    float tick(float v0, float& bp, float& hp) {
        float v3 = v0 - ic2;
        float v1 = a1 * ic1 + a2 * v3;
        float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2 * v1 - ic1; ic2 = 2 * v2 - ic2;
        bp = v1; hp = v0 - k * v1 - v2;
        return v2;
    }
    float lp(float x) { float b, h; return tick(x, b, h); }
    float bp(float x) { float b, h; tick(x, b, h); return b; }
    float hp(float x) { float b, h; tick(x, b, h); return h; }
    void reset() { ic1 = ic2 = 0; }
};

// one-pole helpers
struct OnePole {
    float z = 0, a = 0;
    void set(float fc, float sr) { a = 1.0f - std::exp(-kTwoPi * clampf(fc, 1.0f, sr * 0.49f) / sr); }
    float lp(float x) { z += a * (x - z); return z; }
    float hp(float x) { z += a * (x - z); return x - z; }
};

// exponential decay envelope with optional attack
struct Env {
    float v = 0, target = 0, rate = 0; int stage = 0; // 0 idle 1 attack 2 decay
    float atk = 0.001f, dec = 0.2f, sr = 48000;
    void trigger(float peak = 1.0f) { target = peak; stage = 1; }
    void setTimes(float a, float d, float s) { atk = std::max(a, 0.0005f); dec = std::max(d, 0.002f); sr = s; }
    float tick() {
        if (stage == 1) { v += target / (atk * sr); if (v >= target) { v = target; stage = 2; } }
        else if (stage == 2) { v *= std::exp(-1.0f / (dec * sr / 4.6f)); if (v < 1e-5f) { v = 0; stage = 0; } }
        return v;
    }
    bool active() const { return stage != 0; }
};

// ADSR for sustained voices
struct Adsr {
    float a = 0.01f, d = 0.2f, s = 0.7f, r = 0.3f, sr = 48000, v = 0; int st = 0; // 0 off 1 a 2 d 3 s 4 r
    void on() { st = 1; }
    void off() { if (st) st = 4; }
    float tick() {
        switch (st) {
            case 1: v += 1.0f / std::max(a * sr, 1.0f); if (v >= 1) { v = 1; st = 2; } break;
            case 2: v += (s - v) * (1.0f - std::exp(-4.6f / std::max(d * sr, 1.0f))); if (std::fabs(v - s) < 1e-4f) st = 3; break;
            case 3: v = s; break;
            case 4: v *= std::exp(-4.6f / std::max(r * sr, 1.0f)); if (v < 1e-5f) { v = 0; st = 0; } break;
        }
        return v;
    }
    bool active() const { return st != 0; }
};

} // namespace dfd
