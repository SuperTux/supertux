#!/bin/sh

aclocal -I mk/autoconf
automake --copy --add-missing
autoconf

# EOF #
