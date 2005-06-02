#!/bin/sh

# Correct working directory?
if test ! -f configure.ac ; then
  echo "*** Please invoke this script from directory containing configure.ac."
  exit 1
fi

aclocal -I mk/autoconf
autoheader

# generate Jamconfig.in
autoconf --trace=AC_SUBST \
  | sed -e 's/configure.ac:[0-9]*:AC_SUBST:\([^:]*\).*/\1 ?= "@\1@" ;/g' \
  > Jamconfig.in
sed -e 's/.*BACKSLASH.*//' -i~ Jamconfig.in
rm Jamconfig.in~
echo 'INSTALL ?= "@INSTALL@" ;' >> Jamconfig.in
echo 'JAMCONFIG_READ = yes ;' >> Jamconfig.in

autoconf

