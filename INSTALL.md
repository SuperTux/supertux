Install instructions for SuperTux - <http://supertux.lethargik.org/>
====================================================================
Last update: December 20, 2007 by Christoph Sommer

Binaries
--------

We try to provide precompiled binaries of SuperTux for a number of
platforms. You should check <http://supertux.lethargik.org/wiki/Download>
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
<http://supertux.lethargik.org/wiki/Building_SuperTux> for up-to-date
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
* [CMake](http://www.cmake.org/) 2.6 or later: most package managers ship this as `cmake`
* OpenGL headers and libraries: OpenGL libraries and headers are
  specific to your graphics card. Make sure that you have hardware
  accelerated OpenGL drivers installed. Software renderers like Mesa
  will make SuperTux unplayable slow.
* [SDL2](http://www.libsdl.org) (2.0.0 or later)
* [SDL2_image](http://www.libsdl.org/projects/SDL_image) (2.0.0 or later)
* [PhysFS](http://www.icculus.org/physfs) (1.0.x or 2.0.x)
* [OpenAL](http://www.openal.org): There are no official Linux releases
  of OpenAL yet (however, many distributions have OpenAL in their package
  archives), you should grab the latest CVS snapshot (any snapshot from 2005
  should work, earlier snapshots tend to have bugs). OpenAL 1.0 or later
  implementations on other systems like Mac OS X or windows should work.
* C++ OpenGL library (choose one of the two options below):
  - [GLEW](http://glew.sourceforge.net/) or
  - [glbinding](https://github.com/hpicgs/glbinding)
* [Boost's](http://www.boost.org) smart_ptr and format headers
* [cURL](http://curl.haxx.se/libcurl/): for Add-on downloads
* [libogg and libvorbis](https://www.xiph.org/)

**Note I:** for any of the above listed libraries (OpenGL, SDL2, SDL2_image,
PhysFS, OpenAL, GLEW/glbinding, Boost, cURL, libogg and libvorbis), you should
also have development headers installed. Debian-based distributions have `-devel`
packages containing the mentioned headers, on Arch Linux these should be included
in the library package.

**Note II:** We tried to write our code clean, portable and platform neutral,
so it should be possible to compile it on a wide range of platforms and also
with other compilers than gcc or clang. We use [Travis CI](https://travis-ci.org/)
to test commits and pull requests in our repository, but unfortunately it's not
always possible to test the code in very exotic setups. However, feel free to
report issues to our bug tracker on GitHub or to supertux-devel@lists.lethargik.org.

**Note III (regarding glbinding):** To use glbinding instead of GLEW, open
CMakeLists.txt and change this line from
```
OPTION(GLBINDING_ENABLED "Use glbinding instead of GLEW" OFF)
```
to
```
OPTION(GLBINDING_ENABLED "Use glbinding instead of GLEW" ON)
```


Installing under Linux/UNIX using CMake
---------------------------------------

SuperTux uses CMake to generate a set of Makefiles for the build
process. To generate these Makefiles and build SuperTux, perform the
following steps:

1. `cd` to the directory where you unpacked the SuperTux source
   archive, i.e. to the directory containing `src` and `data`.
 
2. If you cloned this Supertux repo using git run `git submodule
   update --init --recursive` to fetch/update squirrel and tinygettext.
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

Note for GIT users: SuperTux does not need to be installed on the
system, you can run it from its own directory.


Installing under Windows using CMake and Visual Studio
------------------------------------------------------
To build SuperTux on Windows with Visual Studio you need to have CMake and a
recent version of Visual Studio installed. Visual Studio 2013 Community Edition
is known to work fine.

Because it's difficult to build and download all the dependencies per hand on windows,
SuperTux provides a [dependency package](https://github.com/SuperTux/downloads/raw/master/other/dependencies-0.3.5-win32.zip)
that should contain all headers and libraries needed to build SuperTux on Windows.

1. Unpack the SuperTux source pack or get the source with git (`git clone --recursive https://github.com/SuperTux/supertux.git`).

2. Extract the [dependency package](https://github.com/SuperTux/downloads/raw/master/other/dependencies-0.3.5-win32.zip)
into the source directory, so the `dependencies` folder is besides the `src` folder.

3. Create a new, empty `build` folder.

4. Open a console window and navigate to the `build` directory.

5. Run `cmake ..` to create the VS solution that builds SuperTux with standard options.
For more CMake options, look at end of the Linux/UNIX build section.

5. Open the new Visual Studio solution `SUPERTUX.sln` in the `build` directory.

6. Build the project.

7. Now you can run SuperTux by directly starting the executable or from within Visual Studio (make sure to set
`supertux2` as StartUp project).


Support
-------

You can contact us at [supertux-devel@lists.lethargik.org](mailto:supertux-devel@lists.lethargik.org)
and in the [#supertux](irc://chat.freenode.net/supertux) channel on
the chat.freenode.net IRC server.
