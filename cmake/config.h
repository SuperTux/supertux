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

#define BUILD_DATA_DIR "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/data"

#define BUILD_CONFIG_DATA_DIR "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/cmake/data"

/* #undef ENABLE_DISCORD */

/* #undef UBUNTU_TOUCH */
/* #undef ENABLE_TOUCHSCREEN_SUPPORT */

#endif /*CONFIG_H*/
