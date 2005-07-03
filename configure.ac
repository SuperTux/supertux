dnl ===========================================================================
dnl    "configure.in"
dnl
dnl current contact:
dnl          SuperTux development team
dnl
dnl original author:
dnl          Duong-Khang NGUYEN
dnl          neoneurone@users.sf.net
dnl ===========================================================================

dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.54])
AC_INIT(SuperTux, 0.1.3)
AC_CONFIG_SRCDIR([src/supertux.cpp])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE(dist-bzip2)

SDL_VERSION=1.2.4

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl ===========================================================================
dnl Give advanced users some options to play with

AC_MSG_CHECKING(for gprof mode)
AC_ARG_ENABLE(gprof,
              AC_HELP_STRING([--enable-gprof], [enable GNU profiling support]),
              [enable_gprof="$enableval"],
              [enable_gprof="no"])
if test "x${enable_gprof}" != "xno"; then
    CXXFLAGS="$CXXFLAGS -pg"
    AC_MSG_RESULT([enabled])
else
    AC_MSG_RESULT([disabled])
fi

AC_MSG_CHECKING(for debug mode)
AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug], [enable debugging mode]),
              [enable_debug="$enableval"],
              [enable_debug="no"])
if test "x${enable_debug}" != "xno"; then
    CXXFLAGS="$CXXFLAGS -Wall -W -DDEBUG -O0 -g3"
    AC_MSG_RESULT([enabled])
else
    AC_MSG_RESULT([disabled])
fi

AC_MSG_CHECKING(wether OpenGL should be used)
AC_ARG_ENABLE(opengl,
              AC_HELP_STRING([--disable-opengl], [disable OpenGL support]),, enable_opengl="yes")
if test "x${enable_opengl}" != "xno"; then
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi

dnl ===========================================================================
dnl Check for SDL
AM_PATH_SDL($SDL_VERSION,
            :,
	    AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!]))
CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

dnl Checks for additional libraries.
AC_CHECK_LIB(SDL_mixer, Mix_OpenAudio,,
	AC_MSG_ERROR([SDL_mixer library required]))

AC_CHECK_LIB(SDL_image, IMG_Load,,
	AC_MSG_ERROR([SDL_image library required]))

if test "x${enable_opengl}" != "xno"; then
  AX_CHECK_GL
fi
if test "x$no_gl" = "xyes" -o "x$enable_opengl" = "xno"; then
  CXXFLAGS="$CXXFLAGS -DNOOPENGL"
  enable_opengl="no"
else
  CFLAGS="$CFLAGS $GL_CFLAGS"
  CXXFLAGS="$CXXFLAGS $GL_CFLAGS"
  LIBS="$LIBS $GL_LIBS"
fi

AC_CHECK_LIB(z, gzopen,, AC_MSG_ERROR([*** zlib is missing]))

CXXFLAGS="$CXXFLAGS -DDATA_PREFIX='\"$datadir/supertux\"'" 

dnl Checks for library functions.
AC_CHECK_FUNCS(mkdir strdup strstr)

AC_OUTPUT(Makefile src/Makefile data/Makefile)

echo ""
echo "Features:"
echo "========="
echo " Profile Mode:   $enable_gprof"
echo " Debug Mode:     $enable_debug"
echo " OpenGL Support: $enable_opengl"
echo ""

# EOF #
