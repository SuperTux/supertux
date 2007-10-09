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


#
# FIXME: create config.h from config.h.in
# FIXME: create messages.pot for levels
#


## (Optional) Generate noisy Makefile

#SET(CMAKE_VERBOSE_MAKEFILE on)

## Project name to use as command prefix

PROJECT(SUPERTUX)

## Some default settings

set(DEBUG 0 CACHE BOOL "Build with debugging options")
set(ENABLE_BINRELOC 0 CACHE BOOL "Enable autopackage's BINRELOC features")
set(APPDATADIR "${CMAKE_INSTALL_PREFIX}/share" CACHE STRING "APPDATADIR for autopackage's BINRELOC features")
set(ENABLE_SQDBG 0 CACHE BOOL "Build squirrel script interpreter with debugging options")
set(ENABLE_OPENGL 1 CACHE BOOL "Enable OpenGL support")

## Search here for additional cmake modules (for finding deps)

set(CMAKE_MODULE_PATH ${SUPERTUX_SOURCE_DIR}/mk/cmake ${CMAKE_MODULE_PATH})

## Add lots of dependencies to compiler switches

FIND_PACKAGE(SDL REQUIRED)
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR})
LINK_LIBRARIES(${SDL_LIBRARY})

FIND_PACKAGE(SDL_image REQUIRED)
INCLUDE_DIRECTORIES(${SDLIMAGE_INCLUDE_DIR})
LINK_LIBRARIES(${SDLIMAGE_LIBRARY})

IF(ENABLE_OPENGL)
  FIND_PACKAGE(OpenGL)
  IF(${OPENGL_FOUND} STREQUAL "YES")
    INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
    LINK_LIBRARIES(${OPENGL_LIBRARY})
    SET(HAVE_OPENGL TRUE)
  ENDIF(${OPENGL_FOUND} STREQUAL "YES")
ENDIF(ENABLE_OPENGL)

FIND_PACKAGE(OpenAL REQUIRED)
INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIR})
LINK_LIBRARIES(${OPENAL_LIBRARY})

FIND_PACKAGE(OggVorbis REQUIRED)
INCLUDE_DIRECTORIES(${OGGVORBIS_INCLUDE_DIR})
LINK_LIBRARIES(${OGGVORBIS_LIBRARIES})

FIND_PACKAGE(PhysFS REQUIRED)
INCLUDE_DIRECTORIES(${PHYSFS_INCLUDE_DIR})
LINK_LIBRARIES(${PHYSFS_LIBRARY})

#FIND_PACKAGE(ICONV REQUIRED)
#INCLUDE_DIRECTORIES(${ICONV_INCLUDE_DIR})
#LINK_LIBRARIES(${ICONV_LIBRARY})

## Check platform-dependent build options

INCLUDE(ConfigureChecks)

## Create config.h

configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h )
include_directories (${CMAKE_BINARY_DIR}/)

## Also execute instructions in tools/miniswig/CMakeLists.txt
ADD_SUBDIRECTORY(tools/miniswig)

## Also execute instructions in src/squirrel/CMakeLists.txt

ADD_SUBDIRECTORY(src/squirrel)

## Add squirrel lib dir to search path

LINK_DIRECTORIES(src/squirrel)

## Some additional include paths

include_directories (${SUPERTUX_SOURCE_DIR}/src/)
include_directories (${SUPERTUX_SOURCE_DIR}/src/squirrel/)
include_directories (${SUPERTUX_SOURCE_DIR}/src/squirrel/include/)

## Build list of sources for supertux binary

FILE(GLOB SUPERTUX_SOURCES RELATIVE ${SUPERTUX_SOURCE_DIR} src/*.cpp src/audio/*.cpp src/badguy/*.cpp src/binreloc/*.cpp src/control/*.cpp src/gui/*.cpp src/lisp/*.cpp src/math/*.cpp src/object/*.cpp src/physfs/*.cpp src/sprite/*.cpp src/tinygettext/*.cpp src/trigger/*.cpp src/video/*.cpp src/worldmap/*.cpp src/scripting/*.cpp)

## Generate miniswig wrapper

ADD_CUSTOM_COMMAND(
  OUTPUT ${SUPERTUX_SOURCE_DIR}/src/scripting/wrapper.cpp ${SUPERTUX_SOURCE_DIR}/src/scripting/wrapper.hpp
  COMMAND cd ${SUPERTUX_SOURCE_DIR} && ${CMAKE_CXX_COMPILER}
  ARGS -x "c++" -E -CC -DSCRIPTING_API src/scripting/wrapper.interface.hpp -o ${SUPERTUX_BINARY_DIR}/miniswig.tmp
  COMMAND tools/miniswig/miniswig
  ARGS --input miniswig.tmp --output-cpp ${SUPERTUX_SOURCE_DIR}/src/scripting/wrapper.cpp --output-hpp ${SUPERTUX_SOURCE_DIR}/src/scripting/wrapper.hpp --module supertux --select-namespace Scripting
  DEPENDS src/scripting/wrapper.interface.hpp tools/miniswig/miniswig
)

SET(SUPERTUX_SOURCES ${SUPERTUX_SOURCE_DIR}/src/scripting/wrapper.cpp ${SUPERTUX_SOURCES})

## If xgettext is available, generate messages.pot for sources

#FIND_PROGRAM(
#  XGETTEXT 
#  NAMES "xgettext"
#  PATHS "/usr/bin"
#)
#IF(${XGETTEXT} STREQUAL "XGETTEXT-NOTFOUND")
#  MESSAGE(STATUS "Warning: xgettext not found - will not update messages.pot")
#ELSE(${XGETTEXT} STREQUAL "XGETTEXT-NOTFOUND")
#  MESSAGE(STATUS "Found xgettext: ${XGETTEXT}")
#  SET(MESSAGES_POT_FILE ${SUPERTUX_SOURCE_DIR}/data/locale/messages.pot)
#  ADD_CUSTOM_TARGET(
#    supertux-messages.pot ALL 
#    COMMAND "/usr/bin/xgettext" -k_ -C -o ${MESSAGES_POT_FILE} ${SUPERTUX_SOURCES}
#    DEPENDS ${SUPERTUX_SOURCES}
#    WORKING_DIRECTORY ${SUPERTUX_SOURCE_DIR}
#  )
#ENDIF(${XGETTEXT} STREQUAL "XGETTEXT-NOTFOUND")

## Some additional compiler switches

ADD_DEFINITIONS(-O2 -fno-strict-aliasing)

## Add target for supertux binary

ADD_EXECUTABLE(supertux2 ${SUPERTUX_SOURCES})

## Link supertux binary with squirrel

TARGET_LINK_LIBRARIES(supertux2 squirrel)

## After building, copy binary to source root

ADD_CUSTOM_COMMAND(
  TARGET supertux2
  POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy supertux2 ${SUPERTUX_SOURCE_DIR}/supertux2
)

## Install stuff

INSTALL(TARGETS supertux2 DESTINATION bin)

INSTALL(FILES ${SUPERTUX_SOURCE_DIR}/data/images/engine/icons/supertux.png ${SUPERTUX_SOURCE_DIR}/data/images/engine/icons/supertux.xpm DESTINATION share/pixmaps)

INSTALL(FILES ${SUPERTUX_SOURCE_DIR}/supertux2.desktop DESTINATION share/applications)

INSTALL(FILES ${SUPERTUX_SOURCE_DIR}/INSTALL ${SUPERTUX_SOURCE_DIR}/README ${SUPERTUX_SOURCE_DIR}/COPYING ${SUPERTUX_SOURCE_DIR}/WHATSNEW.txt DESTINATION doc/supertux-0.3.x)

INSTALL(FILES ${SUPERTUX_SOURCE_DIR}/data/credits.txt ${SUPERTUX_SOURCE_DIR}/data/camera.cfg DESTINATION share/supertux2)
INSTALL(DIRECTORY data/images DESTINATION share/supertux2 PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY data/levels DESTINATION share/supertux2 PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY data/music DESTINATION share/supertux2 PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY data/scripts DESTINATION share/supertux2 PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY data/speech DESTINATION share/supertux2 PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY data/sounds DESTINATION share/supertux2 PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY data/locale DESTINATION share/supertux2 PATTERN ".svn" EXCLUDE)
