#!/bin/bash

IN="SDL-work/android-project"
out="mk/android"
mkdir -p "$out"

function _snack
{
    [[ -d "$IN/$1" ]] && mkdir -p "$out/$1" ||
        mv "$IN/$1" "$out"
}

VER=$1
if [[ -z "$1" ]]
then
    echo "Please pass the SDL2 version. (i.e. 2.32.10)"
    exit 1
fi

git clone https://github.com/libsdl-org/SDL.git -b release-${VER} SDL-work --depth 1

_snack "settings.gradle"
_snack "gradlew"
_snack "gradle/wrapper/gradle-wrapper.jar"
_snack "app/proguard-rules.pro"
cp -r "$IN/app/src/main/java" "$out/app/src/main"

# mv SDL-work/gradle/

rm -rf SDL-work
