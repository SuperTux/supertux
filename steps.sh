#!/bin/bash

# =============================================================================
#   WARNING: THIS BUILD IS BUGGY!
#
# This will get to compile SuperTux for WASM properly, but upon running it'll
# crash with an error. This is to help investigate why the error happens.
# =============================================================================

# Create a VM with a fresh Ubuntu 20.04 install, then run these steps:

# Rough overview:
# - Install git
# - Download and setup the emscripten tools
# - Download and install vcpkg
# - Install some dependencies with vcpkg
# - Download the source code of SuperTux
# - Apply some patches for WASM compatibility
# - Build SuperTux for WASM
# - Run SuperTux; it'll end with a browser window open

set -e
cd ~

# Install git
sudo apt-get install -y git cmake build-essential curl

# Download and setup emscripten tools
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
cd ..

# Download vcpkg
git clone https://github.com/microsoft/vcpkg
./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg integrate install

# Install dependencies
./vcpkg/vcpkg install boost-date-time:wasm32-emscripten
./vcpkg/vcpkg install boost-filesystem:wasm32-emscripten
./vcpkg/vcpkg install boost-format:wasm32-emscripten
./vcpkg/vcpkg install boost-locale:wasm32-emscripten
./vcpkg/vcpkg install boost-optional:wasm32-emscripten
./vcpkg/vcpkg install boost-system:wasm32-emscripten
./vcpkg/vcpkg install glbinding:wasm32-emscripten
./vcpkg/vcpkg install libpng:wasm32-emscripten
./vcpkg/vcpkg install libogg:wasm32-emscripten
./vcpkg/vcpkg install libvorbis:wasm32-emscripten
./vcpkg/vcpkg install openal-soft:wasm32-emscripten

# Downolad the SuperTux source code and apply patches
git clone --recursive https://github.com/SuperTux/supertux.git
cd supertux
git fetch origin wasm-buggy
git checkout wasm-buggy
cd external/SDL_ttf
git apply ../../SDL_ttf-wasm-DIRTY.patch
cd ../../

# Build SuperTux
mkdir build
cd build
emcmake cmake -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=wasm32-emscripten -DGLBINDING_ENABLED=ON .. || true
# rerun cmake because it bugs sometimes
emcmake cmake ..
# TODO: Pollutes the source, make it pollute the build instead
rsync -aP data/ ../data/
emmake make -j$(nproc)

# Run
emrun supertux2.html
