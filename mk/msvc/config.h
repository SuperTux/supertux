/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Include pthread support for binary relocation? */
//#undef BR_PTHREAD

/* define to compile in debug checks */
#define DEBUG

/* Use binary relocation? */
//#undef ENABLE_BINRELOC

/* define if sqdbg should be enabled */
//#undef ENABLE_SQDBG

/* Use the Apple OpenGL framework. */
//#undef HAVE_APPLE_OPENGL_FRAMEWORK

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
//#undef HAVE_DIRENT_H

/* Define if you have the iconv() function. */
//#undef HAVE_ICONV

/* Define to 1 if you have the <inttypes.h> header file. */
//#undef HAVE_INTTYPES_H

/* Define to 1 if you have the `pthread' library (-lpthread). */
//#undef HAVE_LIBPTHREAD

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `mkdir' function. */
//#define HAVE_MKDIR

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
//#undef HAVE_NDIR_H

/* Define if you have POSIX threads libraries and header files. */
//#undef HAVE_PTHREAD

/* Define to 1 if you have the <stdint.h> header file. */
//#undef HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the <strings.h> header file. */
//#define HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strstr' function. */
#define HAVE_STRSTR 1

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
//#undef HAVE_SYS_DIR_H

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
//#undef HAVE_SYS_NDIR_H

/* Define to 1 if you have the <sys/stat.h> header file. */
//#undef HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/types.h> header file. */
//#undef HAVE_SYS_TYPES_H

/* Define to 1 if you have the <unistd.h> header file. */
//#undef HAVE_UNISTD_H

/* Define to 1 if you have the <windows.h> header file. */
#define HAVE_WINDOWS_H 1

/* Define as const if the declaration of iconv() needs const. */
#define ICONV_CONST const

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "supertux"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "supertux 0.3.2-SVN"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "supertux"

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.3.2-SVN"

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
//#undef PTHREAD_CREATE_JOINABLE

/* The size of a `void *', as computed by sizeof. */
#define SIZEOF_VOID_P 4

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
// #undef WORDS_BIGENDIAN

/* Define to 1 if `lex' declares `yytext' as a `char *' by default, not a
   `char[]'. */
// #undef YYTEXT_POINTER

/* Define to empty if `const' does not conform to ANSI C. */
// #undef const


#if SIZEOF_VOID_P == 8
#define _SQ64
#endif

/* Disable some very annoying warnings */
#pragma warning(disable: 4244)
#pragma warning(disable: 4267)
#pragma warning(disable: 4800)
#pragma warning(disable: 4103)

/* some stuff is not in standard lib in msvc */
#define _CRT_SECURE_NO_DEPRECATE
#define snprintf _snprintf
#define strcasecmp _strcmpi
#define strncasecmp _strncmpi
#define strdup _strdup

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2      (3.14159265358979323846/2.0)
#endif

static inline int roundf(float val)
{
  if(val > 0) {
    return (int) (val + 0.5f);
  } else {
    return (int) (val - 0.5f);
  }
}

// msvc has no sscanf it seems, this breaks demo recording, but who cares...
static inline int sscanf(...)
{
  return -1;
}
