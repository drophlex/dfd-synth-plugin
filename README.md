<p align="center"><img src="docs/plugin_ui.jpg" width="760" alt="Dark Funk Durcal Synt v2"></p>

<h1 align="center">DARK FUNK DURCAL SYNT v2 · plugin</h1>
<p align="center"><b>Free and open source (AGPL-3.0)</b><br>
AU · VST3 · Standalone — Mac (Apple Silicon + Intel) and Windows 10/11<br>
Acid · breaks · warped funk · IDM — always in key, made to play live</p>

<p align="center">
<a href="https://drophlex.github.io/dark-funk-durcal-synt-fm1/plugin.html"><img src="https://img.shields.io/badge/DOWNLOAD-MAC%20%C2%B7%20WINDOWS-ff4a1c?style=for-the-badge" alt="Download"></a>
<a href="https://github.com/drophlex/dfd-synth-plugin/releases/latest"><img src="https://img.shields.io/badge/RELEASES-v2-2fb8ff?style=for-the-badge" alt="Releases"></a>
<a href="https://www.paypal.com/donate/?business=drophlex%40gmail.com&currency_code=MXN"><img src="https://img.shields.io/badge/%E2%99%A5-buy%20me%20a%20coffee%20(PayPal)-555?style=flat-square" alt="Donate"></a>
</p>

## Install

| | |
|---|---|
| **Mac** | Download `DarkFunkDurcalSynt-v2-macOS.zip` → right-click **Install.command** › **Open**. |
| **Windows** | Download `DarkFunkDurcalSynt-v2-Windows.zip` → extract → double-click **Install.bat** › **Yes**. |
| **Ableton + KeyLab Essential** | Plug it in: Live links the `DFD_KeyLab_Essential` script by itself (screen, knobs, faders, pads, keys, transport). |
| **Other DAWs** | Logic, FL Studio, Bitwig, Reaper, Cubase, Studio One, GarageBand: find it as a *Polaris Sound Lab* instrument. |

**Manual (PDF):** [English](manual/DarkFunkDurcal-Plugin-Manual-EN.pdf) · [Español](manual/DarkFunkDurcal-Plugin-Manual-ES.pdf) · [both / ambos](manual/DarkFunkDurcal-Plugin-Manual-EN-ES.pdf)

It comes in **English**; the **ES** button in the bottom-right corner switches it to **Spanish**. **?** opens the quick guide.
Guide with pictures: **https://drophlex.github.io/dark-funk-durcal-synt-fm1/plugin.html**

<p align="center"><img src="docs/plugin_help.jpg" width="620" alt="Help"></p>

**Español:** viene en inglés; el botón **ES** (esquina de abajo a la derecha) lo pasa a español y **?** abre la guía rápida.
Mac: clic derecho en **Install.command** › **Abrir**. Windows: doble clic en **Install.bat**. En Ableton el KeyLab Essential se enlaza solo.

## Build

You need CMake 3.22+ and a C++17 compiler (Xcode on Mac, Visual Studio on Windows). JUCE 8 downloads itself.

```sh
cmake -S . -B build                      # Mac: -G Xcode -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build --config Release     # -> build/DarkFunkDurcalSynt_artefacts/Release/{AU,VST3,Standalone}
cmake -S . -B build-test -DDFD_TESTS=ON && cmake --build build-test --target DfdHostTest   # the host test
```

- `Source/dsp` — the engine (no JUCE): drums, 303-style acid, bass, lead, tape echo, reverb, stutter, generators.
- `Source/` — the JUCE plugin and its editor (English / Spanish). `ParamTable.h` = the 64 parameters in KeyLab bank order.
- `KeyLab/DFD_KeyLab_Essential` — the Ableton Live control surface (with auto-detection).
- `packaging/` — the installers (Install.command, Install.bat) and the readme.
- `.github/workflows/build.yml` — every push builds Mac (universal, AU validated with `auval`) and Windows and publishes
  the zips to the **v2** release.

## License

**GNU AGPL-3.0** (see `LICENSE`, `LICENSE-NOTES.md`). Built with JUCE 8 (AGPLv3). Fonts Cinzel and Barlow Condensed (OFL 1.1).

<sub>♥ If you like it, buy me a coffee · Si te gusta, invítame un café — PayPal **drophlex@gmail.com**</sub>

**Created at Polaris Sound Lab · Mérida, Yucatán, México** · [Instagram @polaris.sound.lab](https://www.instagram.com/polaris.sound.lab/) ·
[YouTube @PolarisSoundLab](https://www.youtube.com/@PolarisSoundLab) ·
*What is Dark Funk Durcal? Search it on Spotify, YouTube and every audio platform.*
