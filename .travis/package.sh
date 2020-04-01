#!/usr/bin/env bash

shopt -s nullglob

if [ "$TRAVIS_OS_NAME" = "osx" ] && [ "$PACKAGE" = "ON" ]; then
    sudo chmod -R +w /usr/local/Cellar
    cpack -G Bundle;
fi

if [ "$TRAVIS_OS_NAME" = "linux" ] && [ "$PACKAGE" = "ON" ]; then
    cpack --config CPackSourceConfig.cmake -G TGZ;
    ../.travis/build_appimage.sh
    #extract built appimages for uploading
    mv ~/out/* .
fi

mkdir s3-upload
mv SuperTux* s3-upload/

if [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
    for file in s3-upload/SuperTux*; do
        echo "Uploading $file";
        url=$(curl --upload-file "$file" "https://transfer.sh/$file")
        echo $url
    done
fi
