#!/bin/sh
# Wrapper around hdiutil that removes .fseventsd and other system files from DMGs

# Check if this is a 'detach' command - clean up the volume before detaching
if [ "$1" = "detach" ]; then
    VOLUME="$2"
    # Remove macOS system folders that we don't want in the DMG
    rm -rf "${VOLUME}/.fseventsd" 2>/dev/null
    rm -rf "${VOLUME}/.Trashes" 2>/dev/null
    rm -rf "${VOLUME}/.Spotlight-V100" 2>/dev/null
fi

exec /usr/bin/hdiutil "$@"
