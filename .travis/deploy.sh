#!/usr/bin/env bash

shopt -s nullglob

for file in s3-upload/SuperTux*; do
    file=$(basename($file))
    echo "Uploading $file";
    url="https://supertux-ci-downloads.s3-us-west-2.amazonaws.com/${TRAVIS_COMMIT}/travis/${TRAVIS_OS_NAME}/${TRAVIS_JOB_ID}/$file"
    size=$(($(wc -c < "$file")))
    shasum=$(shasum -a 256 "$file" | cut -d " " -f 1)
    curl --data "apikey=$DOWNLOAD_APIKEY" \
         --data "url=$url" \
         --data "size=$size" \
         --data "branch=$TRAVIS_BRANCH" \
         --data "shasum=$shasum" \
         -L -s https://download.supertux.org/submit.php
done
