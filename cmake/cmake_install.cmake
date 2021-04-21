# Install script for directory: /media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications" TYPE FILE FILES "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/cmake/supertux2.desktop")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/pixmaps" TYPE FILE FILES
    "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/data/images/engine/icons/supertux.png"
    "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/data/images/engine/icons/supertux.xpm"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps" TYPE FILE FILES "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/supertux2.svg")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/games/supertux2" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/games/supertux2")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/games/supertux2"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/games" TYPE EXECUTABLE FILES "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/cmake/supertux2")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/games/supertux2" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/games/supertux2")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/games/supertux2"
         OLD_RPATH "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/cmake/SDL_ttf/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/games/supertux2")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/supertux2" TYPE FILE FILES
    "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/INSTALL.md"
    "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/README.md"
    "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/LICENSE.txt"
    "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/NEWS.md"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/games/supertux2" TYPE FILE FILES "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/data/credits.stxt")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/metainfo" TYPE FILE FILES "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/supertux2.appdata.xml")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/games/supertux2" TYPE DIRECTORY FILES
    "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/data/images"
    "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/data/fonts"
    "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/data/music"
    "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/data/particles"
    "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/data/scripts"
    "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/data/shader"
    "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/data/speech"
    "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/data/sounds"
    "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/data/locale"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/games/supertux2" TYPE DIRECTORY FILES "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/data/levels" REGEX "/data\\/levels\\/misc\\/menu\\.stl\\.in$" EXCLUDE)
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/games/supertux2/levels/misc" TYPE FILE FILES "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/cmake/data/levels/misc/menu.stl")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/media/kuba/eb6790e7-7282-439d-88d1-8294732abb36/Collaboration/supertux/cmake/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
