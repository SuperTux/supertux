#!/bin/sh

# we need a minimum of automake 1.6 and automake 1.8 seems to be buggy
export WANT_AUTOMAKE=1.6

aclocal -I mk/autoconf
automake --copy --add-missing
autoconf

# EOF #
