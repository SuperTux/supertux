#!/usr/bin/env bash

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    sudo chmod -R +w /usr/local/Cellar
    cpack -G Bundle;
fi

if [ "$TRAVIS_OS_NAME" = "linux" ] && [ "$CC" = "gcc" ]; then
    cpack --config CPackSourceConfig.cmake -G TGZ;
fi

echo -e "Host upload.supertuxproject.org\n\tStrictHostKeyChecking no\n" >> ~/.ssh/config
eval "$(ssh-agent -s)"
chmod 600 ../.travis/travis-upload.pem
ssh-add ../.travis/travis-upload.pem

for file in SuperTux-*; do
    echo "Uploading $file";
    curl -T "$file" -umaths22:$BINTRAY_KEY "https://api.bintray.com/content/supertux/SuperTux-Nightly/travis/$TRAVIS_COMMIT/$file"
done
