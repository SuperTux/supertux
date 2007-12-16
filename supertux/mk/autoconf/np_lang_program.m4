# NP_LANG_PROGRAM, custom version of AC_LANG_PROGRAM (because SDL on win32
# NEEDS main(int argc, char** argv)
AC_DEFUN([NP_LANG_PROGRAM],
[$1
m4_ifdef([_AC_LANG_PROGRAM_C_F77_HOOKS], [_AC_LANG_PROGRAM_C_F77_HOOKS])[]dnl
int
main(int argc, char** argv)
{
$2
  ;
  return 0;
}
])
