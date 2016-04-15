#!/usr/bin/env bash


if [ "$TRAVIS_OS_NAME" = "linux" ] && [ "$CC" = "gcc" ]; then
    echo -e "Host upload.supertuxproject.org\n\tStrictHostKeyChecking no\n" >> ~/.ssh/config
    eval "$(ssh-agent -s)"
    chmod 600 ../.travis/travis-upload.pem
    ssh-add ../.travis/travis-upload.pem

    cpack --config CPackSourceConfig.cmake -G TGZ;
    for file in SuperTux-*; do
        echo "Uploading $file";
        scp $file travis-upload@upload.supertuxproject.org:/files/
    done
fi
