#!/bin/bash
# OMEGA installer for macOS — Polaris Sound Lab
cd "$(dirname "$0")"
echo "★ Installing OMEGA…"
mkdir -p ~/Library/Audio/Plug-Ins/VST3 ~/Library/Audio/Plug-Ins/Components
rm -rf ~/Library/Audio/Plug-Ins/VST3/Omega.vst3 ~/Library/Audio/Plug-Ins/Components/Omega.component
cp -R Omega.vst3 ~/Library/Audio/Plug-Ins/VST3/
cp -R Omega.component ~/Library/Audio/Plug-Ins/Components/
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/Omega.vst3 2>/dev/null
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/Omega.component 2>/dev/null
killall -9 AudioComponentRegistrar 2>/dev/null
echo ""
echo "✓ Done! / ¡Listo!"
echo "Ableton: Preferences → Plug-Ins → turn on 'Use VST3 Plug-in System Folders' and press Rescan."
echo "Ableton: Preferencias → Plug-Ins → activa 'Usar carpetas de sistema VST3' y presiona Re-escanear."
read -n 1 -s -r -p "Press any key / Presiona una tecla…"
