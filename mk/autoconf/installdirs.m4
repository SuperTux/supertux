#-----------------------------------------------------------------------------
# installdirs.m4 (c) Matze Braun <matze@braunis.de>
# Macros for outputing the installation paths which autoconf gathers into the
# Jamconfig file.
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# AC_OUTPUT_INSTALLDIRS
#   Transforms the installation dirs which are gathered by autoconf and sets
#   properties in the Jamconfig file for them. We deal with stuff like
#   variable references inside the paths (often the paths contain ${prefix})
#   and with correct quoting here.
#   The script will set the INSTALLDIR.PREFIX, INSTALLDIR.EXEC_PREFIX,
#   INSTALLDIR.APPLICATION, INSTALLDIR.SBIN, INSTALLDIR.LIBEXEC,
#   INSTALLDIR.DATA, INSTALLDIR.MAP, INSTALLDIR.CONFIG, INSTALLDIR.SHAREDSTATE
#   INSTALLDIR.LOCALSTATE, INSTALLDIR.PLUGIN, INSTALLDIR.DOC
#   INSTALLDIR.LIBRARY, INSTALLDIR.INCLUDE, INSTALLDIR.OLDINCLUDE,
#   INSTALLDIR.INFO, INSTALLDIR.MAN
#-----------------------------------------------------------------------------
AC_DEFUN([AC_OUTPUT_INSTALLDIRS],[
# Handle the case when no prefix is given. And the special case when a path
# contains more than 2 slashes, these paths seem to be correct but jam fails
# on them.
AS_IF([test $prefix = NONE], [prefix="$ac_default_prefix"],
    [prefix=`echo "$prefix" | sed -e 's:///*:/:g'`])
AS_IF([test $exec_prefix = NONE],
    [exec_prefix="AS_ESCAPE([$(prefix)])"],
    [exec_prefix=`echo "$exec_prefix" | sed -e 's:///*:/:g'`])

prefix=AC_FIX_VARIABLEREF([$prefix])
exec_prefix=AC_FIX_VARIABLEREF([$exec_prefix])
bindir=AC_FIX_VARIABLEREF([$bindir])
sbindir=AC_FIX_VARIABLEREF([$sbindir])
libexecdir=AC_FIX_VARIABLEREF([$libexecdir])
datadir=AC_FIX_VARIABLEREF([$datadir])
sysconfdir=AC_FIX_VARIABLEREF([$sysconfdir])
sharedstatedir=AC_FIX_VARIABLEREF([$sharedstatedir])
localstatedir=AC_FIX_VARIABLEREF([$localstatedir])
libdir=AC_FIX_VARIABLEREF([$libdir])
includedir=AC_FIX_VARIABLEREF([$includedir])
oldincludedir=AC_FIX_VARIABLEREF([$oldincludedir])
infodir=AC_FIX_VARIABLEREF([$infodir])
mandir=AC_FIX_VARIABLEREF([$mandir])

#hack to get the order right :-/ (autoconf --trace reports wrong order...)
AC_SUBST(prefix)
AC_SUBST(exec_prefix)
AC_SUBST(bindir)
AC_SUBST(sbindir)
AC_SUBST(libexecdir)
AC_SUBST(datadir)
AC_SUBST(sysconfdir)
AC_SUBST(sharedstatedir)
AC_SUBST(localstatedir)
AC_SUBST(libdir)
AC_SUBST(includedir)
AC_SUBST(oldincludedir)
AC_SUBST(infodir)
AC_SUBST(mandir)
])

#-----------------------------------------------------------------------------
# AC_FIX_INSTALL
#   Fixes the output from AC_PROG_INSTALL
#-----------------------------------------------------------------------------
AC_DEFUN([AC_FIX_INSTALL], [
AC_REQUIRE([AC_PROG_INSTALL])
INSTALL=AC_FIX_VARIABLEREF([$INSTALL])
INSTALL_PROGRAM=AC_FIX_VARIABLEREF([$INSTALL_PROGRAM])
INSTALL_SCRIPT=AC_FIX_VARIABLEREF([$INSTALL_SCRIPT])
INSTALL_DATA=AC_FIX_VARIABLEREF([$INSTALL_DATA])

# fix for order...
AC_SUBST([INSTALL])
AC_SUBST([INSTALL_PROGRAM])
AC_SUBST([INSTALL_SCRIPT])
AC_SUBST([INSTALL_DATA])
])

#-----------------------------------------------------------------------------
# AC_PREPARE_INSTALLPATH
#   Transform variables of the form ${bla} to $(bla) inside the string and
#   correctly quotes backslashes.
#   This is needed if you want to output some of the paths that autoconf
#   creates to the Jamconfig file.
#-----------------------------------------------------------------------------
AC_DEFUN([AC_FIX_VARIABLEREF],
dnl We need all the strange \\\\ quoting here, because the command will be
dnl inserted into a "" block and sed needs quoting as well
[`echo "$1" | sed -e 's/\${\([[a-zA-Z_][a-zA-Z_]]*\)}/$(\1)/g' -e 's/\\\\/\\\\\\\\/g'`])

