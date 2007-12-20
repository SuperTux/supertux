#!/bin/sh
DISTDIR="supertux-0.3.2-SVN"
rm -rf $DISTDIR
mkdir $DISTDIR
cp "CMakeLists.txt" "COPYING" "INSTALL" "Jamconfig.in" "Jamfile" "Jamrules" "README" "WHATSNEW.txt" "autogen.sh" "config.h.in" "configure" "configure.ac" "config.h.cmake" "makedist.sh" "makepot.sh" "supertux2.desktop" $DISTDIR
cp --parents mk/autoconf/*.m4 mk/autoconf/config.guess mk/autoconf/config.sub mk/autoconf/install-sh $DISTDIR
cp --parents mk/jam/*.jam $DISTDIR
cp --parents mk/cmake/*.cmake $DISTDIR
cp --parents mk/msvc/* $DISTDIR
find contrib -type f -exec "cp" "--parents" "{}" "$DISTDIR" ";" -o -name .svn -prune
find data -type f -exec "cp" "--parents" "{}" "$DISTDIR" ";" -o -name .svn -prune
find docs -type f -exec "cp" "--parents" "{}" "$DISTDIR" ";" -o -name .svn -prune
find man -type f -exec "cp" "--parents" "{}" "$DISTDIR" ";" -o -name .svn -prune
find src -type f -exec "cp" "--parents" "{}" "$DISTDIR" ";" -o -name .svn -prune
find tools -type f -exec "cp" "--parents" "{}" "$DISTDIR" ";" -o -name .svn -prune
tar czf $DISTDIR.tar.gz $DISTDIR
tar cjf $DISTDIR.tar.bz2 $DISTDIR
