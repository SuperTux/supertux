#ifndef CONFIG_H
#define CONFIG_H

#define PACKAGE_NAME "supertux2"

/* #undef ENABLE_SQDBG */

#define INSTALL_SUBDIR_BIN "games"
#define INSTALL_SUBDIR_SHARE "share/games/supertux2"

/* #undef HAVE_ICONV_CONST */
#ifdef HAVE_ICONV_CONST
#define ICONV_CONST const
#else
#define ICONV_CONST
#endif

/* #undef WORDS_BIGENDIAN */

#define HAVE_OPENGL
/* #undef USE_OPENGLES2 */
/* #undef USE_OPENGLES1 */

/* #undef HAVE_LIBCURL */

#define BUILD_DATA_DIR "/home/runner/work/supertux/supertux/data"

#define BUILD_CONFIG_DATA_DIR "/home/runner/work/supertux/supertux/_codeql_build_dir/data"

/* #undef ENABLE_DISCORD */
/* #undef STEAM_BUILD */

/* #undef UBUNTU_TOUCH */
/* #undef HIDE_NONMOBILE_OPTIONS */

/* #undef REMOVE_QUIT_BUTTON */

#endif /*CONFIG_H*/
