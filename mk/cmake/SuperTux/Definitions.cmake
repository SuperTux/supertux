macro(_add_define tar)
  set(defines ${ARGN})
  foreach(define ${defines})
    if(${define})
      target_compile_definitions(${tar} PUBLIC ${define})
    endif()
  endforeach()
endmacro()

macro(setup_supertux_definitions tar)
  target_compile_definitions(${tar} PUBLIC
    PACKAGE_VERSION="${VERSION_NUMBER_GIT}"
    PACKAGE_NAME="supertux2"
    SUPERTUX_BUILD_NUMBER="${VERSION_NUMBER_GIT}"
    LOGO_FILE="${LOGO_FILE}"
    BUILD_DATA_DIR="${BUILD_DATA_DIR}"
    BUILD_CONFIG_DATA_DIR="${BUILD_CONFIG_DATA_DIR}"
    INSTALL_SUBDIR_BIN="${INSTALL_SUBDIR_BIN}"
    INSTALL_SUBDIR_SHARE="${INSTALL_SUBDIR_SHARE}"
    GLM_ENABLE_EXPERIMENTAL # glm wont work if we dont have this for weird reasons
  )

  _add_define(${tar}
    WORDS_BIGENDIAN
    HAVE_OPENGL
    HAVE_LIBCURL
    ENABLE_SQDBG
    ENABLE_DISCORD
    STEAM_BUILD
    UBUNTU_TOUCH
    HIDE_NONMOBILE_OPTIONS
    REMOVE_QUIT_BUTTON
  )

  if(HAVE_ICONV_CONST)
    target_compile_definitions(${tar} PUBLIC
      HAVE_ICONV_CONST
      ICONV_CONST=const
    )
  else()
    target_compile_definitions(${tar} PUBLIC ICONV_CONST)
  endif()

  get_target_property(defs ${tar} COMPILE_DEFINITIONS)
  list(FILTER defs EXCLUDE REGEX [[^FT_CONFIG_OPTION_USE_HARFBUZZ$]])
  set_property(TARGET ${tar} PROPERTY COMPILE_DEFINITIONS ${defs})
endmacro()
