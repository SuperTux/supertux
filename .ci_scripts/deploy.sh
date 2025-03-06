#!/usr/bin/env bash

shopt -s nullglob

for file in upload/SuperTux*; do
    file_base=$(basename $file)
    echo "Uploading $file_base";
    url="https://supertux-ci-downloads.s3-us-west-2.amazonaws.com/${PREFIX}/$file_base"
    size=$(($(wc -c < "$file")))
    if [ $IS_WINDOWS = true ] ; then
        shasum=$(powershell -command "Get-FileHash \"$file\" -Algorithm SHA256 | Select-Object -ExpandProperty Hash")
    else
        shasum=$(shasum -a 256 "$file" | cut -d " " -f 1)
    fi
    echo "Checksum: $shasum";
    echo "Branch: $BRANCH_NAME";
    curl --data "apikey=$DOWNLOAD_APIKEY" \
         --data "url=$url" \
         --data "size=$size" \
         --data "branch=$BRANCH_NAME" \
         --data "shasum=$shasum" \
         -L -s https://download.supertux.org/submit.php
done
