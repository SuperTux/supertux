#
# SuperTux - root build script
# Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#


#
# INSTRUCTIONS:
# -------------
#
# Create a directory build/ and change to it. Run
#
#   cmake ..
#
# This creates a set of Makefiles to build the project. Run
#
#   make
#


## Project name to use as command prefix.

PROJECT(SUPERTUX)

## Version, for display in title bar.
## Will add 'commit <hash>' to the end if git is available and this is a git repository.
SET(SUPERTUX_VERSION "0.3.6-GIT")

### CMake configuration

# allow building with CMake 2.6. Set compatibility options. Use some bundled modules as a fallback
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.1)
IF(COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 NEW)
  CMAKE_POLICY(SET CMP0008 NEW)
  IF(POLICY CMP0043)
    CMAKE_POLICY(SET CMP0043 OLD)
  ENDIF(POLICY CMP0043)
ENDIF(COMMAND cmake_policy)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/mk/cmake)

## For autopackage
SET(APPDATADIR "${CMAKE_INSTALL_PREFIX}/share/games/supertux2")

## Check endianess

INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

## These variables enable MSVC to find libraries located in "dependencies"
IF(WIN32)
  SET(ENV{PATH} "$ENV{PATH};${PROJECT_SOURCE_DIR}/dependencies/include")
  SET(ENV{LIB} ${PROJECT_SOURCE_DIR}/dependencies/lib)
  SET(ENV{OPENALDIR} ${PROJECT_SOURCE_DIR}/dependencies)

  SET_PROPERTY(DIRECTORY PROPERTY COMPILE_DEFINITIONS_DEBUG DEBUG)
  ## Enable multi-processor compilation (faster)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  ## And shut up about unsafe stuff
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  ## Add an icon
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/mk/msvc/icon_rc.template" "${PROJECT_BINARY_DIR}/tmp/icon.rc")

  ## Find all used libraries
  FILE(GLOB DLLS "${PROJECT_SOURCE_DIR}/dependencies/dll/*")
ENDIF(WIN32)

## Add lots of dependencies to compiler switches

SET(Boost_ADDITIONAL_VERSIONS "1.41" "1.41.0")
FIND_PATH(PHYSFS_INCLUDE_DIR NAMES physfs.h PATHS "${PROJECT_SOURCE_DIR}/dependencies/include/physfs")
FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

IF(WIN32)
  FIND_PATH(SDL2_INCLUDE_DIRS NAMES SDL.h PATHS "${PROJECT_SOURCE_DIR}/dependencies/include/SDL2")
  FIND_PATH(SDL2IMAGE_INCLUDE_DIRS NAMES SDL_image.h PATHS "${PROJECT_SOURCE_DIR}/dependencies/include/SDL2_image")
  FIND_LIBRARY(SDL2_LIBRARIES NAMES SDL2 PATHS "${PROJECT_SOURCE_DIR}/dependencies/lib")
  FIND_LIBRARY(SDL2IMAGE_LIBRARIES NAMES SDL2_image PATHS "${PROJECT_SOURCE_DIR}/dependencies/lib")
ELSE(WIN32)
  INCLUDE(FindPkgConfig)
  PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2>=2.0.1)
  PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
ENDIF(WIN32)
SET(HAVE_SDL TRUE)

OPTION(ENABLE_OPENGL "Enable OpenGL support" ON)
OPTION(GLBINDING_ENABLED "Use glbinding instead of GLEW" OFF)
OPTION(GLBINDING_DEBUG_OUTPUT "Enable glbinding debug output for each called OpenGL function" OFF)
IF(ENABLE_OPENGL)
  FIND_PACKAGE(OpenGL)
  IF(OPENGL_FOUND)
    INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
    SET(HAVE_OPENGL TRUE)
  ENDIF(OPENGL_FOUND)

  IF(GLBINDING_ENABLED)
    FIND_PACKAGE(GLBINDING REQUIRED)
    IF(GLBINDING_FOUND)
      INCLUDE_DIRECTORIES(${GLBINDING_INCLUDES})
      ADD_DEFINITIONS(-DUSE_GLBINDING)
      IF (GLBINDING_DEBUG_OUTPUT)
        ADD_DEFINITIONS(-DUSE_GLBINDING_DEBUG_OUTPUT)
      ENDIF()
    ENDIF()
  ELSE()
    FIND_PACKAGE(GLEW REQUIRED)
    IF(GLEW_FOUND)
      INCLUDE_DIRECTORIES(${GLEW_INCLUDE_DIR})
    ENDIF(GLEW_FOUND)
  ENDIF()
ENDIF(ENABLE_OPENGL)

FIND_PACKAGE(OpenAL REQUIRED)
INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIR})

FIND_PACKAGE(OggVorbis REQUIRED)
INCLUDE_DIRECTORIES(${VORBIS_INCLUDE_DIR})

FIND_PACKAGE(PhysFS REQUIRED)
INCLUDE_DIRECTORIES(${PHYSFS_INCLUDE_DIR})

## The curl headers are in the standard include path in dependencies
## so it's not required to add them here
IF(NOT WIN32)
  FIND_PACKAGE(CURL REQUIRED)
  IF(CURL_FOUND)
    INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
    SET(HAVE_LIBCURL TRUE)
  ENDIF(CURL_FOUND)
ENDIF(NOT WIN32)

#FIND_PACKAGE(ICONV REQUIRED)
#INCLUDE_DIRECTORIES(${ICONV_INCLUDE_DIR})
#LINK_LIBRARIES(${ICONV_LIBRARY})

## Find revision of WC
MARK_AS_ADVANCED(GIT_EXECUTABLE)
FIND_PROGRAM(GIT_EXECUTABLE git)

# The .git directory inside the base directory
SET(GIT_CFG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.git/")

IF(GIT_EXECUTABLE AND EXISTS ${GIT_CFG_DIR})
  ADD_CUSTOM_COMMAND(OUTPUT "${CMAKE_BINARY_DIR}/version.h"
                    COMMAND ${CMAKE_COMMAND} -E echo "#ifndef VERSION_H" > "${CMAKE_BINARY_DIR}/version.h.tmp"
                    COMMAND ${CMAKE_COMMAND} -E echo "#define VERSION_H" >> "${CMAKE_BINARY_DIR}/version.h.tmp"
                    COMMAND ${CMAKE_COMMAND} >> "${CMAKE_BINARY_DIR}/version.h.tmp" -E echo_append "#define PACKAGE_VERSION \"${SUPERTUX_VERSION} commit "
                    COMMAND ${GIT_EXECUTABLE} rev-parse --sq HEAD >> "${CMAKE_BINARY_DIR}/version.h.tmp"
                    COMMAND ${CMAKE_COMMAND} >>  "${CMAKE_BINARY_DIR}/version.h.tmp" -E echo "\""
                    COMMAND ${CMAKE_COMMAND} -E echo "#endif" >>  "${CMAKE_BINARY_DIR}/version.h.tmp"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/version.h.tmp" "${CMAKE_BINARY_DIR}/version.h"
                    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/version.h.tmp"
                    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
                    COMMENT "Updating ${CMAKE_BINARY_DIR}/version.h..."
                    VERBATIM)
ELSE(GIT_EXECUTABLE AND EXISTS ${GIT_CFG_DIR})
  ADD_CUSTOM_COMMAND(OUTPUT "${CMAKE_BINARY_DIR}/version.h"
                    COMMAND ${CMAKE_COMMAND} -E echo "#ifndef VERSION_H" > "${CMAKE_BINARY_DIR}/version.h.tmp"
                    COMMAND ${CMAKE_COMMAND} -E echo "#define VERSION_H" >> "${CMAKE_BINARY_DIR}/version.h.tmp"
                    COMMAND ${CMAKE_COMMAND} >> "${CMAKE_BINARY_DIR}/version.h.tmp" -E echo "#define PACKAGE_VERSION \"${SUPERTUX_VERSION}\""
                    COMMAND ${CMAKE_COMMAND} -E echo "#endif" >>  "${CMAKE_BINARY_DIR}/version.h.tmp"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/version.h.tmp" "${CMAKE_BINARY_DIR}/version.h"
                    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/version.h.tmp"
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Updating ${CMAKE_BINARY_DIR}/version.h..."
                    VERBATIM)
ENDIF(GIT_EXECUTABLE AND EXISTS ${GIT_CFG_DIR})

SET_SOURCE_FILES_PROPERTIES(${CMAKE_BINARY_DIR}/version.h
                            PROPERTIES GENERATED true)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/src/supertux/main.cpp
                            PROPERTIES OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/version.h")
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/src/supertux/title_screen.cpp
                            PROPERTIES OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/version.h")
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/src/addon/addon_manager.cpp
                            PROPERTIES OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/version.h")
## Check platform-dependent build options

INCLUDE(ConfigureChecks)

## Also execute instructions in external/squirrel/CMakeLists.txt

ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/external/squirrel)

## Add squirrel lib dir to search path

LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/external/squirrel)

## Some additional include paths

include_directories (${CMAKE_BINARY_DIR}/)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src/)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/external/squirrel/include/)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/external/tinygettext/include/)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/external/findlocale/)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/external/obstack/)

IF(WIN32)
  ADD_DEFINITIONS(-D_USE_MATH_DEFINES -DNOMINMAX)
ENDIF(WIN32)

## Build list of sources for supertux binary

FILE(GLOB SUPERTUX_SOURCES_C RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} external/obstack/*.c external/findlocale/findlocale.c)

FILE(GLOB_RECURSE SUPERTUX_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
FILE(GLOB SUPERTUX_RESOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${PROJECT_BINARY_DIR}/tmp/*.rc")

FILE(GLOB TINYGETTEXT_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} external/tinygettext/src/*.cpp)
SET_SOURCE_FILES_PROPERTIES(${TINYGETTEXT_SOURCES_CXX} PROPERTIES COMPILE_DEFINITIONS HAVE_SDL)

IF(HAVE_OPENGL)
  FILE(GLOB SUPERTUX_OPENGL_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/video/gl/*.cpp)
  SET(SUPERTUX_SOURCES_CXX ${SUPERTUX_SOURCES_CXX} ${SUPERTUX_OPENGL_SOURCES})
ENDIF(HAVE_OPENGL)

## Build miniswig and generate miniswig wrapper

OPTION(GENERATE_WRAPPER "Build miniswig and generate the wrapper" OFF)
IF(GENERATE_WRAPPER)
  ADD_SUBDIRECTORY(tools/miniswig)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.hpp
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${CMAKE_CXX_COMPILER}
    ARGS -x "c++" -E -CC -DSCRIPTING_API src/scripting/wrapper.interface.hpp -o ${CMAKE_CURRENT_BINARY_DIR}/miniswig.tmp -I${CMAKE_CURRENT_SOURCE_DIR}/src
    COMMAND tools/miniswig/miniswig
    ARGS --input miniswig.tmp --output-cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.cpp --output-hpp ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.hpp --module supertux --select-namespace scripting
    DEPENDS tools/miniswig/miniswig
    IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.interface.hpp
  )
ENDIF(GENERATE_WRAPPER)

IF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.cpp)
  SET(SUPERTUX_SOURCES_CXX ${SUPERTUX_SOURCES_CXX} ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.cpp)
ENDIF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.cpp)

## Compile everything at once (roughly equivalent to cat *.cpp | gcc)

OPTION(COMPILE_AMALGATION "Compile all the files together at once (experimental)" OFF)
IF(COMPILE_AMALGATION)
  FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp "// Includes all source files of the project\n")
  FOREACH(ST_FILE ${SUPERTUX_SOURCES_CXX})
    FILE(RELATIVE_PATH CXX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src "${CMAKE_CURRENT_SOURCE_DIR}/${ST_FILE}")
    FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp "#include \"${CXX_FILE}\"\n")
  ENDFOREACH(ST_FILE)
  ## Build instead of CXX sources
  SET(SUPERTUX_SOURCES_CXX ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp)
ELSE(COMPILE_AMALGATION)
  IF(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp)
    FILE(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp)
  ENDIF(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp)
ENDIF(COMPILE_AMALGATION)

## Debug options

OPTION(WERROR "Stop on first compiler warning" OFF)
OPTION(WARNINGS "Enable long list of warnings for compiler to check" OFF)

# http://www.guyrutenberg.com/2014/01/05/enabling-c11-c0x-in-cmake/
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else(COMPILER_SUPPORTS_CXX11)
  CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
  if(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else(COMPILER_SUPPORTS_CXX0X)
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
  endif(COMPILER_SUPPORTS_CXX0X)
endif(COMPILER_SUPPORTS_CXX11)


IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g" CACHE STRING "Debug flags")
  SET(CMAKE_C_FLAGS_DEBUG "-O0 -g" CACHE STRING "Debug flags")
  SET(CMAKE_CXX_FLAGS_PROFILE "-pg" CACHE STRING "Profile flags")
  SET(CMAKE_C_FLAGS_PROFILE "-pg" CACHE STRING "Profile flags")
  SET(CMAKE_LD_FLAGS_PROFILE "-lgmon" CACHE STRING "Profile flags")
  ADD_DEFINITIONS(-Wall -Wextra -Wno-unused-parameter -funit-at-a-time)
  IF(WERROR)
    ADD_DEFINITIONS(-Werror)
  ELSE(WERROR)
    REMOVE_DEFINITIONS(-Werror)
  ENDIF(WERROR)
  IF(WARNINGS)
    # temporarily disabled:
    #   -Wsign-conversion -Wfloat-equal -Wconversion -Wundef -Wswitch-default
    #   -Wswitch-enum -Wsign-promo -Wcast-qual -Woverloaded-virtual -Wmissing-format-attribute
    #   -Wold-style-cast -Wpadded -Wabi -Winline -Wunsafe-loop-optimizations -Wstrict-overflow=5
    # fails on MinGW:
    #   -ansi
    # fails on clang:
    #   -Wlogical-op -Wstrict-null-sentinel
    SET(SUPERTUX2_EXTRA_WARNING_FLAGS "-fdiagnostics-show-option -pedantic -Wno-long-long -Wcast-align -Wdisabled-optimization -Winit-self -Winvalid-pch -Wmissing-include-dirs -Wmissing-noreturn -Wpacked -Wredundant-decls -Wstack-protector -Wformat=2 -Weffc++ -Wctor-dtor-privacy  -Wno-unused-parameter -Wshadow -Wnon-virtual-dtor -Wcast-qual")
  ENDIF(WARNINGS)
ENDIF(CMAKE_COMPILER_IS_GNUCC)

## If xgettext is available, generate messages.pot for sources

OPTION(GENERATE_MESSAGESPOT "Generate messages.pot files" OFF)
IF(GENERATE_MESSAGESPOT)
  MARK_AS_ADVANCED(
    XGETTEXT_EXECUTABLE
  )
  FIND_PROGRAM(XGETTEXT_EXECUTABLE xgettext)
  IF(NOT XGETTEXT_EXECUTABLE)
    MESSAGE(STATUS "Warning: xgettext not found - will not update messages.pot")
  ELSE(NOT XGETTEXT_EXECUTABLE)
    MESSAGE(STATUS "Found xgettext: ${XGETTEXT_EXECUTABLE}")

    SET(MESSAGES_POT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/data/locale/messages.pot)
    ADD_CUSTOM_COMMAND(
      OUTPUT ${MESSAGES_POT_FILE}
      COMMAND ${XGETTEXT_EXECUTABLE}
      ARGS --keyword=_ --language=C++ --output=${MESSAGES_POT_FILE} ${SUPERTUX_SOURCES_CXX}
      DEPENDS ${SUPERTUX_SOURCES_CXX}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Generating POT file ${MESSAGES_POT_FILE}"
    )
    LIST(APPEND MESSAGES_POT_FILES ${MESSAGES_POT_FILE})

    FILE(GLOB SUPERTUX_LEVEL_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} data/levels/*)
    FOREACH(SUPERTUX_LEVEL_DIR ${SUPERTUX_LEVEL_DIRS})
      FILE(GLOB SUPERTUX_LEVELS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SUPERTUX_LEVEL_DIR}/*.stl ${SUPERTUX_LEVEL_DIR}/*.stwm ${SUPERTUX_LEVEL_DIR}/*.txt)

      ## Do not add target if SUPERTUX_LEVELS is empty. This is needed for cmake <=2.4
      IF(SUPERTUX_LEVELS)
      SET(MESSAGES_POT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${SUPERTUX_LEVEL_DIR}/messages.pot)
      ADD_CUSTOM_COMMAND(
        OUTPUT ${MESSAGES_POT_FILE}
        COMMAND ${XGETTEXT_EXECUTABLE}
        ARGS --keyword="_:1" --language=Lisp --force-po --sort-by-file "--output=${MESSAGES_POT_FILE}" ${SUPERTUX_LEVELS}
        DEPENDS ${SUPERTUX_LEVELS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating POT file ${MESSAGES_POT_FILE}"
      )
      LIST(APPEND MESSAGES_POT_FILES ${MESSAGES_POT_FILE})
      ENDIF(SUPERTUX_LEVELS)

    ENDFOREACH(SUPERTUX_LEVEL_DIR)

    ADD_CUSTOM_TARGET(
      supertux2-messages ALL
      DEPENDS ${MESSAGES_POT_FILES}
    )

  ENDIF(NOT XGETTEXT_EXECUTABLE)
ENDIF(GENERATE_MESSAGESPOT)

## Some additional compiler switches
IF(CMAKE_COMPILER_IS_GNUCC)
  ADD_DEFINITIONS(-fno-strict-aliasing)
ENDIF(CMAKE_COMPILER_IS_GNUCC)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  ADD_DEFINITIONS(-DMACOSX)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


## On Windows, add an icon

IF(WIN32)
#  SET(SUPERTUX_SOURCES_C ${SUPERTUX_SOURCES_C} ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.rc)
  IF(MINGW)
    ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o
    COMMAND ${CMAKE_RC_COMPILER} -I${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons -i${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.rc -o ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o)
    SET(SUPERTUX_SOURCES_C ${SUPERTUX_SOURCES_C} ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o)
  ENDIF(MINGW)
ENDIF(WIN32)

## Generate supertux executable in the right place

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

## Add target for supertux binary

ADD_LIBRARY(supertux2_c OBJECT ${SUPERTUX_SOURCES_C})
ADD_EXECUTABLE(supertux2 ${CMAKE_BINARY_DIR}/version.h ${SUPERTUX_SOURCES_CXX} ${TINYGETTEXT_SOURCES_CXX} ${SUPERTUX_RESOURCES} $<TARGET_OBJECTS:supertux2_c>)
SET_TARGET_PROPERTIES(supertux2 PROPERTIES COMPILE_FLAGS "${SUPERTUX2_EXTRA_WARNING_FLAGS}")

IF(WIN32)
  ## Copy dlls on windows
  ADD_CUSTOM_COMMAND(TARGET supertux2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${PROJECT_SOURCE_DIR}/dependencies/dll"
      $<TARGET_FILE_DIR:supertux2>)

  ## Link data directory - doesn't work because you would need admin rights...
  ## Copying is really heavy, so finding the data in the parent directory would be nice
  #ADD_CUSTOM_COMMAND(TARGET supertux2 POST_BUILD
  #  COMMAND ${CMAKE_COMMAND} -E create_symlink
  #    "${PROJECT_SOURCE_DIR}/data"
  #    $<TARGET_FILE_DIR:supertux2>/data)
ENDIF(WIN32)

## Link supertux binary with squirrel and other libraries

TARGET_INCLUDE_DIRECTORIES(supertux2 SYSTEM PUBLIC ${SDL2_INCLUDE_DIRS})
TARGET_INCLUDE_DIRECTORIES(supertux2 SYSTEM PUBLIC ${SDL2IMAGE_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(supertux2 ${SDL2_LIBRARIES})
TARGET_LINK_LIBRARIES(supertux2 ${SDL2IMAGE_LIBRARIES})

TARGET_LINK_LIBRARIES(supertux2 squirrel)
TARGET_LINK_LIBRARIES(supertux2 ${OPENAL_LIBRARY})
TARGET_LINK_LIBRARIES(supertux2 ${OGGVORBIS_LIBRARIES})
TARGET_LINK_LIBRARIES(supertux2 ${PHYSFS_LIBRARY})
IF(HAVE_OPENGL)
    TARGET_LINK_LIBRARIES(supertux2 ${OPENGL_LIBRARY})
    IF(GLBINDING_FOUND)
      TARGET_LINK_LIBRARIES(supertux2 ${GLBINDING_LIBRARIES})
    ELSE()
      TARGET_LINK_LIBRARIES(supertux2 ${GLEW_LIBRARY})
    ENDIF()
ENDIF(HAVE_OPENGL)
IF(HAVE_LIBCURL)
  TARGET_LINK_LIBRARIES(supertux2 ${CURL_LIBRARY})
ENDIF(HAVE_LIBCURL)

## Link curl on windows
IF(WIN32)
    TARGET_LINK_LIBRARIES(supertux2 ${PROJECT_SOURCE_DIR}/dependencies/lib/libcurldll.a)
ENDIF()

## Install stuff

IF(WIN32 AND NOT UNIX)

  SET(INSTALL_SUBDIR_BIN "." CACHE STRING "Installation subdir for binaries")
  SET(INSTALL_SUBDIR_SHARE "data" CACHE STRING "Installation subdir for data")
  SET(INSTALL_SUBDIR_DOC "." CACHE STRING "Installation subdir for docs")

  INSTALL(FILES ${DLLS} DESTINATION ${INSTALL_SUBDIR_BIN})

  INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.png ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.ico DESTINATION ".")

ELSE(WIN32 AND NOT UNIX)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  SET(INSTALL_SUBDIR_BIN "SuperTux.app/Contents/MacOS" CACHE STRING "Installation subdir for binaries")
  SET(INSTALL_SUBDIR_SHARE "SuperTux.app/Contents/Resources/data" CACHE STRING "Installation subdir for data")
  SET(INSTALL_SUBDIR_DOC "SuperTux.app/Contents/Resources" CACHE STRING "Installation subdir for docs")

  INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/info.plist DESTINATION "SuperTux.app/Contents/")
  INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/receipt DESTINATION "SuperTux.app/Contents/_MASReceipt/")

  INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.png ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.icns DESTINATION "SuperTux.app/Contents/Resources/")

ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  SET(INSTALL_SUBDIR_BIN "games" CACHE STRING "Installation subdir for binaries")
  SET(INSTALL_SUBDIR_SHARE "share/games/supertux2" CACHE STRING "Installation subdir for data")
  SET(INSTALL_SUBDIR_DOC "share/doc/supertux2" CACHE STRING "Installation subdir for docs")

  INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/supertux2.desktop DESTINATION "share/applications")

  INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.png ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.xpm DESTINATION "share/pixmaps/")

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
ENDIF(WIN32 AND NOT UNIX)

INSTALL(TARGETS supertux2 DESTINATION ${INSTALL_SUBDIR_BIN})

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/INSTALL.md ${CMAKE_CURRENT_SOURCE_DIR}/README.md ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/WHATSNEW.txt DESTINATION ${INSTALL_SUBDIR_DOC})

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/credits.txt DESTINATION ${INSTALL_SUBDIR_SHARE})

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/supertux2.appdata.xml DESTINATION "share/appdata" )

INSTALL(DIRECTORY data/images
                  data/fonts
                  data/levels
                  data/music
                  data/scripts
                  data/speech
                  data/sounds
                  data/locale
                  DESTINATION ${INSTALL_SUBDIR_SHARE})

## Create config.h now that INSTALL_SUBDIR_* have been set.

configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h )


## CPack/Installation-specific stuff

INCLUDE(InstallRequiredSystemLibraries)
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Jump'n'Run Game featuring Tux")
SET(CPACK_PACKAGE_VENDOR "SuperTux Devel Team")
SET(CPACK_PACKAGE_CONTACT "SuperTux Devel Team <supertux-devel@lists.lethargik.org>")
SET(CPACK_DEBIAN_PACKAGE_NAME "supertux2")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.5), libgcc1 (>= 1:4.1), libgl1-mesa-glx | libgl1, libogg0 (>= 1.1.3), libopenal0a, libphysfs-1.0-0, libsdl-image1.2 (>= 1.2.5), libsdl1.2debian (>= 1.2.10-1), libstdc++6 (>= 4.1.2), libvorbis0a (>= 1.1.2), libvorbisfile3 (>= 1.1.2), libcurl3 (>= 7.16)")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Classic 2D jump 'n run sidescroller with Tux\n SuperTux is a classic 2D jump 'n run sidescroller game in a similar\n style like the original SuperMario games. This release of SuperTux\n features 9 enemies, 26 playable levels, software and OpenGL rendering\n modes, configurable joystick and keyboard input, new music and\n completely redone graphics.\n .\n This is a development snapshot of SuperTux. It may suffer from\n critical bugs and has not been fully tested. \n .\n Homepage: http://supertux.lethargik.org/")
SET(CPACK_DEBIAN_PACKAGE_SECTION "games")
SET(CPACK_RPM_PACKAGE_NAME "supertux2")
SET(CPACK_RPM_PACKAGE_DESCRIPTION "Classic 2D jump 'n run sidescroller with Tux\n SuperTux is a classic 2D jump 'n run sidescroller game in a similar\n style like the original SuperMario games. This release of SuperTux\n features 9 enemies, 26 playable levels, software and OpenGL rendering\n modes, configurable joystick and keyboard input, new music and\n completely redone graphics.\n .\n This is a development snapshot of SuperTux. It may suffer from\n critical bugs and has not been fully tested. \n .\n Homepage: http://supertux.lethargik.org/")
SET(CPACK_RPM_PACKAGE_LICENSE "GNU General Public License (GPL)")
SET(CPACK_RPM_PACKAGE_GROUP "Amusements/Games/Action/Arcade")
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # The OS X PackageMaker doesn't seem to like files with no extension
ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
  SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "3")
SET(CPACK_PACKAGE_VERSION_PATCH "4-GIT")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "SuperTux ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_PACKAGE_EXECUTABLES "..\\\\supertux2" "SuperTux ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
IF(WIN32 AND NOT UNIX)
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "..\\\\supertux2")
  SET(CPACK_NSIS_DISPLAY_NAME "SuperTux ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
  SET(CPACK_NSIS_COMPRESSOR "/SOLID lzma")
ELSE(WIN32 AND NOT UNIX)
ENDIF(WIN32 AND NOT UNIX)
INCLUDE(CPack)

# move some config clutter to the advanced section
MARK_AS_ADVANCED(
  INSTALL_SUBDIR_BIN
  INSTALL_SUBDIR_SHARE
  INSTALL_SUBDIR_DOC
)

MARK_AS_ADVANCED(
  CMAKE_BACKWARDS_COMPATIBILITY
  CMAKE_BUILD_TYPE
  CMAKE_INSTALL_PREFIX
  EXECUTABLE_OUTPUT_PATH
  LIBRARY_OUTPUT_PATH
  CMAKE_OSX_ARCHITECTURES
  CMAKE_OSX_SYSROOT
)

MARK_AS_ADVANCED(
  APPDATADIR
)

MARK_AS_ADVANCED(
  Boost_INCLUDE_DIR
  Boost_LIBRARIES
)

MARK_AS_ADVANCED(
  GLEW_INCLUDE_DIR
  GLEW_LIBRARY
)

MARK_AS_ADVANCED(
  SDL2_INCLUDE_DIRS
  SDL2_LIBRARIES
)

MARK_AS_ADVANCED(
  SDL2IMAGE_INCLUDE_DIRS
  SDL2IMAGE_LIBRARIES
)

MARK_AS_ADVANCED(
  OPENAL_INCLUDE_DIR
  OPENAL_LIBRARY
)

MARK_AS_ADVANCED(
  OGG_LIBRARY
  VORBISENC_LIBRARY
  VORBISFILE_LIBRARY
  VORBIS_INCLUDE_DIR
  VORBIS_LIBRARY
)

MARK_AS_ADVANCED(
  PHYSFS_INCLUDE_DIR
  PHYSFS_LIBRARY
)
