#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$SCRIPT_DIR/.."

echo "=== Wallpaper Engine Linux — Installer ==="

for dep in npm cargo cmake git; do
    command -v "$dep" >/dev/null 2>&1 || { echo "Error: $dep not found in PATH"; exit 1; }
done

# Fetch/build linux-wallpaperengine
bash "$SCRIPT_DIR/fetch-lwe.sh"

# Build the Tauri app (no-bundle = just the binary, no deb/AppImage)
echo "[install] Building app..."
cd "$ROOT"
npm run tauri build -- --no-bundle

BINARY=$(find src-tauri/target/release -maxdepth 1 -name "wallpaper-engine-linux" -type f | head -1)
if [ -z "$BINARY" ]; then
    echo "[install] Error: build succeeded but binary not found"
    exit 1
fi

INSTALL_BIN="$HOME/.local/bin"
APP_DATA="$HOME/.local/share/wpe-linux"
DESKTOP_DIR="$HOME/.local/share/applications"
SYSTEMD_DIR="$HOME/.config/systemd/user"

mkdir -p "$INSTALL_BIN" "$APP_DATA" "$DESKTOP_DIR" "$SYSTEMD_DIR"

cp "$BINARY" "$INSTALL_BIN/wallpaper-engine-linux"
chmod +x "$INSTALL_BIN/wallpaper-engine-linux"

cp src-tauri/icons/icon.png "$APP_DATA/icon.png"

cat > "$DESKTOP_DIR/wallpaper-engine-linux.desktop" << EOF
[Desktop Entry]
Name=Wallpaper Engine Linux
Comment=Steam Workshop wallpapers for Hyprland
Exec=env WEBKIT_DISABLE_COMPOSITING_MODE=1 $INSTALL_BIN/wallpaper-engine-linux
Icon=$APP_DATA/icon.png
Terminal=false
Type=Application
Categories=Utility;
Keywords=wallpaper;steam;hyprland;workshop;
StartupWMClass=wallpaper-engine-linux
EOF

# Install systemd user service
cp "$SCRIPT_DIR/wallpaper-engine-linux.service" "$SYSTEMD_DIR/wallpaper-engine-linux.service"
systemctl --user daemon-reload

# Enable service (start on login) unless --no-service passed
if [[ "$*" != *"--no-service"* ]]; then
    systemctl --user enable wallpaper-engine-linux.service
    echo "[install] Service enabled (starts at graphical login)"
    echo ""
    echo "  To start now:    systemctl --user start wallpaper-engine-linux"
    echo "  To stop:         systemctl --user stop wallpaper-engine-linux"
    echo "  To disable:      systemctl --user disable wallpaper-engine-linux"
else
    echo "[install] Skipped service enable (--no-service)"
fi

echo ""
echo "=== Done ==="
echo "  Binary:  $INSTALL_BIN/wallpaper-engine-linux"
echo "  Desktop: $DESKTOP_DIR/wallpaper-engine-linux.desktop"
echo "  Service: $SYSTEMD_DIR/wallpaper-engine-linux.service"
echo ""

# Warn if ~/.local/bin not in PATH
if ! echo "$PATH" | grep -q "$HOME/.local/bin"; then
    echo "  Warning: $HOME/.local/bin is not in your PATH"
    echo "  Add to ~/.bashrc or ~/.config/fish/config.fish:"
    echo "    fish: fish_add_path ~/.local/bin"
    echo "    bash: export PATH=\"\$HOME/.local/bin:\$PATH\""
fi
