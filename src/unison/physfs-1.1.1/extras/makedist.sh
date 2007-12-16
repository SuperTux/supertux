#!/bin/sh

# This shell script is roughly equivalent to what "make dist" did in the
#  autotools build system and is called from a custom CMake target.

# !!! FIXME: This code sort of sucks. Consider using CPack instead...

if [ ! -f ./CMakeLists.txt ]; then
    echo "you are in the wrong place."
    exit 1
fi

if [ -z "$1" ]; then
    echo "Wrong arguments."
    exit 2
fi

set -e

VERSION="$1"
BASENAME="physfs-$VERSION"
TARBALL="$BASENAME.tar.gz"
TMPCPDIR="../9sdkujy75jv932-physfstmp-$VERSION"
CPDIR="$TMPCPDIR/$BASENAME"

echo "Packing PhysicsFS $VERSION source tarball..."
echo " + Setting up scratch dir..."
rm -rf $TMPCPDIR
mkdir $TMPCPDIR
mkdir $CPDIR

echo " + Making copy of source tree in scratch dir..."
cp -R . $CPDIR/
echo " + Deleting cruft..."
pushd $CPDIR >/dev/null
rm -rf `svn propget svn:ignore .`
rm -rf `svn status |grep '?' |sed -s 's/\?//'`
popd >/dev/null
rm -rf `find $CPDIR -type d -name '.svn'`
echo " + Deleting Subversion metadata..."
rm -rf `find $CPDIR -type d -name '.svn'`
echo " + Fixing up permissions..."
chmod -R a+rw $CPDIR
chmod a+x `find $CPDIR -type d`
echo " + Building final tarball..."
rm -f $TARBALL
tar -czf $TARBALL -C $TMPCPDIR $BASENAME
echo " + Cleaning up..."
rm -rf $TMPCPDIR
echo " + All done! Packed to '$TARBALL' ..."
set +e

exit 0

