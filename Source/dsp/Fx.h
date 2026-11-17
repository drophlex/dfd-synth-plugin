// Dark Funk Durcal Synt — effects: three-head tape echo, reverb, beat-repeat stutter, master section.
#pragma once
#include "Common.h"

namespace dfd {

struct FxParams {
    int heads = 7;            // bitmask H1 H2 H3 (0 = plain digital delay)
    float echoTime = 0.375f;  // in beats (1 = quarter)
    float echoFb = 0.45f, echoWow = 0.3f, echoMix = 0.35f;
    float revSize = 0.6f, revDamp = 0.5f, revMix = 0.25f;
    float crushBits = 0, crushRate = 0;
    float filter = 0;         // -1 low-pass ... 0 open ... +1 high-pass (DJ filter macro)
    float drive = 0.15f, master = 0.8f;
};

struct TapeEcho {
    std::vector<float> bufL, bufR; int w = 0, n = 0; float sr = 48000;
    float wowPh = 0, fltPh = 0; OnePole lpL, lpR, hpL, hpR;
    void prepare(float s) { sr = s; n = 1; while (n < (int)(s * 6)) n <<= 1; bufL.assign(n, 0); bufR.assign(n, 0); w = 0; }
    float read(const std::vector<float>& b, float d) const {
        float pos = (float)w - d; while (pos < 0) pos += n;
        int i = (int)pos; float t = pos - i;
        return b[i & (n - 1)] + (b[(i + 1) & (n - 1)] - b[i & (n - 1)]) * t;
    }
    void process(float inL, float inR, float& outL, float& outR, const FxParams& p, float bpm) {
        float beat = 60.0f / std::max(bpm, 30.0f) * sr;
        float base = clampf(p.echoTime * beat, 64.0f, n / 3.2f);
        int heads = p.heads <= 0 ? 1 : p.heads;
        wowPh += 0.55f / sr; if (wowPh > 1) wowPh -= 1;
        fltPh += 5.8f / sr; if (fltPh > 1) fltPh -= 1;
        float mod = (std::sin(kTwoPi * wowPh) * 18.0f + std::sin(kTwoPi * fltPh) * 2.5f) * (p.heads > 0 ? p.echoWow : 0.0f);
        float sl = 0, sr2 = 0; int nh = 0;
        for (int h = 0; h < 3; ++h) if (heads >> h & 1) {
            float d = base * (h + 1) + mod * (h + 1);
            sl += read(bufL, d); sr2 += read(bufR, d * (p.heads > 0 ? 1.0f : 1.0f)); ++nh;
        }
        float norm = nh == 1 ? 1.0f : nh == 2 ? 0.7071f : 0.577f;
        float yl = sl * norm, yr = sr2 * norm;
        float fbl = sl / nh, fbr = sr2 / nh;
        lpL.set(3500.0f - p.echoWow * 1800.0f, sr); lpR.set(3500.0f - p.echoWow * 1800.0f, sr);
        hpL.set(p.heads > 0 ? 140.0f : 40.0f, sr); hpR.set(p.heads > 0 ? 140.0f : 40.0f, sr);
        fbl = hpL.hp(lpL.lp(fbl)); fbr = hpR.hp(lpR.lp(fbr));
        float fb = clampf(p.echoFb, 0.0f, 0.95f);
        float sat = p.heads > 0 ? 1.0f + p.echoWow * 2.0f : 1.0f;
        bufL[w] = softclip((inL + fbr * fb) * sat) / sat;   // ping-pong-ish cross feed
        bufR[w] = softclip((inR + fbl * fb) * sat) / sat;
        w = (w + 1) & (n - 1);
        outL = yl; outR = yr;
    }
};

struct Reverb {  // 4-line FDN with Householder mixing; cheap and smooth
    std::vector<float> d[4]; int len[4] = {0}, w[4] = {0}; float lp[4] = {0}; float sr = 48000;
    std::vector<float> ap[2]; int apLen[2] = {0}, apW[2] = {0};
    void prepare(float s) {
        sr = s; static const float L[4] = {0.0297f, 0.0371f, 0.0411f, 0.0437f};
        for (int i = 0; i < 4; ++i) { len[i] = (int)(L[i] * s * 1.6f); d[i].assign(len[i] + 1, 0); w[i] = 0; lp[i] = 0; }
        static const float A[2] = {0.0051f, 0.0127f};
        for (int i = 0; i < 2; ++i) { apLen[i] = (int)(A[i] * s); ap[i].assign(apLen[i] + 1, 0); apW[i] = 0; }
    }
    void process(float inL, float inR, float& outL, float& outR, const FxParams& p) {
        float x = (inL + inR) * 0.5f;
        for (int i = 0; i < 2; ++i) { float& z = ap[i][apW[i]]; float y = z - 0.6f * x; z = x + 0.6f * y; x = y; apW[i] = (apW[i] + 1) % apLen[i]; }
        float g = 0.7f + p.revSize * 0.27f, damp = 0.15f + p.revDamp * 0.6f;
        float o[4];
        for (int i = 0; i < 4; ++i) {
            int scaled = std::max(64, (int)(len[i] * (0.5f + p.revSize * 0.5f)));
            int r = w[i] - scaled; while (r < 0) r += len[i];
            o[i] = d[i][r];
            lp[i] += (o[i] - lp[i]) * (1.0f - damp);
            o[i] = lp[i];
        }
        float sum = (o[0] + o[1] + o[2] + o[3]) * 0.5f;
        for (int i = 0; i < 4; ++i) { d[i][w[i]] = x + (o[i] - sum) * g; w[i] = (w[i] + 1) % len[i]; }
        outL = o[0] + o[2] * 0.6f; outR = o[1] + o[3] * 0.6f;
    }
};

// beat-repeat: records continuously; while active, loops the slice captured at the press
struct Stutter {
    std::vector<float> L, R; int n = 0, w = 0, start = 0, pos = 0, sliceLen = 0; bool active = false; float sr = 48000, fade = 0;
    void prepare(float s) { sr = s; n = (int)(s * 4); L.assign(n, 0); R.assign(n, 0); w = 0; }
    void engage(float beats, float bpm) {
        sliceLen = std::max(32, (int)(beats * 60.0f / std::max(bpm, 30.0f) * sr));
        start = (w - sliceLen + n) % n; pos = 0; active = true;
    }
    void release() { active = false; }
    void process(float& l, float& r) {
        L[w] = l; R[w] = r; w = (w + 1) % n;
        float target = active ? 1.0f : 0.0f; fade += (target - fade) * 0.01f;
        if (fade < 0.001f) return;
        int i = (start + pos) % n; pos = (pos + 1) % std::max(sliceLen, 1);
        float env = std::min(1.0f, std::min(pos, sliceLen - pos) / 64.0f);  // de-click each repeat
        l = lerp(l, L[i] * env, fade); r = lerp(r, R[i] * env, fade);
    }
};

struct Master {
    Svf fl, fr; OnePole dcl, dcr; float sr = 48000; float holdL = 0, holdR = 0; int hc = 0; float gr = 1;
    void prepare(float s) { sr = s; dcl.set(15.0f, s); dcr.set(15.0f, s); }
    void process(float& l, float& r, const FxParams& p) {
        if (std::fabs(p.filter) > 0.02f) {
            if (p.filter < 0) { float c = 20000.0f * std::pow(2.0f, p.filter * 9.0f); fl.set(c, 0.9f + std::fabs(p.filter), sr); fr.set(c, 0.9f + std::fabs(p.filter), sr); l = fl.lp(l); r = fr.lp(r); }
            else { float c = 20.0f * std::pow(2.0f, p.filter * 9.0f); fl.set(c, 0.9f + p.filter, sr); fr.set(c, 0.9f + p.filter, sr); l = fl.hp(l); r = fr.hp(r); }
        }
        if (p.crushRate > 0.01f) { int hold = 1 + (int)(p.crushRate * p.crushRate * 30.0f); if (++hc >= hold) { hc = 0; holdL = l; holdR = r; } l = holdL; r = holdR; }
        if (p.crushBits > 0.01f) { float q = std::pow(2.0f, 14.0f - p.crushBits * 11.0f); l = std::round(l * q) / q; r = std::round(r * q) / q; }
        float dr = 1.0f + p.drive * 4.0f;
        l = softclip(l * dr) / std::sqrt(dr); r = softclip(r * dr) / std::sqrt(dr);
        l = dcl.hp(l); r = dcr.hp(r);
        // gentle peak limiter at about -1 dBFS: honest gain staging, no loudness trick
        float pk = std::max(std::fabs(l), std::fabs(r)) * p.master;
        float want = pk > 0.89f ? 0.89f / pk : 1.0f;
        gr = want < gr ? want : gr + (1.0f - gr) * 0.0005f;
        l *= p.master * gr; r *= p.master * gr;
    }
};

} // namespace dfd
