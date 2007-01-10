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
# FIXME: compile miniswig
# FIXME: use miniswig to create squirrel wrappers
# FIXME: create messages.pot for levels
# FIXME: provide working install target
#


## (Optional) Generate noisy Makefile

#SET(CMAKE_VERBOSE_MAKEFILE on)

## Project name to use as command prefix

PROJECT(SUPERTUX)

## Search here for additional cmake modules (for finding deps)

set(CMAKE_MODULE_PATH ${SUPERTUX_SOURCE_DIR}/mk/cmake ${CMAKE_MODULE_PATH})

## Add lots of dependencies to compiler switches

FIND_PACKAGE(SDL REQUIRED)
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR})
LINK_LIBRARIES(${SDL_LIBRARY})

FIND_PACKAGE(SDL_image REQUIRED)
INCLUDE_DIRECTORIES(${SDLIMAGE_INCLUDE_DIR})
LINK_LIBRARIES(${SDLIMAGE_LIBRARY})

FIND_PACKAGE(OpenGL REQUIRED)
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
LINK_LIBRARIES(${OPENGL_LIBRARY})

FIND_PACKAGE(OpenAL REQUIRED)
INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIR})
LINK_LIBRARIES(${OPENAL_LIBRARY})

FIND_PACKAGE(OggVorbis REQUIRED)
INCLUDE_DIRECTORIES(${OGGVORBIS_INCLUDE_DIR})
LINK_LIBRARIES(${OGGVORBIS_LIBRARIES})

FIND_PACKAGE(PhysFS REQUIRED)
INCLUDE_DIRECTORIES(${PHYSFS_INCLUDE_DIR})
LINK_LIBRARIES(${PHYSFS_LIBRARY})

## Also execute instructions in src/squirrel/CMakeLists.txt

ADD_SUBDIRECTORY(src/squirrel)

## Add squirrel lib dir to search path

LINK_DIRECTORIES(src/squirrel)

## Some additional include paths

include_directories (${SUPERTUX_SOURCE_DIR}/)
include_directories (${SUPERTUX_SOURCE_DIR}/src/)
include_directories (${SUPERTUX_SOURCE_DIR}/src/squirrel/include/)

## Build list of sources for supertux binary

FILE(GLOB SUPERTUX_SOURCES RELATIVE ${SUPERTUX_SOURCE_DIR} src/*.cpp src/audio/*.cpp src/badguy/*.cpp src/binreloc/*.cpp src/control/*.cpp src/gui/*.cpp src/lisp/*.cpp src/math/*.cpp src/object/*.cpp src/physfs/*.cpp src/sprite/*.cpp src/tinygettext/*.cpp src/trigger/*.cpp src/video/*.cpp src/worldmap/*.cpp src/scripting/*.cpp)

## If xgettext is available, generate messages.pot for sources

FIND_PROGRAM(
  XGETTEXT 
  NAMES "xgettext"
  PATHS "/usr/bin"
)
IF(${XGETTEXT} STREQUAL "XGETTEXT-NOTFOUND")
  MESSAGE(STATUS "Warning: xgettext not found - will not update messages.pot")
ELSE(${XGETTEXT} STREQUAL "XGETTEXT-NOTFOUND")
  MESSAGE(STATUS "Found xgettext: ${XGETTEXT}")
  SET(MESSAGES_POT_FILE ${SUPERTUX_SOURCE_DIR}/data/locale/messages.pot)
  ADD_CUSTOM_TARGET(
    supertux-messages.pot ALL 
    COMMAND "/usr/bin/xgettext" -k_ -C -o ${MESSAGES_POT_FILE} ${SUPERTUX_SOURCES}
    DEPENDS ${SUPERTUX_SOURCES}
    WORKING_DIRECTORY ${SUPERTUX_SOURCE_DIR}
  )
ENDIF(${XGETTEXT} STREQUAL "XGETTEXT-NOTFOUND")

## Some additional compiler switches

ADD_DEFINITIONS(-O2 -fno-strict-aliasing)

## Add target for supertux binary

ADD_EXECUTABLE(supertux ${SUPERTUX_SOURCES} )

## Link supertux binary with squirrel

TARGET_LINK_LIBRARIES(supertux squirrel)

## After building, copy binary to source root

ADD_CUSTOM_COMMAND(
  TARGET supertux
  POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy supertux ${SUPERTUX_SOURCE_DIR}/supertux
)

