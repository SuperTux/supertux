#!/bin/bash

########################################################################
# Package the binaries built on Travis-CI as an AppImage
# By Simon Peter 2016
# For more information, see http://appimage.org/
########################################################################

export ARCH=$(arch)

APP=SuperTux
LOWERAPP=supertux2

GIT_REV=$(git rev-parse --short HEAD)
echo $GIT_REV

make install DESTDIR=$HOME/$APP/$APP.AppDir

cd $HOME/$APP/

wget -q https://github.com/probonopd/AppImages/raw/master/functions.sh -O ./functions.sh
. ./functions.sh

cd $APP.AppDir

########################################################################
# Copy desktop and icon file to AppDir for AppRun to pick them up
########################################################################

get_apprun
get_desktop

# Our icon filename doesn't match the binary filename, so we can't use the
# get_icon function here.
find ./usr/share/pixmaps/supertux.png -exec cp {} . \; 2>/dev/null || true
find ./usr/share/icons -path *64* -name supertux.png -exec cp {} . \; 2>/dev/null || true
find ./usr/share/icons -path *128* -name supertux.png -exec cp {} . \; 2>/dev/null || true
find ./usr/share/icons -path *512* -name supertux.png -exec cp {} . \; 2>/dev/null || true
find ./usr/share/icons -path *256* -name supertux.png -exec cp {} . \; 2>/dev/null || true
ls -lh supertux.png || true

########################################################################
# Copy in the dependencies that cannot be assumed to be available
# on all target systems
########################################################################

copy_deps

########################################################################
# Delete stuff that should not go into the AppImage
########################################################################

# Delete dangerous libraries; see
# https://github.com/probonopd/AppImages/blob/master/excludelist
delete_blacklisted

########################################################################
# desktopintegration asks the user on first run to install a menu item
########################################################################

get_desktopintegration $LOWERAPP

########################################################################
# Determine the version of the app; also include needed glibc version
########################################################################

GLIBC_NEEDED=$(glibc_needed)
VERSION=${RELEASE_VERSION}-glibc$GLIBC_NEEDED

########################################################################
# Patch away absolute paths; it would be nice if they were relative
########################################################################

sed -i -e 's|/usr|././|g' usr/bin/supertux2

########################################################################
# AppDir complete
# Now packaging it as an AppImage
########################################################################

cd .. # Go out of AppImage

mkdir -p ../out/
generate_appimage

########################################################################
# Upload the AppDir
########################################################################

transfer ../out/*
