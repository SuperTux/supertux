#!/bin/bash

# Try to source android studio sdk
if [[ -d "$HOME/Android/Sdk/platform-tools" ]]
then
    PATH="$PATH:$HOME/Android/Sdk/platform-tools"
fi

case "$1" in
	install) adb install app/build/outputs/apk/debug/app-debug.apk ;;
    deploy) ;&
    run) adb shell am start -a android.intent.action.MAIN -n org.supertux.supertux2/.MainActivity ;;
    stop) ;&
    kill) adb shell am force-stop org.supertux.supertux2 ;;
	*) echo "Usage: install, run, kill :)"; exit 42 ;;
esac
