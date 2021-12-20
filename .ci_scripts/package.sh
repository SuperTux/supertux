#!/usr/bin/env bash

shopt -s nullglob

if ([ "$OS_NAME" = "macos-10.15" ] || [ "$OS_NAME" = "macos-11" ]) && [ "$PACKAGE" = "ON" ]; then
    sudo chmod -R +w /usr/local/Cellar
    cpack -G Bundle;
fi

# make only one source package
if [ "$OS_NAME" = "ubuntu-latest" ] && [ "$COMPILER_NAME" = "gcc" ] && [ "$BUILD_NAME" = "Debug" ] && [ "$ARCH" = "64" ] && [ "$PACKAGE" = "ON" ]; then
    cpack --config CPackSourceConfig.cmake -G TGZ;
fi

if [ "$OS_NAME" = "ubuntu-latest" ] && [ "$PACKAGE" = "ON" ]; then
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
