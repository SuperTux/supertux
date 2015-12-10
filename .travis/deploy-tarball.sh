#!/usr/bin/env bash


if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    echo -e "Host brlcad.org\n\tStrictHostKeyChecking no\n" >> ~/.ssh/config
    eval "$(ssh-agent -s)"
    chmod 600 ../.travis/travis-upload.pem
    ssh-add ../.travis/travis-upload.pem

    cpack --config CPackSourceConfig.cmake -G TGZ;
    for file in SuperTux-*; do
        echo "Uploading $file";
        scp $file travis-upload@brlcad.org:/files/
    done
fi
