#!/bin/sh

find data/ -name '*.png' -exec sh -c '
	(magick identify {} 2>&1 | grep -q iCCP) && 
	 echo libpng warning from {}, fixing! &&
	 magick {} {} 2>&1 &&
	 echo Fixed! || printf .
' \;
echo
