dnl ===========================================================================
dnl    "configure.in"
dnl
dnl author: Duong-Khang NGUYEN
dnl         neoneurone@users.sf.net
dnl ===========================================================================

dnl Process this file with autoconf to produce a configure script.

AC_INIT(SuperTux, 0.0.6-cvs)
AC_CONFIG_SRCDIR([src/supertux.c])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE

SDL_VERSION=1.2.5

AC_PROG_CC
AC_PROG_INSTALL
AC_LANG(C)

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
              AC_HELP_STRING([--enable-gprof], [enable GNU profiling support],, enable_gprof="no"))
if test "x${enable_gprof}" != "xno"; then
    CFLAGS="$CFLAGS -pg"
    AC_MSG_RESULT([enabled])
else
    AC_MSG_RESULT([disabled])
fi

AC_MSG_CHECKING(for debug mode)
AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug], [enable debugging mode],, enable_debug="no"))
if test "x${enable_debug}" != "xno"; then
    AC_MSG_RESULT([enabled])
else
    AC_MSG_RESULT([disabled])
fi

dnl ===========================================================================
dnl Check for SDL
AM_PATH_SDL($SDL_VERSION,
            :,
	    AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!]))

CFLAGS="$CFLAGS $SDL_CFLAGS"
LDFLAGS="$LDFLAGS $SDL_LIBS"

dnl Checks for additional libraries.
AC_CHECK_LIB(SDL_mixer, SDL_OpenAudio, ,AC_MSG_ERROR(SDL_mixer library required))
AC_CHECK_LIB(SDL_image, IMG_Load, ,AC_MSG_ERROR(SDL_image library required))
AC_CHECK_LIB(GL, glBegin, ,AC_MSG_ERROR(GL library required))

# FIXME: Evil
CFLAGS="-DDATA_PREFIX=\\\"$PWD/data\\\" -DLINUX"

dnl Checks for library functions.
AC_CHECK_FUNCS(mkdir strdup strstr)

AC_OUTPUT(Makefile src/Makefile data/Makefile)

# EOF #
