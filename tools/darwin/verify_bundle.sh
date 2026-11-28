#!/usr/bin/env bash
set -euo pipefail

app=${1:?Usage: verify_bundle.sh /path/to/SuperTux.app}
codesign --verify --deep --strict --verbose=2 "$app"

mach_o_count=0
while IFS= read -r -d '' item; do
    if [[ $(file -b "$item") == Mach-O* ]]; then
        codesign --verify --strict --verbose=2 "$item"
        ((mach_o_count += 1))
    fi
done < <(find "$app/Contents" -type f -print0)

if ((mach_o_count == 0)); then
    echo "No Mach-O files found in $app" >&2
    exit 1
fi
echo "Verified $mach_o_count Mach-O files and $app"
