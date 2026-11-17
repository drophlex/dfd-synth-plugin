# Dark Funk Durcal Synt v2 — plugin (private)

VST3 / AU / Standalone instrument for macOS (Apple Silicon + Intel), with a control surface for the Arturia KeyLab
Essential (mk1) in Ableton Live. Created at Polaris Sound Lab, Mérida, Yucatán, México.

- `Source/dsp` — the engine (no JUCE): drums, 303-style acid, bass, lead, tape echo, reverb, stutter, generators.
- `Source/` — the JUCE plugin and its editor. `ParamTable.h` = the 64 parameters in KeyLab bank order.
- `KeyLab/DFD_KeyLab_Essential` — the Ableton Live control surface (LCD, knobs, faders, pads).
- `.github/workflows/build.yml` — every push to `main` builds and signs (ad hoc) the macOS universal plugins, validates
  the AU with `auval` and uploads **DarkFunkDurcalSynt-v2-macOS.zip** (Actions → the run → Artifacts).
- Tests: `g++ -std=c++17 -O2 -o render tests/render.cpp && ./render` (engine); `-DDFD_TESTS=ON` builds the host test.
