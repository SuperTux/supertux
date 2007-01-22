#!/bin/bash
# $Id$
# This script recompressess .png files using pngcrush and
# optipng to get the smallest images. All recompression is
# looseless.
#
# NOTE: files must be in same directory as the script is run from!
# Examples:
#  ~/supertux/tools/image_recompress.sh *.png
#   (good)
#  ~/supertux/tools/image_recompress.sh */*.png
#   (bad, does not work)
#
# TODO:
#  * Make it work recursivly
#  * Make it work on files not in current directory

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


echo -e "Please wait, this can take a \e[1mlong\e[0m time."

echo -e "\n\n\n\e[1mPass 1: pngcrush\e[0m\n\n\n"
for image in "$@"; do
	echo -e "\e[1m$image\e[0m : $(du -b $image | awk '{print $1}')"
	newsize="$(pngcrush -reduce -brute -d c "$image" | grep -E "filesize reduction")"
	if [[ $newsize =~ ".+reduction.+" ]]; then
		echo "$newsize"
		cp -v "c/$image" "$image"
	fi
	echo
done
rm -rvf c

echo -e "\n\n\n\e[1mPass 2: optipng\e[0m\n\n\n"
for image in "$@"; do
	echo -e "\e[1m$image\e[0m : $(du -b $image | awk '{print $1}')"
	newsize="$(optipng -i 0 -o 7 -dir out "$image" | grep -E '^Output file size')"
	echo "$newsize"
	if [[ $newsize =~ ".+decrease.+" ]]; then
		cp -v "out/$image" "$image"
	else
		rm -v "out/$image"
	fi
	echo
done
rm -rvf out
