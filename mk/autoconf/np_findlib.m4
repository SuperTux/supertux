#  NP_FINDLIB(VARNAME, NAME, STRING, TESTAPP, CFLAGS, LIBS, ACTION_IF_FOUND,
#             ACTION_IF_NOT_FOUND, EXTRACFLAGS, EXTRALIBS)
AC_DEFUN([NP_FINDLIB], [
    AC_ARG_WITH([lib$2], [AC_HELP_STRING([--with-lib$2=dir],
        [specify location of lib$2 if not detected automatically; uses
        dir, dir/include and dir/lib])])
            
    save_CFLAGS="$CFLAGS"
    save_CPPFLAGS="$CPPFLAGS"
    save_LIBS="$LIBS"

    RESCFLAGS="$5"
    RESLIBS="$6"
    CFLAGS="$CFLAGS $5 $9"
    CPPFLAGS="$CPPFLAGS $5 $9"
    LIBS="$LIBS $6 $10"

    AS_IF([test -n "$with_lib$2"], [
        CFLAGS="-I$with_lib$2/include $CFLAGS"
        CPPFLAGS="-I$with_lib$2/include $CPPFLAGS"
        LIBS="-L$with_lib$2/lib $LIBS"
        RESCFLAGS="-I$with_lib$2/include $RESCFLAGS"
        RESLIBS="-L$with_lib$2/lib $RESLIBS"
    ])

    AC_MSG_CHECKING([for $3])

    AC_LINK_IFELSE([$4], [buildok=yes], [buildok=no])

    LIBS=$save_LIBS
    CPPFLAGS=$save_CPPFLAGS
    CFLAGS=$save_CFLAGS

    AS_IF([test $buildok = yes],
	[AC_MSG_RESULT([found])
	 $1_AVAILABLE=yes
	 $1_CFLAGS="$RESCFLAGS"
	 $1_LIBS="$RESLIBS"],
	[AC_MSG_RESULT([not found])
	 $1_AVAILABLE=no
	 $1_CFLAGS=""
	 $1_LIBS=""])
    
    AC_SUBST([$1_AVAILABLE])
    AC_SUBST([$1_CFLAGS])
    AC_SUBST([$1_LIBS])

    AS_IF([test $buildok = yes],
 	[ifelse([$7], , :, [$7])],
 	[ifelse([$8], , :, [$8])])
])
