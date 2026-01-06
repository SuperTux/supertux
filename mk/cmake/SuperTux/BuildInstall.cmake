if(WIN32 AND NOT UNIX)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.png
                ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.ico
          DESTINATION ".")

  if(MSVC)
    # Install PDB files for use with the error handler.
    install(FILES $<TARGET_PDB_FILE:supertux2>
            DESTINATION ${INSTALL_SUBDIR_BIN}
            OPTIONAL)
  endif()

  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/mk/msvc/run_supertux.bat
                ${CMAKE_CURRENT_SOURCE_DIR}/mk/msvc/run_supertux_portable.bat
          DESTINATION ".")

  option(PACKAGE_VCREDIST "Package the VCREDIST libraries with the program" OFF)

  if(PACKAGE_VCREDIST)
    set(CMAKE_INSTALL_UCRT_LIBRARIES true)
    include(InstallRequiredSystemLibraries)
  endif()

else()
  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND DISABLE_CPACK_BUNDLING)

    set(INFOPLIST_CFBUNDLEEXECUTABLE "supertux2")

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/info.plist.in ${CMAKE_BINARY_DIR}/tools/darwin/info.plist)
    install(FILES ${CMAKE_BINARY_DIR}/tools/darwin/info.plist DESTINATION "SuperTux.app/Contents/")
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/receipt DESTINATION "SuperTux.app/Contents/_MASReceipt/")

    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.png ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.icns DESTINATION "SuperTux.app/Contents/Resources/")

  else()

    if(IS_SUPERTUX_RELEASE)
      set(LINUX_DESKTOP_ICON "supertux2")
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.png ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.xpm DESTINATION "share/pixmaps/")
    else()
      set(LINUX_DESKTOP_ICON "supertux-nightly")
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux-nightly.png ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.xpm DESTINATION "share/pixmaps/")
      # TODO: Is this really needed?
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux-nightly.png DESTINATION "share/icons/hicolor/128x128/apps")
    endif()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/supertux2.desktop.in ${CMAKE_BINARY_DIR}/supertux2.desktop)
    install(FILES ${CMAKE_BINARY_DIR}/supertux2.desktop DESTINATION "share/applications")
    set(APPS "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${INSTALL_SUBDIR_BIN}/supertux2")

    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/supertux2.svg DESTINATION "share/icons/hicolor/scalable/apps")

  endif()
endif()

if(WIN32 AND CMAKE_HOST_WIN32)
  install(TARGETS supertux2
          DESTINATION ${INSTALL_SUBDIR_BIN}
          RUNTIME_DEPENDENCIES PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
                               POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
                               DIRECTORIES $<TARGET_RUNTIME_DLL_DIRS:supertux2>)
else()
  install(TARGETS supertux2
          DESTINATION ${INSTALL_SUBDIR_BIN})
  if(NOT USE_STATIC_SIMPLESQUIRREL)
    install(TARGETS simplesquirrel)
  endif()
endif()

if(EMSCRIPTEN)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mk/emscripten/template.html.in ${CMAKE_CURRENT_BINARY_DIR}/template.html)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mk/emscripten/supertux2.png ${CMAKE_CURRENT_BINARY_DIR}/supertux2.png COPYONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mk/emscripten/supertux2_bkg.png ${CMAKE_CURRENT_BINARY_DIR}/supertux2_bkg.png COPYONLY)
  if(IS_SUPERTUX_RELEASE)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.ico ${CMAKE_CURRENT_BINARY_DIR}/supertux2.ico COPYONLY)
  else()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux-nightly.ico ${CMAKE_CURRENT_BINARY_DIR}/supertux2.ico COPYONLY)
  endif()
endif()

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/README.md
              ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt
              ${CMAKE_CURRENT_SOURCE_DIR}/NEWS.md
        DESTINATION ${INSTALL_SUBDIR_DOC})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/credits.stxt
        DESTINATION ${INSTALL_SUBDIR_SHARE})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/org.supertuxproject.SuperTux.metainfo.xml
        DESTINATION "share/metainfo")

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/images
                  ${CMAKE_CURRENT_SOURCE_DIR}/data/fonts
                  ${CMAKE_CURRENT_SOURCE_DIR}/data/music
                  ${CMAKE_CURRENT_SOURCE_DIR}/data/particles
                  ${CMAKE_CURRENT_SOURCE_DIR}/data/scripts
                  ${CMAKE_CURRENT_SOURCE_DIR}/data/shader
                  ${CMAKE_CURRENT_SOURCE_DIR}/data/speech
                  ${CMAKE_CURRENT_SOURCE_DIR}/data/sounds
                  ${CMAKE_CURRENT_SOURCE_DIR}/data/locale
        DESTINATION ${INSTALL_SUBDIR_SHARE})

if(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/levels
          DESTINATION ${INSTALL_SUBDIR_SHARE}
          PATTERN "data/levels/test" EXCLUDE
          PATTERN "data/levels/test_old" EXCLUDE
          PATTERN "data/levels/incubator" EXCLUDE)
else()
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/levels
          DESTINATION ${INSTALL_SUBDIR_SHARE})
endif()

# move some config clutter to the advanced section
mark_as_advanced(
  INSTALL_SUBDIR_BIN
  INSTALL_SUBDIR_SHARE
  INSTALL_SUBDIR_DOC
)

# EOF #
