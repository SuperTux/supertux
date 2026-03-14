#!/bin/sh

DIR=$(dirname "$0")

generate ()
{
    # TODO: Install in the "correct" direction (not ../../). Just lazy and need
    # to get this working for now.
    printf "$(cat $DIR/org.supertux.template.yml.in)\n" \
           "${1}" \
           "${2}" \
           "${3}" \
        > "$DIR/../../org.supertux.${1}.yml"
}

# I actually felt kind of clever for this. It's nice to actually feel something
# sometimes. It's not even that clever, really.
if [ "$1" == "clean" ]; then
    generate ()
    {
        rm -f "$DIR/../../org.supertux.${1}.yml"
    }
fi

generate SuperTuxNightly supertux-nightly "Off"
generate SuperTux supertux-nightly "On"
