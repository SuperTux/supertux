#!/usr/bin/env bash

shopt -s nullglob

if [ "$OS_NAME" = "macos-latest" ] && [ "$PACKAGE" = "ON" ]; then
    sudo chmod -R +w /usr/local/Cellar
    cpack -G Bundle;
fi

if [ "$OS_NAME" = "ubuntu-latest" ] && [ "$PACKAGE" = "ON" ]; then
    cpack --config CPackSourceConfig.cmake -G TGZ;
    ../.travis/build_appimage.sh
    #extract built appimages for uploading
    mv ~/out/* .
fi

mkdir upload
mv SuperTux* upload/

if [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
    for file in s3-upload/SuperTux*; do
        echo "Uploading $file";
        url=$(curl --upload-file "$file" "https://transfer.sh/$file")
        echo $url
    done
fi
