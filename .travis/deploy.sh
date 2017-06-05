#!/usr/bin/env bash

shopt -s nullglob

. ~/urls.dat

for file in SuperTux*; do
    echo "Uploading $file";
    url="${urls[$file]}"
    size=$(($(wc -c < "$file")))
    curl --data "apikey=$DOWNLOAD_APIKEY" --data "url=$url" --data "size=$size" -s https://download.supertuxproject.org/submit.php
done
