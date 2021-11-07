#!/usr/bin/env bash

shopt -s nullglob

for file in upload/SuperTux*; do
    file_base=$(basename $file)
    echo "Uploading $file_base";
    url="https://supertux-ci-downloads.s3-us-west-2.amazonaws.com/${PREFIX}/$file_base"
    echo "Upload file to $url";
    size=$(($(wc -c < "$file")))
    shasum=$(shasum -a 256 "$file" | cut -d " " -f 1)
    curl --data "apikey=$DOWNLOAD_APIKEY" \
         --data "url=$url" \
         --data "size=$size" \
         --data "branch=$(git branch --show-current)" \
         --data "shasum=$shasum" \
         -L -s https://download.supertux.org/submit.php
done
