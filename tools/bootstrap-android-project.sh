#!/bin/bash
#
# This tool can be invoked to bootstrap the Android environment.
#
#   Usage: ./tools/boostrap-android-project.sh 2.32.10
#
# SDL has some boilerplate, as well as the actual SDLActivity class which is the
# heart of what makes the SDL build work. Since we have to obviously hook our
# own stuff in too, this will fetch from the proper SDL version.
#
# Note: This does _NOT_ invoke gradle or anything like that. It is a conscious
# choice on how you choose to invoke gradle, either via ./gradlew or via your
# own locally installed version, or even Android Studio.
#

ROOT=$(git rev-parse --show-toplevel)

IN="SDL-work/android-project"
out="$ROOT/mk/android"
mkdir -p "$out"

function _log
{
    echo -e " \033[32m*\033[0m $*"
}

function _snack
{
    [[ -d "$IN/$1" ]] && mkdir -p "$out/$1" ||
        mv "$IN/$1" "$out/$1"
}

function _cleanup
{
    rm -rf SDL-work
}

# TODO: Get this information from the gradle stuff or store elsewhere
VER=$1
if [[ -z "$1" ]]
then
    echo "Please pass the SDL2 version. (i.e. 2.32.10)"
    exit 1
fi

_log "Cloning SDL ${VER} for android-project!"
git clone https://github.com/libsdl-org/SDL.git -b release-${VER} SDL-work --depth 1
trap _cleanup INT
trap _cleanup TERM

_log "Copying over files..."
_snack "settings.gradle"
_snack "gradlew"
_snack "gradle/wrapper/gradle-wrapper.jar"
_snack "app/proguard-rules.pro"
cp -r "$IN/app/src/main/java" "$out/app/src/main"

_log "Finished bootstraping! You may run gradle in mk/android or use Android Studio."

# mv SDL-work/gradle/

_cleanup
