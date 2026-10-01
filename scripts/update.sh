#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$SCRIPT_DIR/.."

SERVICE="wallpaper-engine-linux"
INSTALL_BIN="$HOME/.local/bin"
APP_DATA="$HOME/.local/share/wpe-linux"
DESKTOP_DIR="$HOME/.local/share/applications"
SYSTEMD_DIR="$HOME/.config/systemd/user"

BIN_PATH="$INSTALL_BIN/$SERVICE"
SERVICE_PATH="$SYSTEMD_DIR/$SERVICE.service"
SERVICE_FILE="$SCRIPT_DIR/$SERVICE.service"

echo "=== Wallpaper Engine Linux — Updater ==="
echo ""

# ── detect what is installed ──────────────────────────────────────────────────

app_installed=false
service_installed=false

[ -f "$BIN_PATH" ]      && app_installed=true
[ -f "$SERVICE_PATH" ]  && service_installed=true

# ── handle missing components ─────────────────────────────────────────────────

ask_install() {
    local component="$1"
    read -rp "  $component is not installed. Install it now? [Y/n] " reply
    reply="${reply:-Y}"
    [[ "$reply" =~ ^[Yy]$ ]]
}

if ! $app_installed; then
    echo "[update] App binary not found at $BIN_PATH"
    if ask_install "App"; then
        app_install=true
    else
        app_install=false
        echo "[update] Skipping app install."
    fi
else
    app_install=true
fi

if ! $service_installed; then
    echo "[update] Systemd service not found at $SERVICE_PATH"
    if ask_install "Systemd service"; then
        service_install=true
    else
        service_install=false
        echo "[update] Skipping service install."
    fi
else
    service_install=true
fi

# nothing to do
if ! $app_install && ! $service_install; then
    echo ""
    echo "Nothing to update. Exiting."
    exit 0
fi

# ── build ─────────────────────────────────────────────────────────────────────

if $app_install; then
    for dep in npm cargo; do
        command -v "$dep" >/dev/null 2>&1 || { echo "Error: $dep not found in PATH"; exit 1; }
    done

    echo ""
    echo "[update] Building app..."
    cd "$ROOT"
    npm run tauri build -- --no-bundle

    BINARY=$(find src-tauri/target/release -maxdepth 1 -name "$SERVICE" -type f | head -1)
    if [ -z "$BINARY" ]; then
        echo "[update] Error: build succeeded but binary not found"
        exit 1
    fi
fi

# ── stop service if running (before replacing binary) ─────────────────────────

was_running=false
if $service_installed; then
    if systemctl --user is-active --quiet "$SERVICE" 2>/dev/null; then
        was_running=true
        echo "[update] Stopping running service..."
        systemctl --user stop "$SERVICE"
    fi
fi

# ── install / update app ──────────────────────────────────────────────────────

if $app_install; then
    mkdir -p "$INSTALL_BIN" "$APP_DATA" "$DESKTOP_DIR"

    cp "$BINARY" "$BIN_PATH"
    chmod +x "$BIN_PATH"

    cp src-tauri/icons/icon.png "$APP_DATA/icon.png"

    cat > "$DESKTOP_DIR/$SERVICE.desktop" << EOF
[Desktop Entry]
Name=Wallpaper Engine Linux
Comment=Steam Workshop wallpapers for Hyprland
Exec=env WEBKIT_DISABLE_COMPOSITING_MODE=1 $BIN_PATH
Icon=$APP_DATA/icon.png
Terminal=false
Type=Application
Categories=Utility;
Keywords=wallpaper;steam;hyprland;workshop;
StartupWMClass=$SERVICE
EOF

    echo "[update] App updated: $BIN_PATH"
fi

# ── install / update service ──────────────────────────────────────────────────

if $service_install; then
    mkdir -p "$SYSTEMD_DIR"
    cp "$SERVICE_FILE" "$SERVICE_PATH"
    systemctl --user daemon-reload

    if ! $service_installed; then
        # first install — ask to enable
        read -rp "  Enable service to start at login? [Y/n] " reply
        reply="${reply:-Y}"
        if [[ "$reply" =~ ^[Yy]$ ]]; then
            systemctl --user enable "$SERVICE"
            echo "[update] Service enabled (starts at graphical login)"
        fi
    fi

    echo "[update] Service updated: $SERVICE_PATH"
fi

# ── restart if was running ────────────────────────────────────────────────────

if $was_running || { $service_install && ! $service_installed; }; then
    if systemctl --user is-enabled --quiet "$SERVICE" 2>/dev/null; then
        read -rp "  Start service now? [Y/n] " reply
        reply="${reply:-Y}"
        if [[ "$reply" =~ ^[Yy]$ ]]; then
            systemctl --user start "$SERVICE"
            echo "[update] Service started"
        fi
    fi
fi

# ── summary ───────────────────────────────────────────────────────────────────

echo ""
echo "=== Done ==="
$app_install     && echo "  Binary:  $BIN_PATH"
$app_install     && echo "  Desktop: $DESKTOP_DIR/$SERVICE.desktop"
$service_install && echo "  Service: $SERVICE_PATH"
echo ""
echo "  status:  systemctl --user status $SERVICE"
echo "  start:   systemctl --user start $SERVICE"
echo "  stop:    systemctl --user stop $SERVICE"
echo ""

# warn if ~/.local/bin not in PATH
if $app_install && ! echo "$PATH" | grep -q "$HOME/.local/bin"; then
    echo "  Warning: $HOME/.local/bin is not in your PATH"
    echo "  fish: fish_add_path ~/.local/bin"
    echo "  bash: export PATH=\"\$HOME/.local/bin:\$PATH\""
fi
