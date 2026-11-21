#!/usr/bin/env bash
set -euo pipefail

root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
build_dir=${WPE_BUILD_DIR:-"$root/build"}

if [[ ${1:-} == --help ]]; then
    printf 'Usage: %s\nInstalls the existing build. Set DESTDIR to stage files.\n' "${0##*/}"
    exit 0
fi
if (( $# != 0 )); then
    printf 'Set CMake install options with build.sh before running install.sh.\n' >&2
    exit 2
fi
if [[ ! -x "$build_dir/bin/wpe-linux" || ! -x "$build_dir/bin/wpe-wallpaper-host" || ! -x "$build_dir/bin/wpectl" ]]; then
    printf 'Build the app first with %s/build.sh.\n' "$root" >&2
    exit 1
fi

cmake --install "$build_dir"
