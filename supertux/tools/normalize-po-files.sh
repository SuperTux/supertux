#!/bin/sh
find -iname '*.po' | xargs -I xx msgmerge --backup=off --no-fuzzy-matching --no-wrap -U xx messages.pot

