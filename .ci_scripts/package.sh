#!/usr/bin/env bash
set -e

shopt -s nullglob

if ([ "$OS_NAME" = "macos-10.15" ] || [ "$OS_NAME" = "macos-15" ]) && [ "$PACKAGE" = "ON" ]; then
    cpack -G Bundle
    dmgs=(SuperTux*.dmg)
    if ((${#dmgs[@]} == 0)); then
        echo "CPack produced no SuperTux DMG" >&2
        exit 1
    fi
    for dmg in "${dmgs[@]}"; do
        hdiutil verify "$dmg"
        mount_dir=$(mktemp -d)
        cleanup_bundle_mount() {
            hdiutil detach "$mount_dir" >/dev/null 2>&1 || true
            rmdir "$mount_dir" 2>/dev/null || true
        }
        trap cleanup_bundle_mount EXIT
        hdiutil attach -readonly -nobrowse -mountpoint "$mount_dir" "$dmg"
        ../tools/darwin/verify_bundle.sh "$mount_dir/SuperTux.app"
        cleanup_bundle_mount
        trap - EXIT
    done
fi

# make only one source package
if [ "$SOURCE" = "ON" ]; then
    cpack --config CPackSourceConfig.cmake -G TGZ;
fi

if ([ "$OS_NAME" = "ubuntu-26.04" ]) && [ "$PACKAGE" = "ON" ]; then
    ../.ci_scripts/build_appimage.sh
    # extract built appimages for uploading
    mv ~/out/* .

    # CI expects all artifacts to start with "SuperTux-", AppImage generates "SuperTux_v2-...."
    for filename in SuperTux_2-*.AppImage; do 
        [ -f "$filename" ] || continue
        mv "$filename" "${filename//_2/}"
    done
fi

mkdir upload
mv SuperTux* upload/
