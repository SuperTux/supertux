dnl ===========================================================================
dnl    "configure.in"
dnl
dnl author: Duong-Khang NGUYEN
dnl         neoneurone@users.sf.net
dnl ===========================================================================

dnl Process this file with autoconf to produce a configure script.

AC_INIT(SuperTux, 0.0.7-cvs)
AC_CONFIG_SRCDIR([src/supertux.cpp])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE

SDL_VERSION=1.2.4

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_LANG(C++)

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
              AC_HELP_STRING([--enable-gprof], [enable GNU profiling support]),, enable_gprof="no")
if test "x${enable_gprof}" != "xno"; then
    CXXFLAGS="$CXXFLAGS -pg"
    AC_MSG_RESULT([enabled])
else
    AC_MSG_RESULT([disabled])
fi

AC_MSG_CHECKING(for debug mode)
AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug], [enable debugging mode]),, enable_debug="yes")
if test "x${enable_debug}" != "xno"; then
    CXXFLAGS="$CXXFLAGS -DDEBUG"
    AC_MSG_RESULT([enabled])
else
    AC_MSG_RESULT([disabled])
fi

AC_MSG_CHECKING(weather OpenGL should be used)
AC_ARG_ENABLE(opengl,
              AC_HELP_STRING([--disable-opengl], [disable OpenGL support]),, enable_opengl="yes")
if test "x${enable_opengl}" != "xno"; then
    CXXFLAGS="$CXXFLAGS -D__OPENGL"
    AC_MSG_RESULT([yes])
else
    CXXFLAGS="$CXXFLAGS -DNOOPENGL"
    AC_MSG_RESULT([no])
fi

AC_MSG_CHECKING(weather Sound should be supported)
AC_ARG_ENABLE(sound,
              AC_HELP_STRING([--disable-sound], [disable sound support]),, enable_sound="yes")
if test "x${enable_sound}" != "xno"; then
    CXXFLAGS="$CXXFLAGS -D__SOUND"
    AC_MSG_RESULT([yes])
else
    CXXFLAGS="$CXXFLAGS -DNOSOUND"
    AC_MSG_RESULT([no])
fi

dnl ===========================================================================
dnl Check for SDL
AC_LANG_PUSH(C)
AM_PATH_SDL($SDL_VERSION,
            :,
	    AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!]))
CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
LDFLAGS="$LDFLAGS $SDL_LIBS"

dnl Checks for additional libraries.
if test "x${enable_sound}" != "xno"; then
  AC_CHECK_LIB(SDL_mixer, SDL_OpenAudio,, 
  [CXXFLAGS="$CXXFLAGS -DNOSOUND"
   enable_sound="no"])
fi

AC_CHECK_LIB(SDL_image, IMG_Load, ,AC_MSG_ERROR(SDL_image library required))

if test "x${enable_opengl}" != "xno"; then
  # grumbel: xlib path is needed on my system to get OpenGL right,
  # else I end up with a black screen, might be a debian issue or a
  # local issue on my system, not sure
  AC_PATH_XTRA
  CXXFLAGS="$CXXFLAGS $X_CFLAGS"
  LDFLAGS="$LDFLAGS $X_LIBS"

  AC_CHECK_LIB(GL, glBegin,, 
  [CXXFLAGS="$CXXFLAGS -DNOOPENGL"
   enable_opengl="no"])
fi
AC_LANG_POP()

AC_CHECK_LIB(z, gzopen,, AC_MSG_ERROR([*** zlib is missing]))

# FIXME: Evil
CXXFLAGS="$CXXFLAGS -DDATA_PREFIX='\"$datadir/supertux/\"' -DLINUX"

dnl Checks for library functions.
AC_CHECK_FUNCS(mkdir strdup strstr)

AC_OUTPUT(Makefile src/Makefile data/Makefile)

echo ""
echo "Features:"
echo "========="
echo " Profile Mode:   $enable_gprof"
echo " Debug Mode:     $enable_debug"
echo " OpenGL Support: $enable_opengl"
echo " Sound Support:  $enable_sound"
echo ""

# EOF #
