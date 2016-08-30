#!/usr/bin/env bash

shopt -s nullglob

for file in SuperTux-*; do
    echo "Uploading $file";
    curl -T "$file" -umaths22:$BINTRAY_KEY "https://api.bintray.com/content/supertux/SuperTux-Nightly/travis/$TRAVIS_BUILD_NUMBER/$file?publish=1&override=1"
done
