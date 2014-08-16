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

* Linux development tools (your distribution should come with these):
  - gcc compiler suite version 3.2 or newer (you need the c++ compiler
    g++) <http://gcc.gnu.org>
  - gnu binutils <http://www.gnu.org/software/binutils>
  - a shell and POSIX commandline tools
* CMake 2.6 or later:
  [Download](http://www.cmake.org/HTML/Download.html);
  [Homepage](http://www.cmake.org/)
* OpenGL headers and libraries: OpenGL libraries and headers are
  specific to your graphics card. Make sure that you have hardware
  accelerated OpenGL drivers installed. Software renderers like Mesa
  will make SuperTux unplayable slow.
* SDL 1.2.5 or later (1.2.8 is recommended on Mac OS X)
  <http://www.libsdl.org>
* SDL_image (any version that includes PNG and JPEG support)
  <http://www.libsdl.org/projects/SDL_image>
* PhysicsFS (1.0.x or 2.0.x) <http://www.icculus.org/physfs>
* OpenAL: There are no official Linux releases of OpenAL yet, you
  should grab the latest CVS snapshot (any snapshot from 2005 should
  work, earlier snapshots tend to have bugs). OpenAL 1.0 or later
  implementations on other systems like Mac OS X or windows should
  work. <http://www.openal.org>
* GLEW <http://glew.sourceforge.net/>
* Boost's smart_ptr and format headers <http://www.boost.org/>

Note: We tried to write our code clean, portable and platform neutral,
so it should be possible to compile it on a wide range of platforms
and also with other compilers than gcc. However we have no resources
to test other setups and it is likely that you hit small problems.
Reports and binary compiled packages are of course always welcome.
Send them to supertux-devel@lists.lethargik.org


Installing under Linux/UNIX using CMake
---------------------------------------

SuperTux uses CMake to generate a set of Makefiles for the build
process. To generate these Makefiles and build SuperTux, perform the
following steps:

1. `cd' to the directory where you unpacked the SuperTux source
   archive, i.e. to the directory containing `src' and `data'.

2. Create and change to a new, empty build directory by running `mkdir
   build', `cd build'.

3. Run `cmake ..' to create the Makefiles needed to build SuperTux
   with standard options. If you are missing any libraries needed to
   build SuperTux, install those first, then try running CMake again.
   See below for instructions on how to change to standard options.

4. Type `make' to start the build process.

5. Type `make install' to install the programs and any data files and
   documentation. (You should be a root user on Linux systems. You can
   become a root user with the `su' command or by using `sudo make
   install') Note that there is no uninstall target, so you might wish
   to create a package or other system-specific installation instead.
   
6. The game should work now and you can remove the source directory.

You can customize the build process by setting additional options for
CMake. The easiest way to do this is to use run `ccmake ..` instead of
`cmake ..` to bring up the curses-based user interface of CMake.
Select an option using the arrow keys, change the selected option by
pressing the Enter key, then hit the `c` (repeatedly, if necessary) to
apply your changes and bring up new options resulting from your newly
set ones. When you are done, press the `g` key to generate a new set
of Makefiles and exit.

Alternatively, you can pass options to `cmake ..' via the command
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


Support
-------

You can contact us at [supertux-devel@lists.lethargik.org](mailto:supertux-devel@lists.lethargik.org)
and in the [#supertux](irc://chat.freenode.net/supertux) channel on
the chat.freenode.net IRC server.
