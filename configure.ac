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
AC_INIT(supertux, 0.2-svn)
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_AUX_DIR([mk/autoconf])
AC_CANONICAL_TARGET

AC_CONFIG_HEADERS(config.h)

# we don't want the stupid autoconf default -g -O2
test ".$CXXFLAGS" = "." && CXXFLAGS=" "

CXXFLAGS="$CXXFLAGS -ffast-math"

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PATH_PROG(AR, ar)
if test "$AR" = "" ; then
  AC_MSG_ERROR([Couldn't find ar])
fi
AR="$AR ru"

AC_C_BIGENDIAN

AC_CHECK_PROGS(XGETTEXT, [xgettext xgettext.exe])
if test -n "$XGETTEXT" ; then
    AC_MSG_CHECKING([if xgettext supports lisp])
    if echo '(gettext "test")' | xgettext -o - -L Lisp - >& /dev/null; then
        AC_MSG_RESULT(yes)
    else
        AC_MSG_RESULT(no)
        AC_MSG_WARN([It seems your gettext installation is too old, try updating it])
        XGETTEXT=""
    fi
fi

AC_PROG_LEX
LEX_LIBS=$LEXLIB
AC_SUBST([LEX_LIBS])
AC_CHECK_PROGS([BISON], [bison bison.exe])
AC_SUBST([BISON])

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_BIGENDIAN
AC_CHECK_SIZEOF([void *])
AH_BOTTOM([
#if SIZEOF_VOID_P == 8
#define _SQ64
#endif
])

dnl ===========================================================================
dnl Give advanced users some options to play with

VARIANT=optimize
AC_MSG_CHECKING([for build variant])

AC_ARG_ENABLE(gprof,
              AC_HELP_STRING([--enable-gprof], [enable GNU profiling support]),
              [enable_gprof=$enableval], [enable_gprof=no])
if test "$enable_gprof" = "yes"; then
    VARIANT=profile
fi

AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug], [enable debugging mode]),
              [enable_debug=$enableval], [enable_debug=no])
if test "$enable_debug" = "yes"; then
    AC_DEFINE([DEBUG], 1, [define to compile in debug checks])
    VARIANT=debug
else
    VARIANT=optimize
fi
AC_SUBST([VARIANT])
AC_MSG_RESULT([$VARIANT])

AC_ARG_ENABLE(sqdbg,
              AC_HELP_STRING([--enable-sqdbg], [enable squirrel remote debugger]),
              [enable_sqdbg=$enableval], [enable_sqdbg=no])
if test "$enable_sqdbg" = "yes"; then
    AC_DEFINE([ENABLE_SQDBG], 1, [define if sqdbg should be enabled])
    AC_SUBST([enable_sqdbg])
fi

AM_ICONV
AC_SUBST([ICONV_LIBS], [$LIBICONV])

dnl ===========================================================================
dnl Check for SDL
SDL_VERSION=1.2.4
AM_PATH_SDL($SDL_VERSION,
            :,
	    AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!]))

XIPH_PATH_VORBIS(, [AC_MSG_ERROR([Please install libvorbis])])
XIPH_PATH_OGG(, [AC_MSG_ERROR([Please install libogg])])

NP_FINDLIB([SDLIMAGE], [SDL_image], [SDL_image >= 1.2],
        NP_LANG_PROGRAM([#include <SDL_image.h>], [IMG_Load("");]),
        [], [-lSDL_image],
        [],
        [AC_MSG_ERROR([Please install SDLImage >= 1.2.1])],
        [$SDL_CFLAGS], [$SDL_LIBS])

NP_FINDLIB([PHYSFS], [physfs], [physfs >= 1.0.0],
        NP_LANG_PROGRAM([
#include <stdio.h>
#include <physfs.h>
#if PHYSFS_VER_MAJOR < 1
# error PHYSFS is too old
#endif]),
        [], [-lphysfs],
        [],
        [AC_MSG_ERROR([Please install physfs >= 1.0])])

NP_FINDLIB([OPENAL], [OpenAL], [OpenAL],
         NP_LANG_PROGRAM([#include <AL/al.h>],
           [alcOpenDevice(0);]),
         [], [-lopenal],
         [],
         [AC_MSG_ERROR([Please intall OpenAL])],
         [], [])

AX_CHECK_GL
if test "$no_gl" = "yes"; then
  AC_MSG_ERROR([Please install opengl libraries and headers])
fi

dnl Checks for library functions.
AC_CHECK_FUNCS(mkdir strdup strstr)

AC_INIT_JAM
AC_CONFIG_FILES([Jamconfig])

AC_OUTPUT

echo ""
echo "       NOTE: This project uses jam (and not make) as build tool"
echo ""

