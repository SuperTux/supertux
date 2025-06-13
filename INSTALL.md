Install instructions for SuperTux - <https://supertux.org/>
====================================================================
Last update: May 4, 2025

Categories:
- [Binaries](#binaries)
  - [Releases](#releases)
  - [Nightlies](#nightlies)
- [Compiling](#compiling)
  - [Requirements](#requirements)
  - [Unix and Unix-like (Linux/MacOS/\*BSD)](#linuxunix-using-cmake)
  - [Windows](#windows-using-cmake-and-visual-studio)

Binaries
--------

### Releases

We try to provide precompiled binaries of SuperTux for a number of
platforms. You should check <https://supertux.org/download.html>
for the packages and instructions on how to install them. If there are
no prebuilt binaries for your platform, then you might still be able
to compile the source code yourself. In this case read the next
sections.

### Nightlies

We also provide binaries automatically generated as code is added to the
repository. These are built using GitHub Actions and are generally used to
validate code quality, but as they produce binaries, those can be downloaded
and installed like any release. You may find the nightlies at
<https://download.supertux.org/>.

Alternatively, if you have a GitHub account, you may find nightlies directly on GitHub
[here](https://github.com/SuperTux/supertux/actions/workflows/); click on any entry in the list on
the right (topmost = most recent), then scroll down to the Artifacts section.
Note that this only works if you are logged in to GitHub.

Compiling
---------

<sub>Tip: You may take inspiration from the [workflow files](.github/workflows).</sub>

### Requirements

To build SuperTux from source, you need to have a number of tools and
libraries installed.

One of these tools is CMake, preferrably with the latest version.

The libraries are listed in the [vcpkg manifest file](vcpkg.json).
- On Linux, install the development package for all of those libraries.
  In debian, these are usually the library name with the prefix `lib` and
  the suffix `-dev`. So, for example, you'd want to install `libvorbis-dev`
  instead of `libvorbis`
- On Windows, you can use vcpkg with the `x64-windows` or `x86-windows` triplet.

**Note:** If you are having issues compiling SuperTux, feel free to
report issues to our bug tracker on GitHub, or contact us on [IRC or Discord](README.md#Community).

For ease of use, here are some installation lines for some Linux distributions:

- Ubuntu 22.xx or later:
  ```
  sudo apt-get update && sudo apt-get install -y cmake build-essential libogg-dev libvorbis-dev libopenal-dev libsdl2-dev libsdl2-image-dev libfreetype6-dev libraqm-dev libcurl4-openssl-dev libglew-dev libharfbuzz-dev libfribidi-dev libglm-dev zlib1g-dev libfmt-dev libsdl2-ttf-dev libphysfs-dev
  ```

- ArchLinux (using sudo, as of August 28th 2024)
  ```
  sudo pacman -S cmake base-devel libogg libvorbis openal sdl2 sdl2_image sdl2_ttf freetype2 libraqm curl openssl glew harfbuzz fribidi glm zlib fmt physfs
  ```

### Linux/UNIX using CMake

SuperTux uses CMake to generate a set of Makefiles for the build
process. To generate these Makefiles and build SuperTux, perform the
following steps:

1. `cd` to the directory where you unpacked the SuperTux source
   archive, i.e. to the directory containing `src` and `data`.

2. If you cloned this Supertux repo using git run `git submodule
   update --init --recursive` to fetch/update submodules.
   (If you got this version of Supertux from an archive,
   submodules should already be included in the package.)

3. Create and change to a new, empty build directory by running `mkdir
   build` and then `cd build`.

4. Run `cmake ..` to create the Makefiles needed to build SuperTux
   with standard options. If you are missing any libraries needed to
   build SuperTux, install those first, then try running CMake again.
   See below for instructions on how to change to standard options.
   - Pass `-DCMAKE_BUILD_TYPE=Release` to build the executable with
     release options.
   - Pass `-GNinja` to use the Ninja build system, which is generally
     faster than traditional Makefiles. You need to have `ninja-build`
     installed in order to do this.

5. Type `cmake --build .` to start the build process.

6. At this point, the game should work by running `./supertux2`
  - If you want to install it to your system, you can run
    `cmake --install .` with superuser privileges.

Here are some SuperTux specific CMake options:

- `-DENABLE_OPENGL=ON`
  : Enabling will integrate OpenGL support (else the game
  will use the SDL renderer),

- `-DBUILD_DOCUMENTATION=ON`
  : Enables building documentation. Requires downloading and installing
  [Doxygen](https://www.doxygen.nl/index.html) as an additional dependency.
  You may build documentation after building SuperTux by running
  "doxygen docs/Doxyfile" from the build directory, and by opening
  "docs/doxygen/html/index.html".

- `-DENABLE_DISCORD=ON`
  : Enables compiling the Discord integration in SuperTux. You may re-disable
  the integration later by replacing "ON" with "OFF".

- `-DWARNINGS=ON`
  : Enable all warnings. The CI enables this option, so it might be useful to
  keep this option activated during development.

- `-DWERROR=ON`
  : Treat all warnings as errors. The CI enables this option, so it might be
  useful to enable this option and to recompile before opening a pull request.

### Windows using CMake and Visual Studio

To build SuperTux on Windows with Visual Studio, you will need:
- An IDE of your choice (optional)
- Visual Studio Community Build Tools (or any edition).
  SuperTux is tested with Visual Studio 17 (2022).
- [CMake](https://cmake.org/download/)
- [vcpkg](https://vcpkg.io)
- [Git](https://git-scm.com/)

Follow the steps to installing vcpkg.

**Note:** If you wish to produce 32-bit builds, replace `x64-windows` with `x86-windows`.

Once dependencies are installed:

1. Unpack the SuperTux source pack or get the source with git (`git clone --recurse-submodules https://github.com/SuperTux/supertux.git`).

2. Create a new, empty `build` folder.

3. Open a console window and navigate to the `build` directory.

5. Run `cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows`.

**Make sure to change the path to vcpkg to the actual path to vcpkg on your system!**

Options:
- `-DCMAKE_BUILD_TYPE=Release` creates a Release build, which does not contain debug information and runs faster. You may also produce `Debug` builds, but these run significantly slower. Other options may be available, such as `RelWithDebInfo` and `MinSizeRel`, but they are not tested. [More info](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html)
- `-DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT/buildsystems/vcpkg.cmake` tells CMake to use the vcpkg toolchain to load dependencies.
- `-DVCPKG_TARGET_TRIPLET=x64-windows` tells vcpkg to use 64-bit dependencies for Windows. If you are compiling 32-bit executables, replace "x64" with "x86".
- Optionally, you may add `-G "Visual Studio 16 2019"` to force a certain version of Visual Studio if multiple are installed on your system. You can also use Ninja with `-GNinja`

For more CMake options, look at end of the Linux/UNIX build section.

5. You may now build SuperTux by running `cmake --build .`.

6. Install the game to a folder by running `cmake --install . .\SuperTux`
   This will create a folder called SuperTux inside the build directory.

7. You can now run the game by going to that SuperTux folder and
   running `run_supertux2.bat`. You can also use `run_supertux2_portable.bat`
   instead to store configuration files inside of the SuperTux installation.

### Other platforms

If you would like to compile SuperTux for a supported platform that isn't listed here,
you can refer to the [workflow files](.github/workflows).
