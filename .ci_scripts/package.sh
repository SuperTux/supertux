#!/usr/bin/env bash

shopt -s nullglob

if [ "$OS_NAME" = "macos-latest" ] && [ "$PACKAGE" = "ON" ]; then
    sudo chmod -R +w /usr/local/Cellar
    cpack -G Bundle;
fi

# make only one source package
if [ "$OS_NAME" = "ubuntu-latest" ] && [ "$COMPILER_NAME" = "gcc" ] && [ "$BUILD_NAME" = "Debug" ] && [ "$PACKAGE" = "ON" ]; then
    cpack --config CPackSourceConfig.cmake -G TGZ;
fi

if [ "$OS_NAME" = "ubuntu-latest" ] && [ "$PACKAGE" = "ON" ]; then
    ../.ci_scripts/build_appimage.sh
    # extract built appimages for uploading
    mv ~/out/* .
fi

mkdir upload
mv SuperTux* upload/
