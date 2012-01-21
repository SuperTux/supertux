#
# TinyGetText build script
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


CMAKE_POLICY(SET CMP0005 NEW)

## Project name to use as command prefix

PROJECT(tinygettext)
SET(VERSION "0.1")

### CMake configuration

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND cmake_policy)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${tinygettext_SOURCE_DIR})

# move some config clutter to the advanced section
MARK_AS_ADVANCED(
	CMAKE_BACKWARDS_COMPATIBILITY
	CMAKE_BUILD_TYPE
	CMAKE_INSTALL_PREFIX
	EXECUTABLE_OUTPUT_PATH
	CMAKE_OSX_ARCHITECTURES
	CMAKE_OSX_SYSROOT
)

## Reveal library type choice to users
OPTION(BUILD_SHARED_LIBS "Produce dynamic library instead of static archive" ON)

## Add iconv to include directories

FIND_PACKAGE(ICONV REQUIRED)
INCLUDE_DIRECTORIES(${ICONV_INCLUDE_DIR})

## Check iconv_const

INCLUDE(CheckCXXSourceCompiles)

SET(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${ICONV_INCLUDE_DIR})
CHECK_CXX_SOURCE_COMPILES(
	"
	#include <iconv.h>
	// this declaration will fail when there already exists a non const char** version which returns size_t
	double iconv(iconv_t cd,  char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft);
	int main() { return 0; }
	"
	HAVE_ICONV_CONST
)

# TODO: better way of config

IF(HAVE_ICONV_CONST)
  ADD_DEFINITIONS(-DHAVE_ICONV_CONST)
ELSE(HAVE_ICONV_CONST)
  REMOVE_DEFINITIONS(-DHAVE_ICONV_CONST)
ENDIF(HAVE_ICONV_CONST)

## TinyGetText library compilation

## build list of source files

FILE(GLOB TINYGETTEXT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} tinygettext/*.cpp)
FILE(GLOB TINYGETTEXT_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} tinygettext/*.hpp)

## define a target for building the library

ADD_LIBRARY(tinygettext ${TINYGETTEXT_SOURCES})

## Add tinygettext dir to search path

INCLUDE_DIRECTORIES(${tinygettext_SOURCE_DIR})

## Debug options

OPTION(WERROR "Stops on first compiler warning in debug mode" OFF)
IF(CMAKE_COMPILER_IS_GNUCC)
  ADD_DEFINITIONS(-O3 -Wall -Wextra -Weffc++ -pedantic) 
  # -ansi fails in MinGW
  OPTION(WARNINGS "Enable long list of warnings for compiler to check" ON)
  IF(WARNINGS)
    ADD_DEFINITIONS(
          -Wabi  -Wctor-dtor-privacy
          -Wstrict-null-sentinel
          -Wold-style-cast
          -Woverloaded-virtual
          -Wsign-promo -Wswitch-enum
          -Wcast-align  -Wcast-qual
          -Wdisabled-optimization
          -Wfloat-equal  
          -Wformat=2
          -Winit-self 
          -Winvalid-pch  -Wunsafe-loop-optimizations
          -Wlogical-op 
          -Wmissing-format-attribute  -Wmissing-include-dirs -Wmissing-noreturn 
          -Wpacked
          -Wredundant-decls
          -Wshadow
          -Wsign-conversion  -Wstack-protector
          -Wstrict-overflow=5
          -Wswitch-default  -Wswitch-enum
          -Wundef)
        # Still left:
        # -Wconversion  (find alternative to using toupper(int) on char)
        # -Wpadded      (DictionaryManager has a bool that sticks out)
  ENDIF(WARNINGS)
  IF(WERROR)
    ADD_DEFINITIONS(-Werror)
  ENDIF(WERROR)
ENDIF(CMAKE_COMPILER_IS_GNUCC)

## Extra definitions

ADD_DEFINITIONS(-DVERSION=\\\"${VERSION}\\\")

## Generate test executables in the right place

SET(EXECUTABLE_OUTPUT_PATH ${tinygettext_BINARY_DIR}/test)

## Build tinygettext tests

FOREACH(TEST tinygettext_test po_parser_test)
  ## Add target for tinygettext test
  ADD_EXECUTABLE(${TEST} test/${TEST}.cpp)
  ## Link with tinygettext library
  TARGET_LINK_LIBRARIES(${TEST} tinygettext)
  TARGET_LINK_LIBRARIES(${TEST} ${ICONV_LIBRARY})
ENDFOREACH(TEST)

## Install tinygettext

# use standardized variable name
SET(LIB_SUBDIR "lib${LIB_SUFFIX}"
	CACHE STRING "Subdirectory of prefix into which libraries are installed (e.g., lib32, lib64)")

## prepare tinygettext.pc
CONFIGURE_FILE(tinygettext.pc.in tinygettext.pc @ONLY)

INSTALL(TARGETS tinygettext
	ARCHIVE DESTINATION ${LIB_SUBDIR}
	LIBRARY DESTINATION ${LIB_SUBDIR})
INSTALL(FILES ${TINYGETTEXT_HEADERS}
	DESTINATION include/tinygettext)
INSTALL(FILES ${tinygettext_BINARY_DIR}/tinygettext.pc
	DESTINATION ${LIB_SUBDIR}/pkgconfig)
