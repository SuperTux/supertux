#!/usr/bin/env bash

shopt -s nullglob

if [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    sudo chmod -R +w /usr/local/Cellar
    cpack -G Bundle;
fi

if [ "${TRAVIS_OS_NAME}" = "linux" ] && [ "${CC}" = "gcc" ]; then
    cpack --config CPackSourceConfig.cmake -G TGZ;
fi

for file in SuperTux-*; do
    hash="$(sha512sum "${file}" | awk -F' ' '{print $1;}')"
    printf 'Uploading %s (SHA-2-512: %s)\n' "${file}" "${hash}"
    curl --upload-file "${file}" "https://transfer.sh/${file}"
done
