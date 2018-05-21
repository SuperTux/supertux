#!/usr/bin/env bash

shopt -s nullglob

. ~/urls.dat

for file in SuperTux*; do
    echo "Uploading $file";
    url="${urls[$file]}"
    size=$(($(wc -c < "$file")))
    shasum=$(shasum -a 256 "$file" | cut -d " " -f 1)
    curl --data "apikey=$DOWNLOAD_APIKEY" \
         --data "url=$url" \
         --data "size=$size" \
         --data "branch=$TRAVIS_BRANCH" \
         --data "shasum=$shasum" \
         -L -s -k https://download.supertux.org/submit.php
done
