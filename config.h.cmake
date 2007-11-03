#define PACKAGE_NAME "supertux"
#define PACKAGE_VERSION "0.3.0"

#cmakedefine DEBUG
#cmakedefine ENABLE_SQDBG

#cmakedefine ENABLE_BINRELOC
#define APPDATADIR "${APPDATADIR}"

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
