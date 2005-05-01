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
AC_INIT(supertux, 0.2-cvs)
AC_CONFIG_SRCDIR([src/supertux.cpp])
AC_CONFIG_AUX_DIR([mk/autoconf])
AC_CANONICAL_TARGET

AC_CONFIG_HEADERS(config.h)

# we don't want the stupid autoconf default -g -O2
test ".$CXXFLAGS" = "." && CXXFLAGS=" "

CXXFLAGS="$CXXFLAGS -ffast-math"

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

AC_CHECK_PROGS(XGETTEXT, xgettext)
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

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl ===========================================================================
dnl Give advanced users some options to play with

VARIANT=optimize
AC_MSG_CHECKING(for gprof mode)
AC_ARG_ENABLE(gprof,
              AC_HELP_STRING([--enable-gprof], [enable GNU profiling support]),
              [enable_gprof=$enableval], [enable_gprof=no])
if test "$enable_gprof" = "yes"; then
    #CXXFLAGS="$CXXFLAGS -pg"
    VARIANT=profile
    AC_MSG_RESULT([enabled])
else
    AC_MSG_RESULT([disabled])
fi

AC_MSG_CHECKING(for debug mode)
AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug], [enable debugging mode]),
              [enable_debug=$enableval], [enable_debug=no])
if test "$enable_debug" = "yes"; then
    AC_DEFINE([DEBUG], 1, [define to compile in debug checks])
    VARIANT=debug
    AC_MSG_RESULT([enabled])
else
    VARIANT=optimize
    AC_MSG_RESULT([disabled])
fi
AC_SUBST([VARIANT])

AC_MSG_CHECKING(wether OpenGL should be used)
AC_ARG_ENABLE(opengl,
              AC_HELP_STRING([--disable-opengl], [disable OpenGL support]),
              [enable_opengl=$enableval], enable_opengl=yes)
if test "$enable_opengl" = "yes"; then
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi

iconv_found=no
AC_MSG_CHECKING([for iconv])
AC_TRY_LINK([#include <iconv.h>], [iconv_open(0, 0);], [iconv_found="yes"])
AS_IF([test "$iconv_found" = no],
    LIBS="$LIBS -liconv"
    AC_TRY_LINK([#include <iconv.h>], [iconv_open(0, 0);], [iconv_found="yes"]))
AS_IF([test "$iconv_found" = no],
    AC_MSG_ERROR([Couldn't find iconv library]),
    AC_MSG_RESULT([found]))

dnl ===========================================================================
dnl Check for SDL
SDL_VERSION=1.2.4
AM_PATH_SDL($SDL_VERSION,
            :,
	    AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!]))

NP_FINDLIB([SDLMIXER], [SDL_mixer], [SDL_mixer >= 1.2],
        NP_LANG_PROGRAM([#include <SDL_mixer.h>
#if MIX_MAJOR_VERSION < 1
# error SDLMix too old
#else
# if MIX_MAJOR_VERSION == 1
#  if MIX_MINOR_VERSION < 2
#   error SDLMix too old
#  endif
# endif
#endif], [Mix_OpenAudio(0, 0, 0, 0);]),
        [], [-lSDL_mixer],
        [],
        [AC_MSG_ERROR([Please install SDLMixer >=1.2.2])],
        [$SDL_CFLAGS], [$SDL_LIBS])

NP_FINDLIB([SDLIMAGE], [SDL_image], [SDL_image >= 1.2],
        NP_LANG_PROGRAM([#include <SDL_image.h>], [IMG_Load("");]),
        [], [-lSDL_image],
        [],
        [AC_MSG_ERROR([Please install SDLImage >= 1.2.1])],
        [$SDL_CFLAGS], [$SDL_LIBS])

if test "$enable_opengl" = "yes"; then
  AX_CHECK_GL
fi
if test "$no_gl" = "yes" -o "$enable_opengl" = "no"; then
  GL_AVAILABLE="no"
  AC_DEFINE([NOOPENGL],, [Define if opengl should not be used])
else
  GL_AVAILABLE="yes"
fi
AC_SUBST([GL_AVAILABLE])

dnl Checks for library functions.
AC_CHECK_FUNCS(mkdir strdup strstr)

AC_INIT_JAM
AC_CONFIG_FILES([Jamconfig])

AC_OUTPUT

echo ""
echo "Features:"
echo "========="
echo " Profile Mode:   $enable_gprof"
echo " Debug Mode:     $enable_debug"
echo " OpenGL Support: $enable_opengl"
echo ""
echo ""
echo "       NOTE: This project uses jam (and not make) as build tool"
echo ""

