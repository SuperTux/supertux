#!/bin/sh

./tinygettext_test translate po/fr.po  "invalid"
./tinygettext_test directory po/ umlaut Deutsch
./tinygettext_test directory po/ umlaut deutsch
./tinygettext_test directory po/ umlaut de

# EOF #
