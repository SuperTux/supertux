#----------------------------------------------------------------------------
#  AC_INIT_JAM
#    This rule fixes several issues related to autoconf being make centric
#----------------------------------------------------------------------------
AC_DEFUN([AC_INIT_JAM],
    [
     AC_INIT_JAMFILE
     AC_OUTPUT_INSTALLDIRS
     AC_FIX_INSTALL])

#----------------------------------------------------------------------------
#  AC_INIT_JAMFILE
#    This rule let's config.status create a wrapper Jamfile in case configure
#    has been invoked from a directory outside the source directory
#----------------------------------------------------------------------------
AC_DEFUN([AC_INIT_JAMFILE],
    [AC_CONFIG_COMMANDS([Jamfile],
      [AS_IF([test ! -f "${ac_top_builddir}Jamfile"],
	[echo Installing Jamfile wrapper.
	 echo "# This file was automatically create by config.status" > Jamfile
	 echo "TOP ?= $ac_top_srcdir ;" >> Jamfile
	 echo "top_builddir ?= . ;" >> Jamfile
	 echo "include \$(TOP)/Jamfile ;" >> Jamfile])])])

