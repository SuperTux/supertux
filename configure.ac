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
#AC_CHECK_LIB(smpeg, SMPEG_status)


AC_ARG_ENABLE(silence,
[  --enable-silence          no sound [default=no]],
              , enable_silence=no)
if test x$enable_silence = xyes; then
    CXXFLAGS="$CXXFLAGS -DNOSOUND"
else
AC_CHECK_LIB(SDL_mixer, Mix_OpenAudio,,
	AC_MSG_ERROR([SDL_mixer library required]))
AC_DEFINE(HAVE_SOUND, 1, [Compile with sound support])
fi
AM_CONDITIONAL(HAVE_SOUND, test "x$enable_silence" != "xyes")

# compile for the gp2x
AC_ARG_ENABLE(gp2x,
[  --enable-gp2x          Build the gp2x version [default=no]],
              , enable_gp2x=no)
if test x$enable_gp2x = xyes; then
    CXXFLAGS="$CXXFLAGS -DGP2X"
if test x$enable_silence = xno; then
  AM_PATH_LIBMIKMOD(,:,AC_MSG_ERROR([*** mikmod not found!]))
  CXXFLAGS="$CXXFLAGS $LIBMIKMOD_CFLAGS"
  CFLAGS="$CFLAGS $LIBMIKMOD_CFLAGS"
  LIBS="$LIBS $LIBMIKMOD_LIBS"

  AC_CHECK_LIB(mikmod, MikMod_Init)
fi
  AC_CHECK_LIB(SDL_gfx, rotozoomSurface,,
	AC_MSG_ERROR([SDL_gfx library required]))
fi

# compile for the testing the 320x240 resolution
AC_ARG_ENABLE(320x240,
[  --enable-320x240          Test the 320x240 resolution [default=no]],
              , enable_320x240=no)
if test x$enable_320x240 = xyes; then
    CXXFLAGS="$CXXFLAGS -DRES320X240"
fi

# Enable touchscreen controls
AC_ARG_ENABLE(touchscreen,
[  --enable-touchscreen          Touchscreen Controls [default=no]],
              , enable_touchscreen=no)
if test x$enable_touchscreen = xyes; then
    CXXFLAGS="$CXXFLAGS -DTSCONTROL"
fi

AC_CHECK_LIB(jpeg,jpeg_abort)

AC_CHECK_LIB(png,png_free)

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


# compile static
AC_ARG_ENABLE(static,
[  --enable-static          Build the static version [default=no]],
              , enable_static=no)
if test x$enable_static = xyes; then
    CXXFLAGS="$CXXFLAGS -static"
fi


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
echo " 320x240:        $enable_320x240"
echo " GP2X Build:     $enable_gp2x"
echo " Static Build:   $enable_static"
echo " Silence:        $enable_silence"
echo " Touchscreen:    $enable_touchscreen"
echo ""

# EOF #
