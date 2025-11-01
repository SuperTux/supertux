#!/usr/bin/env bash

shopt -s nullglob

if ([ "$OS_NAME" = "macos-10.15" ] || [ "$OS_NAME" = "macos-13" ]) && [ "$PACKAGE" = "ON" ]; then
    sudo chmod -R +w /usr/local/Cellar

    # Workaround resource busy bug on github on MacOS 13
    # https://github.com/actions/runner-images/issues/7522
    i=0
    until
        sudo cpack -G Bundle;
        
        base_path="/Users/runner/work/supertux/supertux/build/_CPack_Packages/Darwin/Bundle"
        maybe_target=$(find $base_path -type d -depth -mindepth 1 -maxdepth 1)
        directory_name=$(basename $maybe_target)
        echo "Well, here's the example target:"
        echo $maybe_target
        echo "And here's the basename:"
        echo $directory_name

        /usr/bin/hdiutil create -debug -ov -srcfolder $maybe_target -volname "SuperTux v0.6.3-1698-g56492b0c8" -fs "HFS+" -format UDRW "$base_path/temp.dmg"
        sudo fs_usage -t 10 | grep "/dev/disk"

    do
        if [ $i -eq 10 ]; then exit 1; fi
        i=$((i+1))
        sleep 1
    done
fi

# make only one source package
if [ "$SOURCE" = "ON" ]; then
    cpack --config CPackSourceConfig.cmake -G TGZ;
fi

if ([ "$OS_NAME" = "ubuntu-latest" ]) && [ "$PACKAGE" = "ON" ]; then
    ../.ci_scripts/build_appimage.sh
    # extract built appimages for uploading
    mv ~/out/* .

    # CI expects all artifacts to start with "SuperTux-", AppImage generates "SuperTux_v2-...."
    for filename in SuperTux_2-*.AppImage; do 
        [ -f "$filename" ] || continue
        mv "$filename" "${filename//_2/}"
    done
fi

mkdir upload
mv SuperTux* upload/
