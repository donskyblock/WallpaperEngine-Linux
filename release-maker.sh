#!/usr/bin/env bash
set -euo pipefail

root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
if [[ ${1:-} == --help ]] || (( $# > 1 )); then
    printf 'Usage: %s [output directory]\n' "${0##*/}"
    if (( $# > 1 )); then exit 2; fi
    exit 0
fi

output=${1:-"$root/release"}
mkdir -p -- "$output"
output=$(cd -- "$output" && pwd)
work=$(mktemp -d "$output/.release.XXXXXXXX")
trap 'rm -rf -- "$work"' EXIT

export WPE_BUILD_DIR="$work/build"
"$root/build.sh" -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_LIBDIR=lib -DWPE_REQUIRE_LAYER_SHELL=ON
ctest --test-dir "$WPE_BUILD_DIR" --output-on-failure
DESTDIR="$work/stage" "$root/install.sh"

version=$(sed -n 's/^CMAKE_PROJECT_VERSION:STATIC=//p' "$WPE_BUILD_DIR/CMakeCache.txt")
: "${version:?CMake project version missing}"
name="wpe-linux-$version"
source_archive="$name-source.tar.gz"
binary_archive="$name-linux-$(uname -m).tar.gz"

install -Dm644 "$root/LICENSE" "$work/stage/usr/share/doc/wpe-linux/LICENSE"
install -m644 "$root/NOTICE" "$root/README.md" "$work/stage/usr/share/doc/wpe-linux/"
tar -czf "$work/$binary_archive" --owner=0 --group=0 -C "$work/stage" usr
tar -czf "$work/$source_archive" -C "$root" --owner=0 --group=0 \
    --transform "flags=r;s,^,$name/," \
    .gitignore build.sh install.sh release-maker.sh CMakeLists.txt CONTRIBUTING.md LICENSE NOTICE README.md \
    catalog data docs examples extensions src tests tools

(
    cd -- "$work"
    sha256sum "$binary_archive" "$source_archive" > "$name-SHA256SUMS"
)
mv -- "$work/$binary_archive" "$work/$source_archive" "$work/$name-SHA256SUMS" "$output/"
printf 'Release files written to %s\n' "$output"
