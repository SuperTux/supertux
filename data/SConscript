import os
Import('*')

if 'install' in BUILD_TARGETS:
    patterns = ["*.txt", "levels/*/*.stl", "levels/*/*.stwm", "levelts/*/info",
                "sounds/*.wav", "music/*.mod", "music/*.ogg",
                "images/*/*.jpg", "images/*/*.png", "images/*/*/*.png",
		"images/*.xpm", "images/*.png", "images/*.strf",
		"images/tilesets/*.stgt"]

    files = []
    for pattern in patterns:
        files = files + Glob(pattern)

    InstallData(files)
