#!/bin/sh

aclocal-1.7 -I mk/autoconf
automake-1.7 --copy --add-missing
autoconf

# EOF #
