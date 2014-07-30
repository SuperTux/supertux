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
## Will add r<revision number> to the end if svnversion is available.
SET(SUPERTUX_VERSION "0.3.4-GIT")

### CMake configuration

# allow building with CMake 2.6. Set compatibility options. Use some bundled modules as a fallback
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND cmake_policy)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/mk/cmake)

## For autopackage
OPTION(ENABLE_BINRELOC "Enable autopackage's BINRELOC features" OFF)
SET(APPDATADIR "${CMAKE_INSTALL_PREFIX}/share/games/supertux2")

## Check endianess

INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

## Add lots of dependencies to compiler switches

SET(Boost_ADDITIONAL_VERSIONS "1.41" "1.41.0")
FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS})
LINK_LIBRARIES(supertux2 ${SDL2_LIBRARIES})

PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
INCLUDE_DIRECTORIES(${SDL2IMAGE_INCLUDE_DIRS})
LINK_LIBRARIES(supertux2 ${SDL2IMAGE_LIBRARIES})
SET(HAVE_SDL TRUE)

#FIND_PACKAGE(SDL_image REQUIRED)
#INCLUDE_DIRECTORIES(${SDLIMAGE_INCLUDE_DIR})

OPTION(ENABLE_OPENGL "Enable OpenGL support" ON)
IF(ENABLE_OPENGL)
  FIND_PACKAGE(OpenGL)
  IF(OPENGL_FOUND)
    INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
    SET(HAVE_OPENGL TRUE)
  ENDIF(OPENGL_FOUND)

  FIND_PACKAGE(GLEW)
  IF(GLEW_FOUND)
    INCLUDE_DIRECTORIES(${GLEW_INCLUDE_DIR})
  ENDIF(GLEW_FOUND)
ENDIF(ENABLE_OPENGL)

FIND_PACKAGE(OpenAL REQUIRED)
INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIR})

FIND_PACKAGE(OggVorbis REQUIRED)
INCLUDE_DIRECTORIES(${VORBIS_INCLUDE_DIR})

FIND_PACKAGE(PhysFS REQUIRED)
INCLUDE_DIRECTORIES(${PHYSFS_INCLUDE_DIR})

FIND_PACKAGE(CURL)
IF(CURL_FOUND)
  INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
  SET(HAVE_LIBCURL TRUE)
ENDIF(CURL_FOUND)

#FIND_PACKAGE(ICONV REQUIRED)
#INCLUDE_DIRECTORIES(${ICONV_INCLUDE_DIR})
#LINK_LIBRARIES(${ICONV_LIBRARY})

## Find revision of WC
MARK_AS_ADVANCED(GIT_EXECUTABLE)
FIND_PROGRAM(GIT_EXECUTABLE git)

IF(GIT_EXECUTABLE)
  ADD_CUSTOM_TARGET(gitversion ALL
                    ${CMAKE_COMMAND} -E echo "#ifndef VERSION_H" > "${CMAKE_BINARY_DIR}/version.h.tmp"
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
ELSE(GIT_EXECUTABLE)
  ADD_CUSTOM_TARGET(gitversion ALL
                    ${CMAKE_COMMAND} -E echo "#ifndef VERSION_H" > "${CMAKE_BINARY_DIR}/version.h.tmp"
                    COMMAND ${CMAKE_COMMAND} -E echo "#define VERSION_H" >> "${CMAKE_BINARY_DIR}/version.h.tmp"
                    COMMAND ${CMAKE_COMMAND} >> "${CMAKE_BINARY_DIR}/version.h.tmp" -E echo "#define PACKAGE_VERSION \"${SUPERTUX_VERSION}\""
                    COMMAND ${CMAKE_COMMAND} -E echo "#endif" >>  "${CMAKE_BINARY_DIR}/version.h.tmp"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/version.h.tmp" "${CMAKE_BINARY_DIR}/version.h"
                    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/version.h.tmp"
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Updating ${CMAKE_BINARY_DIR}/version.h..."
                    VERBATIM)
ENDIF(GIT_EXECUTABLE)

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
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/external/tinygettext/)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/external/findlocale/)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/external/obstack/)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/external/binreloc/)

## MSVC Compatibility headers and definitions

IF(MSVC)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/mk/msvc)
ADD_DEFINITIONS(-D_USE_MATH_DEFINES -DNOMINMAX)
ENDIF(MSVC)

## Build list of sources for supertux binary

FILE(GLOB SUPERTUX_SOURCES_C RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} external/obstack/*.c external/findlocale/findlocale.c)

FILE(GLOB SUPERTUX_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/main.cpp src/*/*.cpp src/supertux/menu/*.cpp src/video/sdl/*.cpp)

FILE(GLOB TINYGETTEXT_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} external/tinygettext/tinygettext/*.cpp)
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
IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g" CACHE STRING "Debug flags")
  SET(CMAKE_C_FLAGS_DEBUG "-O0 -g" CACHE STRING "Debug flags")
  SET(CMAKE_CXX_FLAGS_PROFILE "-pg" CACHE STRING "Profile flags")
  SET(CMAKE_C_FLAGS_PROFILE "-pg" CACHE STRING "Profile flags")
  SET(CMAKE_LD_FLAGS_PROFILE "-lgmon" CACHE STRING "Profile flags")
  ADD_DEFINITIONS(-Wall -Wextra -funit-at-a-time)
  IF(WERROR)
    ADD_DEFINITIONS(-Werror)
  ELSE(WERROR)
    REMOVE_DEFINITIONS(-Werror)
  ENDIF(WERROR)
  IF(WARNINGS)
    ADD_DEFINITIONS(-fdiagnostics-show-option -pedantic -Wno-long-long -Wcast-align -Wdisabled-optimization -Winit-self -Winvalid-pch -Wlogical-op -Wmissing-include-dirs -Wmissing-noreturn -Wpacked -Wredundant-decls -Wstack-protector -Wconversion -Wundef -Wsign-conversion -Wshadow -Winline -Wunsafe-loop-optimizations -Wfloat-equal -Wswitch-default -Wswitch-enum -Wcast-qual -Wsign-promo -Woverloaded-virtual -Wmissing-format-attribute -Wstrict-overflow=5 -Wformat=2-Weffc++ -Wabi -Wctor-dtor-privacy -Wstrict-null-sentinel -Wno-unused-parameter)
    # -ansi fails in MinGW
    # still left:  -Wold-style-cast -Wpadded
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
    LIST(REMOVE_ITEM SUPERTUX_LEVEL_DIRS "data/levels/.svn")
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
    COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons -i${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.rc -o ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o)
    SET(SUPERTUX_SOURCES_C ${SUPERTUX_SOURCES_C} ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o)
  ELSE(MINGW)
    SET(SUPERTUX_EXE_SRCS ${SUPERTUX_EXE_SRCS} supertux.rc)
  ENDIF(MINGW)
ENDIF(WIN32)

## Add binreloc.c if enabled
IF(ENABLE_BINRELOC)
  SET(SUPERTUX_SOURCES_C ${SUPERTUX_SOURCES_C} ${CMAKE_CURRENT_SOURCE_DIR}/external/binreloc/binreloc.c)
ENDIF(ENABLE_BINRELOC)


## Generate supertux executable in the right place

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

## Add target for supertux binary

ADD_EXECUTABLE(supertux2 ${CMAKE_BINARY_DIR}/version.h ${SUPERTUX_SOURCES_C} ${SUPERTUX_SOURCES_CXX} ${TINYGETTEXT_SOURCES_CXX})
ADD_DEPENDENCIES(supertux2 gitversion)

## Link supertux binary with squirrel and other libraries

TARGET_LINK_LIBRARIES(supertux2 squirrel)
TARGET_LINK_LIBRARIES(supertux2 ${SDL_LIBRARY})
TARGET_LINK_LIBRARIES(supertux2 ${SDLIMAGE_LIBRARY})
TARGET_LINK_LIBRARIES(supertux2 ${OPENAL_LIBRARY})
TARGET_LINK_LIBRARIES(supertux2 ${OGGVORBIS_LIBRARIES})
TARGET_LINK_LIBRARIES(supertux2 ${PHYSFS_LIBRARY})
IF(HAVE_OPENGL)
    TARGET_LINK_LIBRARIES(supertux2 ${OPENGL_LIBRARY})
    TARGET_LINK_LIBRARIES(supertux2 ${GLEW_LIBRARY})
ENDIF(HAVE_OPENGL)
IF(HAVE_LIBCURL)
  TARGET_LINK_LIBRARIES(supertux2 ${CURL_LIBRARY})
ENDIF(HAVE_LIBCURL)

## Install stuff

IF(WIN32 AND NOT UNIX)

  SET(INSTALL_SUBDIR_BIN "." CACHE STRING "Installation subdir for binaries")
  SET(INSTALL_SUBDIR_SHARE "data" CACHE STRING "Installation subdir for data")
  SET(INSTALL_SUBDIR_DOC "." CACHE STRING "Installation subdir for docs")

  FILE(GLOB DLLS "${CMAKE_CURRENT_SOURCE_DIR}/*.dll")
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

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/README ${CMAKE_CURRENT_SOURCE_DIR}/COPYING ${CMAKE_CURRENT_SOURCE_DIR}/WHATSNEW.txt DESTINATION ${INSTALL_SUBDIR_DOC})

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/credits.txt DESTINATION ${INSTALL_SUBDIR_SHARE})

INSTALL(DIRECTORY data/images
                  data/fonts
                  data/levels
                  data/music
                  data/scripts
                  data/speech
                  data/sounds
                  data/locale
                               DESTINATION ${INSTALL_SUBDIR_SHARE} PATTERN ".svn" EXCLUDE)

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
  SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
  SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "3")
SET(CPACK_PACKAGE_VERSION_PATCH "4-GIT")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "SuperTux ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_PACKAGE_EXECUTABLES "..\\\\supertux2" "SuperTux ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
IF(WIN32 AND NOT UNIX)
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "..\\\\supertux2")
  SET(CPACK_NSIS_DISPLAY_NAME "SuperTux ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
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
	ENABLE_BINRELOC
	APPDATADIR
)

MARK_AS_ADVANCED(
	Boost_INCLUDE_DIR
	Boost_LIBRARIES
)

MARK_AS_ADVANCED(
	SDLMAIN_LIBRARY
	SDL_INCLUDE_DIR
	SDL_LIBRARY
)

MARK_AS_ADVANCED(
	SDLIMAGE_INCLUDE_DIR
	SDLIMAGE_LIBRARY
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
