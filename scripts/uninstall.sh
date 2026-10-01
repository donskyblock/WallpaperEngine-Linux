#!/bin/bash
set -e

echo "=== Wallpaper Engine Linux — Uninstaller ==="

SERVICE="wallpaper-engine-linux"
INSTALL_BIN="$HOME/.local/bin"
APP_DATA="$HOME/.local/share/wpe-linux"
DESKTOP_DIR="$HOME/.local/share/applications"
SYSTEMD_DIR="$HOME/.config/systemd/user"

# Stop and disable service
if systemctl --user is-active --quiet "$SERVICE" 2>/dev/null; then
    systemctl --user stop "$SERVICE"
    echo "[uninstall] Service stopped"
fi
if systemctl --user is-enabled --quiet "$SERVICE" 2>/dev/null; then
    systemctl --user disable "$SERVICE"
    echo "[uninstall] Service disabled"
fi
rm -f "$SYSTEMD_DIR/$SERVICE.service"
systemctl --user daemon-reload

# Remove binary, desktop entry, icon
rm -f "$INSTALL_BIN/$SERVICE"
rm -f "$DESKTOP_DIR/$SERVICE.desktop"
rm -f "$APP_DATA/icon.png"

echo ""
echo "=== Done ==="
echo "  Config/wallpapers in $APP_DATA are preserved."
echo "  Remove manually if wanted: rm -rf $APP_DATA"
