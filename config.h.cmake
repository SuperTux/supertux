#ifndef CONFIG_H
#define CONFIG_H

#define PACKAGE_NAME "supertux2"

#cmakedefine ENABLE_SQDBG

#cmakedefine ENABLE_BINRELOC
#define INSTALL_SUBDIR_BIN "${INSTALL_SUBDIR_BIN}"
#define INSTALL_SUBDIR_SHARE "${INSTALL_SUBDIR_SHARE}"

#define SIZEOF_VOID_P ${SIZEOF_VOID_P}
#if SIZEOF_VOID_P == 8
#define _SQ64
#endif

#cmakedefine HAVE_ICONV_CONST
#ifdef HAVE_ICONV_CONST
#define ICONV_CONST const
#else
#define ICONV_CONST
#endif

#cmakedefine WORDS_BIGENDIAN

#cmakedefine HAVE_OPENGL

#cmakedefine HAVE_LIBCURL

#endif /*CONFIG_H*/
