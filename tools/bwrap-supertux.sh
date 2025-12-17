#!/bin/sh
#
# Runs the game with bwrap based on which directory you are in.
#
# If you are in $ROOT/build (or anything with CMakeCache.txt in it) then it'll
# attempt to bwrap from the local build, otherwise, it'll use /usr/share/games
# and whatever.
#
# Useful if you need to temporary test stuff out--it won't mess with your local
# data--or if you're paranoid
#
# TODO: X11, audio, EGL(?)

ROOT=$(git rev-parse --show-toplevel)

[[ ! -f $PWD/CMakeCache.txt ]]
IN_BUILD=$?

function run_game()
{
    local BIND_DATA BIND_BIN

    if [[ $IN_BUILD -eq 1 ]]; then
        echo "-- Running local supertux"
        BIND_DATA="$ROOT/data"
        BIND_BIN="./"
    else
        echo "-- Running installed supertux"
        if [[ -d /usr/local/share/games/supertux2 ]]; then
            BIND_DATA="/usr/local/share/games/supertux2/"
            BIND_BIN="/usr/local/games/"
        else
            BIND_DATA="/usr/share/games/supertux2/"
            BIND_BIN="/usr/games/"
        fi
    fi

    echo "BIND_DATA=\"$BIND_DATA\""
    echo "BIND_BIN=\"$BIND_BIN\""

    bwrap --new-session \
          --unshare-all \
          --share-net   \
          --ro-bind /usr /usr \
          --ro-bind /lib /lib \
          --ro-bind /lib64 /lib64 \
          --ro-bind /bin /bin \
          --ro-bind /etc /etc \
          --dev /dev \
          --dev-bind /dev/dri /dev/dri \
          --ro-bind "$BIND_DATA" /share/games/supertux2 \
          --ro-bind "$BIND_BIN" /game \
          --setenv LD_LIBRARY_PATH "$LD_LIBRARY_PATH:/usr/lib64:/game/external/simplesquirrel" \
          --dir "$XDG_RUNTIME_DIR" \
          --ro-bind "$XDG_RUNTIME_DIR/wayland-0" "$XDG_RUNTIME_DIR/wayland-0" \
            ./game/supertux2
}

run_game
