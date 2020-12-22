#!/usr/bin/env bash

shopt -s nullglob

if [ "$OS_NAME" = "macos-latest" ] && [ "$PACKAGE" = "ON" ]; then
    sudo chmod -R +w /usr/local/Cellar
    cpack -G Bundle;
fi

if [ "$OS_NAME" = "ubuntu-latest" ] && [ "$PACKAGE" = "ON" ]; then
    cpack --config CPackSourceConfig.cmake -G TGZ;
    ../.ci_scripts/build_appimage.sh
    #extract built appimages for uploading
    mv ~/out/* .
fi

mkdir upload
mv SuperTux* upload/
