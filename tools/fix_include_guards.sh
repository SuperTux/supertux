#!/bin/sh

for i in "$@"; do
    echo $i
    NAME=$(echo $i | sed "s/[\/\.]/_/g;s/^/HEADER_SUPERTUX_/" | tr [a-z] [A-Z])
    sed  -i "s/^#ifndef .*_H_\$/#ifndef $NAME/" $i; 
    sed  -i "s/^#define .*_H_\$/#define $NAME/" $i; 
done

# EOF #
