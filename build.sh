#!/usr/bin/env bash
set -euo pipefail

root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
build_dir=${WPE_BUILD_DIR:-"$root/build"}

if [[ ${1:-} == --help ]]; then
    printf 'Usage: %s [CMake configure options]\n' "${0##*/}"
    exit 0
fi

cmake -S "$root" -B "$build_dir" -G Ninja -DCMAKE_BUILD_TYPE=Release "$@"
cmake --build "$build_dir" --parallel "${CMAKE_BUILD_PARALLEL_LEVEL:-4}"
