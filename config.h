#ifndef CONFIG_H
#define CONFIG_H

#define PACKAGE_NAME "supertux2"

/* #undef ENABLE_SQDBG */

/* #undef ENABLE_BINRELOC */
#define INSTALL_SUBDIR_BIN "games"
#define INSTALL_SUBDIR_SHARE "share/games/supertux2"

#define SIZEOF_VOID_P 8
#if SIZEOF_VOID_P == 8
#define _SQ64
#endif

/* #undef HAVE_ICONV_CONST */
#ifdef HAVE_ICONV_CONST
#define ICONV_CONST const
#else
#define ICONV_CONST
#endif

/* #undef WORDS_BIGENDIAN */

#define HAVE_OPENGL

#define HAVE_LIBCURL

#endif /*CONFIG_H*/
