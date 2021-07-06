Install instructions for SuperTux - <https://supertux.org/>
====================================================================
Last update: September 2, 2016

Binaries
--------

We try to provide precompiled binaries of SuperTux for a number of
platforms. You should check <https://supertux.org/download.html>
for the packages and instructions on how to install them. If there are
no prebuilt binaries for your platform, then you might still be able
to compile the source code yourself. In this case read the next
sections.


Requirements
------------

To build SuperTux from source, you need to have a number of tools and
libraries installed. Note that most of these things should already be
available prepackaged and optimized for your distribution, it is
recommended that you check your distribution first before downloading
from the websites. You can also check
<https://github.com/SuperTux/wiki/blob/master/Building-SuperTux.md> for up-to-date
build instructions for a variety of different platforms and
distributions.

* General development tools:
  - C++ compiler (choose one of the two options below):
    + [gcc compiler suite](http://gcc.gnu.org) version 3.2 or newer (including g++)
    + [LLVM compiler](http://llvm.org/) (you probably want the clang frontend too)
  - [GNU Binutils](http://www.gnu.org/software/binutils) (or the BSD/OS X equivalent)
  - a shell and common POSIX command line tools
  - **Note:** To get these tools, you can install `build-essential` on Debian-based distros,
    `base-devel` on Arch-based distros and the Xcode Command Line tools on OS X.
* [CMake](http://www.cmake.org/) 3.1 or later: most package managers ship this as `cmake`
* OpenGL headers and libraries: OpenGL libraries and headers are
  specific to your graphics card. Make sure that you have hardware
  accelerated OpenGL drivers installed. Software renderers like Mesa
  will make SuperTux unplayable slow.
* [SDL2](http://www.libsdl.org) (2.0.1 or later)
* [SDL2_image](http://www.libsdl.org/projects/SDL_image) (2.0.0 or later)
* [OpenAL](http://www.openal.org): (1.0 or later)
* C++ OpenGL library (choose one of the two options below):
  - [GLEW](http://glew.sourceforge.net/) or
  - [glbinding](https://github.com/hpicgs/glbinding)
* [Boost](http://www.boost.org) smart_ptr and format headers, along with date_time and filesystem libraries
* [cURL](http://curl.haxx.se/libcurl/): for Add-on downloads
* [libogg and libvorbis](https://www.xiph.org/)
* [FreeType](https://www.freetype.org/)
* [GLM](https://github.com/g-truc/glm)
* [libraqm](https://github.com/HOST-Oman/libraqm): optional, but needed
  to display Arabic

**Note I:** for any of the above listed libraries (OpenGL, SDL2, SDL2_image,
OpenAL, GLEW/glbinding, Boost, cURL, libogg and libvorbis), you should
also have development headers installed. Debian-based distributions have `-devel`
packages containing the mentioned headers, on Arch Linux these should be included
in the library package.

**Note II:** We tried to write our code clean, portable and platform neutral,
so it should be possible to compile it on a wide range of platforms and also
with other compilers than gcc or clang. We use [Travis CI](https://travis-ci.org/)
to test commits and pull requests in our repository, but unfortunately it's not
always possible to test the code in very exotic setups. However, feel free to
report issues to our bug tracker on GitHub or to supertux-devel@lists.lethargik.org.

**Note III (regarding glbinding):** To use glbinding instead of GLEW, call `cmake`
with the flag -DGLBINDING_ENABLED=ON

For ease of use, here are some installation lines for some Linux distributions:

- Ubuntu 18.04/20.04:
  ```
  sudo apt-get update && sudo apt-get install -y cmake build-essential libogg-dev libvorbis-dev libopenal-dev libboost-all-dev libsdl2-dev libsdl2-image-dev libfreetype6-dev libraqm-dev libcurl4-openssl-dev libglew-dev libharfbuzz-dev libfribidi-dev libglm-dev
  ```

Installing under Linux/UNIX using CMake
---------------------------------------

SuperTux uses CMake to generate a set of Makefiles for the build
process. To generate these Makefiles and build SuperTux, perform the
following steps:

1. `cd` to the directory where you unpacked the SuperTux source
   archive, i.e. to the directory containing `src` and `data`.

2. If you cloned this Supertux repo using git run `git submodule
   update --init --recursive` to fetch/update squirrel, tinygettext,
   physfs, and some other modules.
   (If you got this version of Supertux from a tarball (.tar), squirrel
   and tinygettext are already in the tarball.)

3. Create and change to a new, empty build directory by running `mkdir
   build`, `cd build`.

4. Run `cmake ..` to create the Makefiles needed to build SuperTux
   with standard options. If you are missing any libraries needed to
   build SuperTux, install those first, then try running CMake again.
   See below for instructions on how to change to standard options.

5. Type `make` to start the build process.

6. Type `make install` to install the programs and any data files and
   documentation. (You should be a root user on Linux systems. You can
   become a root user with the `su` command or by using `sudo make
   install`) Note that there is no uninstall target, so you might wish
   to create a package or other system-specific installation instead.

7. The game should work now and you can remove the source directory.

You can customize the build process by setting additional options for
CMake. The easiest way to do this is to use run `ccmake ..` instead of
`cmake ..` to bring up the curses-based user interface of CMake.
Select an option using the arrow keys, change the selected option by
pressing the Enter key, then hit the `c` (repeatedly, if necessary) to
apply your changes and bring up new options resulting from your newly
set ones. When you are done, press the `g` key to generate a new set
of Makefiles and exit.

Alternatively, you can pass options to `cmake ..` via the command
line. Some common command line switches are:

`-DCMAKE_VERBOSE_MAKEFILE=ON`
: Generates Makefiles that print all commands prior to executing them.

`-Dxxx_LIBRARY=/path/to/library.so -Dxxx_INCLUDE_DIR=/path/to/headerfiles`
: Manually specify the installation directory of a library.

`-DCMAKE_BUILD_TYPE=DEBUG`
: Enables debug mode and compiles extra debug symbols into the SuperTux
executable. This is useful when sending in bug reports to the
developers.

`-DCMAKE_BUILD_TYPE=RELEASE`
: Enables release mode and compiles some sanity checks out of the build.


### Notes for GIT users

SuperTux does not need to be installed on the system;
you can run it from its own directory.

If `ccache` is installed and working,
it can help to signigicantly reduce the (re)compilation time,
e.g. when doing a git bisect with many checkouts.
Furthermore, it is possible to reduce the optimization level to O1,
which may reduce compilation times:
```
cmake .. -DWARNINGS=ON -DCMAKE_CXX_FLAGS="-O1"
```

A general way to reduce compilation time is running `make` with multiple
threads:
```
make -j $(nproc || sysctl -n hw.ncpu || echo 2)
```


Installing under Windows using CMake and Visual Studio
------------------------------------------------------
To build SuperTux on Windows with Visual Studio you need to have CMake and a
recent version of Visual Studio installed. Visual Studio 2015 Community Edition
is known to work fine.

Because it's difficult to build and download all the dependencies per hand on windows,
SuperTux provides a [dependency package](https://download.supertux.org/builddep/)
that should contain all headers and libraries needed to build SuperTux on Windows.

1. Unpack the SuperTux source pack or get the source with git (`git clone --recursive https://github.com/SuperTux/supertux.git`).

2. Extract the [dependency package](https://download.supertux.org/builddep/)
into the source directory, so the `dependencies` folder is besides the `src` folder.

3. Create a new, empty `build` folder.

4. Open a console window and navigate to the `build` directory.

5. Run `cmake ..` to create the VS solution that builds SuperTux with standard options.
For more CMake options, look at end of the Linux/UNIX build section.

5. Open the new Visual Studio solution `SUPERTUX.sln` in the `build` directory.

6. Build the project.

7. Now you can run SuperTux using the run_supertux.bat file
