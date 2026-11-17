#!/bin/bash
# Dark Funk Durcal Synt v2 - installer for macOS (AU + VST3 + KeyLab Essential script for Ableton Live)
# Created at Polaris Sound Lab, Merida, Yucatan, Mexico
cd "$(dirname "$0")"
set -e
echo ""
echo "  DARK FUNK DURCAL SYNT v2"
echo "  Installing... / Instalando..."
COMP=~/Library/Audio/Plug-Ins/Components
VST3=~/Library/Audio/Plug-Ins/VST3
mkdir -p "$COMP" "$VST3"
rm -rf "$COMP/Dark Funk Durcal Synt.component" "$VST3/Dark Funk Durcal Synt.vst3"
cp -R "Plugins/Dark Funk Durcal Synt.component" "$COMP/"
cp -R "Plugins/Dark Funk Durcal Synt.vst3" "$VST3/"
mkdir -p ~/Applications && rm -rf ~/Applications/"Dark Funk Durcal Synt.app" && cp -R "Dark Funk Durcal Synt.app" ~/Applications/
# Ableton Live 11/12 user remote scripts folder
RS=~/Music/Ableton/"User Library"/"Remote Scripts"
mkdir -p "$RS" && rm -rf "$RS/DFD_KeyLab_Essential" && cp -R "KeyLab Essential (Ableton)/DFD_KeyLab_Essential" "$RS/"
# downloaded files are quarantined by macOS: lift it so Ableton can load them
xattr -dr com.apple.quarantine "$COMP/Dark Funk Durcal Synt.component" "$VST3/Dark Funk Durcal Synt.vst3" ~/Applications/"Dark Funk Durcal Synt.app" "$RS/DFD_KeyLab_Essential" 2>/dev/null || true
killall -9 AudioComponentRegistrar 2>/dev/null || true
echo ""
echo "  Done / Listo."
echo "  AU:     $COMP"
echo "  VST3:   $VST3"
echo "  App:    ~/Applications/Dark Funk Durcal Synt.app"
echo "  KeyLab: $RS/DFD_KeyLab_Essential"
echo ""
echo "  Now restart Ableton Live. / Ahora reinicia Ableton Live."
echo "  Live > Settings > Link, Tempo & MIDI > Control Surface: DFD_KeyLab_Essential"
echo ""
read -n 1 -s -r -p "  Press any key to close / Presiona una tecla para cerrar"
