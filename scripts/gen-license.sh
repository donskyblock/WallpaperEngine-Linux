#!/bin/bash
set -e

# Generate / verify the repo LICENSE + NOTICE files.
#
# LICENSE is verbatim GPL 3.0 fetched from gnu.org.
# NOTICE contains your project name + copyright + the GPL "How to Apply" boilerplate.
#
# Usage:
#   gen-license.sh                       Interactive: prompts for author/year/project
#   gen-license.sh --check               Verify LICENSE matches upstream; warn-only
#   gen-license.sh --author NAME --year YYYY --project NAME
#                                        Non-interactive (CI-safe)
#   gen-license.sh --license-only        Skip NOTICE (just refresh GPL text)

GPL_URL="https://www.gnu.org/licenses/gpl-3.0.txt"
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LICENSE_PATH="$REPO_ROOT/LICENSE"
NOTICE_PATH="$REPO_ROOT/NOTICE"
PKG_JSON="$REPO_ROOT/package.json"

MODE="write"
AUTHOR=""
YEAR=""
PROJECT=""
SKIP_NOTICE=0

while [ $# -gt 0 ]; do
    case "$1" in
        --check)         MODE="check"; shift;;
        --force|--write) MODE="write"; shift;;
        --license-only)  SKIP_NOTICE=1; shift;;
        --author)        AUTHOR="$2"; shift 2;;
        --year)          YEAR="$2"; shift 2;;
        --project)       PROJECT="$2"; shift 2;;
        -h|--help)
            sed -n '3,15p' "$0" | sed 's/^# \?//'
            exit 0;;
        *) echo "[license] unknown arg: $1" >&2; exit 2;;
    esac
done

# --- helpers ---

fetch() {
    local out="$1"
    if command -v curl >/dev/null 2>&1; then
        curl -fsSL "$GPL_URL" -o "$out"
    elif command -v wget >/dev/null 2>&1; then
        wget -q "$GPL_URL" -O "$out"
    else
        echo "[license] error: need curl or wget" >&2
        return 1
    fi
}

validate_gpl() {
    local f="$1"
    [ -s "$f" ] || { echo "[license] error: $f empty"; return 1; }
    head -n 5 "$f" | grep -q "GNU GENERAL PUBLIC LICENSE" \
        || { echo "[license] error: $f missing GPL header"; return 1; }
    head -n 5 "$f" | grep -q "Version 3" \
        || { echo "[license] error: $f not version 3"; return 1; }
}

# Pull a top-level string field from package.json. Tolerates simple "key": "val" lines.
pkg_field() {
    local key="$1"
    [ -f "$PKG_JSON" ] || return 1
    grep -E "^[[:space:]]*\"$key\"[[:space:]]*:" "$PKG_JSON" \
        | head -n1 \
        | sed -E 's/^[[:space:]]*"[^"]+"[[:space:]]*:[[:space:]]*"([^"]*)".*/\1/'
}

prompt_default() {
    # prompt_default "Label" "default" -> echoes user input or default
    local label="$1" def="$2" ans=""
    if [ -n "$def" ]; then
        read -r -p "$label [$def]: " ans </dev/tty
        echo "${ans:-$def}"
    else
        read -r -p "$label: " ans </dev/tty
        echo "$ans"
    fi
}

write_notice() {
    local project="$1" year="$2" author="$3"
    cat > "$NOTICE_PATH" <<EOF
$project
Copyright (C) $year  $author

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

Full license text: see LICENSE in this directory, or https://www.gnu.org/licenses/gpl-3.0.txt
EOF
}

# --- modes ---

case "$MODE" in
    check)
        if [ ! -f "$LICENSE_PATH" ]; then
            echo "[license] warn: LICENSE not found at $LICENSE_PATH"
            echo "[license] hint: run 'npm run license:gen' to create it"
            exit 0
        fi
        if ! validate_gpl "$LICENSE_PATH" 2>/dev/null; then
            echo "[license] warn: existing LICENSE failed header check"
            exit 0
        fi
        TMP="$(mktemp)"
        trap 'rm -f "$TMP"' EXIT
        if ! fetch "$TMP"; then
            echo "[license] warn: fetch failed; skipping drift check"
            exit 0
        fi
        if cmp -s "$LICENSE_PATH" "$TMP"; then
            echo "[license] ok: LICENSE matches upstream GPL 3.0"
        else
            echo "[license] warn: LICENSE differs from upstream GPL 3.0"
            echo "[license] hint: run 'npm run license:gen' to refresh"
        fi
        if [ ! -f "$NOTICE_PATH" ]; then
            echo "[license] warn: NOTICE not found (no copyright header)"
            echo "[license] hint: run 'npm run license:gen' to create it"
        fi
        ;;
    write)
        # Collect details unless --license-only
        if [ "$SKIP_NOTICE" -eq 0 ]; then
            DEFAULT_PROJECT="$(pkg_field name 2>/dev/null || true)"
            DEFAULT_AUTHOR="$(pkg_field author 2>/dev/null || true)"
            DEFAULT_YEAR="$(date +%Y)"

            if [ -t 0 ]; then
                [ -z "$PROJECT" ] && PROJECT="$(prompt_default 'Project name' "$DEFAULT_PROJECT")"
                [ -z "$AUTHOR" ]  && AUTHOR="$(prompt_default  'Author name'  "$DEFAULT_AUTHOR")"
                [ -z "$YEAR" ]    && YEAR="$(prompt_default    'Year'         "$DEFAULT_YEAR")"
            else
                [ -z "$PROJECT" ] && PROJECT="$DEFAULT_PROJECT"
                [ -z "$AUTHOR" ]  && AUTHOR="$DEFAULT_AUTHOR"
                [ -z "$YEAR" ]    && YEAR="$DEFAULT_YEAR"
            fi

            if [ -z "$AUTHOR" ] || [ -z "$PROJECT" ] || [ -z "$YEAR" ]; then
                echo "[license] error: missing author/project/year (no TTY for prompts)" >&2
                echo "[license] hint: pass --author NAME --project NAME --year YYYY" >&2
                exit 2
            fi
        fi

        echo "[license] fetching GPL 3.0 from $GPL_URL"
        TMP="$(mktemp)"
        trap 'rm -f "$TMP"' EXIT
        fetch "$TMP"
        validate_gpl "$TMP"
        mv "$TMP" "$LICENSE_PATH"
        trap - EXIT
        echo "[license] wrote $LICENSE_PATH"

        if [ "$SKIP_NOTICE" -eq 0 ]; then
            write_notice "$PROJECT" "$YEAR" "$AUTHOR"
            echo "[license] wrote $NOTICE_PATH ($PROJECT / $YEAR / $AUTHOR)"
        fi
        ;;
esac
