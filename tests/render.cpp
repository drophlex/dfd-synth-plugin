// Offline test: every style summoned and rendered 8 bars; peaks bounded, no NaN, never silent; WAV demos.
#include "../Source/dsp/Engine.h"
#include <cstdio>
#include <string>

static void wav(const char* path, const std::vector<float>& l, const std::vector<float>& r, int sr) {
    FILE* f = fopen(path, "wb"); if (!f) return;
    int n = (int)l.size(), bytes = n * 4;
    auto w32 = [&](uint32_t v) { fwrite(&v, 4, 1, f); }; auto w16 = [&](uint16_t v) { fwrite(&v, 2, 1, f); };
    fwrite("RIFF", 1, 4, f); w32(36 + bytes); fwrite("WAVEfmt ", 1, 8, f); w32(16); w16(1); w16(2); w32(sr); w32(sr * 4); w16(4); w16(16);
    fwrite("data", 1, 4, f); w32(bytes);
    for (int i = 0; i < n; ++i) { int16_t a = (int16_t)(dfd::clampf(l[i], -1, 1) * 32767), b = (int16_t)(dfd::clampf(r[i], -1, 1) * 32767); fwrite(&a, 2, 1, f); fwrite(&b, 2, 1, f); }
    fclose(f);
}

int main(int argc, char** argv) {
    const int sr = 48000; int fails = 0;
    std::string dir = argc > 1 ? argv[1] : "";
    for (int s = 0; s < dfd::NSTYLES; ++s) {
        dfd::Engine e; e.p.styleA = s; e.p.styleB = (s + 3) % dfd::NSTYLES; e.p.fusion = 0.25f; e.p.chaos = 0.5f;
        e.prepare((float)sr); e.internalPlay = true;   // its own PLAY (the DAW's play alone does not start it)
        e.post({dfd::C_SUMMON});
        float bpm = 0; std::vector<float> L, R; float peak = 0; double ms = 0; bool nan = false;
        double ppq = 0; const int blk = 256; float out[2][blk];
        // apply the style's sound like the wrapper does
        int frames = sr * 14;
        for (int i = 0; i < frames; i += blk) {
            if (e.summoned) { const auto* st = e.summonedStyle; e.p.acid.cutoff = st->cut; e.p.acid.reso = st->res; e.p.acid.envmod = st->env; e.p.acid.decay = st->dec; e.p.acid.drive = st->drive;
                e.p.drum.kit = st->kit; e.p.bass.mode = st->bassMode; e.p.lead.mode = e.summonedLead->leadMode; e.summoned = false; }
            bpm = e.pat.bpm;
            if (i == sr * 6) e.pad(5, true); if (i == sr * 7) e.pad(5, false);    // stutter
            if (i == sr * 8) e.pad(6, true); if (i == sr * 10) e.pad(6, false);   // build -> drop
            if (i == sr * 11) e.post({dfd::C_NEXT_GEN});
            e.process(out[0], out[1], blk, true, ppq, bpm);
            ppq += blk * bpm / 60.0 / sr;
            for (int k = 0; k < blk; ++k) { if (!std::isfinite(out[0][k])) nan = true; peak = std::max(peak, std::fabs(out[0][k])); ms += out[0][k] * out[0][k]; L.push_back(out[0][k]); R.push_back(out[1][k]); }
        }
        float rms = (float)std::sqrt(ms / L.size());
        bool ok = !nan && peak <= 1.0f && rms > 0.02f;
        printf("%-7s %-16s bpm %5.1f peak %.3f rms %.3f (%.1f dBFS) %s\n", dfd::kStyles[s].name, dfd::kStyles[s].full, bpm, peak, rms, 20 * std::log10(rms + 1e-9f), ok ? "ok" : "FAIL");
        fails += !ok;
        if (!dir.empty()) wav((dir + "/" + dfd::kStyles[s].name + ".wav").c_str(), L, R, sr);
    }
    printf(fails ? "render test FAILED (%d)\n" : "render test passed\n", fails);
    return fails != 0;
}
