#!/bin/sh

# Correct working directory?
if test ! -f configure.ac ; then
  echo "*** Please invoke this script from directory containing configure.ac."
  exit 1
fi

aclocal -I mk/autoconf
autoheader

# generate Jamconfig.in
cat > Jamconfig.in << __EOF__
# the following unsets some stuff from Jambase
CC = ;
CFLAGS = ;
LINK = ;
LINKFLAGS = ;
AR = ;
# Configuration part (you can customize this)
__EOF__
autoconf --trace=AC_SUBST \
  | sed -e 's/configure.ac:[0-9]*:AC_SUBST:\([^:]*\).*/\1 ?= "@\1@" ;/g' \
  | sed -e 's/.*BACKSLASH.*//' \
  >> Jamconfig.in
echo 'INSTALL ?= "@INSTALL@" ;' >> Jamconfig.in
echo 'JAMCONFIG_READ = yes ;' >> Jamconfig.in

autoconf

