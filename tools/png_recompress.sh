#!/bin/bash
# $Id$

# Copyright (C) 2007 Arvid Norlander <anmaster AT berlios DOT de>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.

# This script recompressess .png files using pngcrush and
# optipng to get the smallest images. All recompression is
# looseless.
#
# This script needs at least bash3, bash2 will not work
#
# TODO:
#  * Make it work recursivly

if [ -z "$1" ] || [ "$1" == "--help" ]; then
	echo "Usage: $(basename $0) files..."
	echo -e '\e[1mNOTE: Files must be in same directory as the script is run from!\e[0m'
	echo 'Examples:'
	echo "  $0 *.png"
	echo '     this works'
	echo "  $0 */*.png"
	echo '     this does NOT work'
	exit 1
fi

# Check that the tools we use exist:
if ! type pngcrush > /dev/null 2>&1; then
	echo "Can't find pngcrush!"
	echo "This script depends on the pngcrush tool to be in PATH."
	echo "Please install it or, if it is already installed add the"
	echo "directory it is in to PATH and try again."
	exit 1
fi
if ! type optipng > /dev/null 2>&1; then
	echo "Can't find optipng!"
	echo "This script depends on the optipng tool to be in PATH."
	echo "Please install it or, if it is already installed add the"
	echo "directory it is in to PATH and try again."
	exit 1
fi

TMPPATH="$$.png-recompress"

echo -e "Please wait, this can take a \e[1mlong\e[0m time."

echo -e "\n\n\n\e[1mPass 1: pngcrush\e[0m\n\n\n"
for image in "$@"; do
	if [ -d "$image" ]; then continue; fi
	fname=${image##*/}
	dname=`dirname -- "$image"`
	echo -e "\e[1m$image\e[0m : $(du -b $image | awk '{print $1}')"
	newsize="$(pngcrush -reduce -brute -d "$TMPPATH" "$image" | grep -E "filesize reduction")"
	echo "$newsize"
	if [[ $newsize =~ ".+reduction.+" ]]; then
		cp -v "${TMPPATH}/$fname" "$dname/$fname"
	else
		rm -v "${TMPPATH}/$fname"
	fi
	echo
done
rm -rvf "$TMPPATH"

echo -e "\n\n\n\e[1mPass 2: optipng\e[0m\n\n\n"
for image in "$@"; do
	if [ -d "$image" ]; then continue; fi
	fname=${image##*/}
	dname=`dirname -- "$image"`
	echo -e "\e[1m$image\e[0m : $(du -b $image | awk '{print $1}')"
	newsize="$(optipng -i 0 -o 7 -dir "$TMPPATH" "$image" | grep -E '^Output file size')"
	echo "$newsize"
	if [[ $newsize =~ ".+decrease.+" ]]; then
		cp -v "${TMPPATH}/$fname" "$dname/$fname"
	else
		rm -v "${TMPPATH}/$fname"
	fi
	echo
done
rm -rvf "$TMPPATH"
