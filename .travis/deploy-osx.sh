#!/usr/bin/env bash


if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    echo -e "Host brlcad.org\n\tStrictHostKeyChecking no\n" >> ~/.ssh/config
    eval "$(ssh-agent -s)"
    chmod 600 ../.travis/travis-upload.pem
    ssh-add ../.travis/travis-upload.pem

    sudo chmod -R +w /usr/local/Cellar
    cpack -G Bundle;
    for file in SuperTux-*; do
        echo "Uploading $file";
        scp $file travis-upload@upload.supertuxproject.org:/files/
    done
fi
