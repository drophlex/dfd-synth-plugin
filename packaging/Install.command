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
echo "  LISTO / DONE"
echo "  AU:     $COMP"
echo "  VST3:   $VST3"
echo "  App:    ~/Applications/Dark Funk Durcal Synt.app"
echo "  KeyLab: $RS/DFD_KeyLab_Essential"
echo ""
echo "  1. Reinicia tu DAW / restart your DAW."
echo "  2. KeyLab Essential + Ableton: conectalo y Live lo enlaza solo / plug it in, Live links it by itself."
echo "  3. Gratis y de codigo abierto / free and open source."
osascript -e 'display dialog "Dark Funk Durcal Synt quedó instalado.\n\n• Reinicia tu DAW (Ableton, Logic, etc.).\n• Conecta tu KeyLab Essential: Ableton lo enlaza solo.\n• Gratis y de código abierto.\n\nInstalled. Restart your DAW." with title "Polaris Sound Lab" buttons {"Ver guía / Guide", "OK"} default button "OK"' -e 'if button returned of result is "Ver guía / Guide" then open location "https://drophlex.github.io/dark-funk-durcal-synt-fm1/plugin.html"' >/dev/null 2>&1 || true
