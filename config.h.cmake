#ifndef CONFIG_H
#define CONFIG_H

#define PACKAGE_NAME "supertux"
#define PACKAGE_VERSION "0.3.2-SVN${REVISION}"

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

#cmakedefine HAVE_LIBCURL

//Useful constants
static const float DELTA = .0001f;

// the engine will be run with a logical framerate of 64fps.
// We chose 64fps here because it is a power of 2, so 1/64 gives an "even"
// binary fraction...
static const float LOGICAL_FPS = 64.0;


#endif /*CONFIG_H*/
