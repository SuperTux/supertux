#!/bin/sh

ST_VERSION=`grep '^SET(SUPERTUX_VERSION' CMakeLists.txt | sed -e 's/SET(SUPERTUX_VERSION "\([^"]\+\)")/\1/'`
DISTDIR="supertux-$ST_VERSION"

if test -e "$DISTDIR"
then
	echo "$DISTDIR already exists."
	exit 1
fi

echo "Creating directory $DISTDIR"
mkdir "$DISTDIR" || exit 1

cp "CMakeLists.txt" "LICENSE" "INSTALL.md" "README.md" "WHATSNEW.txt" "config.h.cmake" "makedist.sh" "makepot.sh" "supertux2.desktop" $DISTDIR
cp --parents mk/cmake/*.cmake $DISTDIR
cp --parents mk/msvc/* $DISTDIR

echo "Copying files:"
for DIR in contrib data docs man src tools external
do
	echo -n "  $DIR ... "
	find "$DIR" -type f -exec "cp" "--parents" "{}" "$DISTDIR" ";" -o -name .svn -prune
	echo "done"
done

echo -n "Creating $DISTDIR.tar.gz ... "
tar czf $DISTDIR.tar.gz $DISTDIR
echo "done"

echo -n "Creating $DISTDIR.tar.bz2 ... "
tar cjf $DISTDIR.tar.bz2 $DISTDIR
echo "done"

echo -n "Removing $DISTDIR ... "
rm -rf $DISTDIR
echo "done"
