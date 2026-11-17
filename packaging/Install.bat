@echo off
REM Dark Funk Durcal Synt v2 - Windows installer (Polaris Sound Lab, Merida, Yucatan, Mexico)
REM Copies the VST3 to the system VST3 folder (needs administrator) and the KeyLab script to Ableton's User Library.
net session >nul 2>&1
if %errorlevel% neq 0 (
  echo Pidiendo permiso de administrador / asking for administrator rights...
  powershell -NoProfile -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
  exit /b
)
set "SRC=%~dp0"
echo.
echo  DARK FUNK DURCAL SYNT v2
echo  Instalando / installing...
xcopy /E /I /Y /Q "%SRC%Plugins\Dark Funk Durcal Synt.vst3" "%CommonProgramFiles%\VST3\Dark Funk Durcal Synt.vst3" >nul
set "RS=%USERPROFILE%\Documents\Ableton\User Library\Remote Scripts"
if not exist "%RS%" mkdir "%RS%"
xcopy /E /I /Y /Q "%SRC%KeyLab Essential (Ableton)\DFD_KeyLab_Essential" "%RS%\DFD_KeyLab_Essential" >nul
echo.
echo  LISTO / DONE
echo  VST3:  %CommonProgramFiles%\VST3\Dark Funk Durcal Synt.vst3
echo  KeyLab: %RS%\DFD_KeyLab_Essential
echo  Abre tu DAW y busca "Dark Funk Durcal Synt" (Polaris Sound Lab).
echo  La app independiente esta en esta carpeta: Dark Funk Durcal Synt.exe
echo  KeyLab Essential + Ableton: conectalo y Live lo enlaza solo.
echo  Gratis y de codigo abierto. Donaciones (PayPal): drophlex@gmail.com
echo.
start "" "https://drophlex.github.io/dark-funk-durcal-synt-fm1/plugin.html"
pause
