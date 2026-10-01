#!/bin/bash
set -e

INSTALL_PATH="$HOME/.local/bin/linux-wallpaperengine"
SYSTEM_PATH="/usr/bin/linux-wallpaperengine"

if [ -f "$INSTALL_PATH" ] || [ -f "$SYSTEM_PATH" ]; then
    echo "[lwe] Already installed — skipping"
    exit 0
fi

# Try AUR first (handles CEF + all deps automatically)
if command -v yay >/dev/null 2>&1; then
    echo "[lwe] Installing via yay (AUR)..."
    yay -S --needed --noconfirm linux-wallpaperengine
    echo "[lwe] Installed via AUR"
    exit 0
fi

if command -v paru >/dev/null 2>&1; then
    echo "[lwe] Installing via paru (AUR)..."
    paru -S --needed --noconfirm linux-wallpaperengine
    echo "[lwe] Installed via AUR"
    exit 0
fi

# Fall back to source build
echo "[lwe] No AUR helper found — building from source..."
echo "[lwe] Note: requires ~1GB disk for CEF download. Install yay for easier setup."
echo "[lwe] Deps: sudo pacman -S --needed cmake git gcc base-devel glew glslang ffmpeg mpv"

for dep in cmake git g++ make; do
    command -v "$dep" >/dev/null 2>&1 || { echo "[lwe] Error: missing $dep"; exit 1; }
done

BUILD_DIR=$(mktemp -d)
echo "[lwe] Cloning linux-wallpaperengine..."
git clone --depth 1 https://github.com/Almamu/linux-wallpaperengine "$BUILD_DIR/src"

echo "[lwe] Configuring (will download CEF ~300MB)..."
cmake -S "$BUILD_DIR/src" -B "$BUILD_DIR/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -Wno-dev

echo "[lwe] Building..."
cmake --build "$BUILD_DIR/build" --parallel "$(nproc)"

BUILT_BIN="$BUILD_DIR/build/linux-wallpaperengine"
if [ ! -f "$BUILT_BIN" ]; then
    echo "[lwe] Error: build finished but binary not found"
    exit 1
fi

mkdir -p "$HOME/.local/bin"
cp "$BUILT_BIN" "$INSTALL_PATH"
chmod +x "$INSTALL_PATH"
rm -rf "$BUILD_DIR"

echo "[lwe] Installed → $INSTALL_PATH"
