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

declare -A urls

for file in SuperTux*; do
    echo "Uploading $file";
    url=$(curl --upload-file "$file" "https://transfer.sh/$file")
    urls[$file]=$url
    echo $url
done

declare -p urls > ~/urls.dat