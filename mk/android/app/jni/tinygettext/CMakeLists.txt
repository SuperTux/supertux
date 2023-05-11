# tinygettext - A gettext replacement that works directly on .po files
# Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgement in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

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

cmake_policy(SET CMP0005 NEW)

## Project name to use as command prefix

project(tinygettext)
set(VERSION "0.1")

### CMake configuration

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
	CMAKE_POLICY(SET CMP0003 NEW)
endif(COMMAND cmake_policy)
SET(CMAKE_MACOSX_RPATH ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${tinygettext_SOURCE_DIR})

# move some config clutter to the advanced section
mark_as_advanced(
	CMAKE_BACKWARDS_COMPATIBILITY
	CMAKE_BUILD_TYPE
	CMAKE_INSTALL_PREFIX
	EXECUTABLE_OUTPUT_PATH
	CMAKE_OSX_ARCHITECTURES
	CMAKE_OSX_SYSROOT
)

## Reveal library type choice to users
option(BUILD_SHARED_LIBS "Produce dynamic library instead of static archive" ON)

## Add iconv to include directories

if(ANDROID)
  find_library(iconv iconv)
else()
  find_package(ICONV REQUIRED)
endif()
include_directories(${ICONV_INCLUDE_DIR})

## Check iconv_const

include(CheckCXXSourceCompiles)

set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${ICONV_INCLUDE_DIR})
check_cxx_source_compiles(
	"
	#include <iconv.h>
	// this declaration will fail when there already exists a non const char** version which returns size_t
	double iconv(iconv_t cd,  char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft);
	int main() { return 0; }
	"
	HAVE_ICONV_CONST
)

# TODO: better way of config

if(HAVE_ICONV_CONST)
  add_definitions(-DHAVE_ICONV_CONST)
else(HAVE_ICONV_CONST)
  remove_definitions(-DHAVE_ICONV_CONST)
endif(HAVE_ICONV_CONST)

## TinyGetText library compilation

## build list of source files

file(GLOB TINYGETTEXT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
file(GLOB TINYGETTEXT_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} include/tinygettext/*.hpp)

## define a target for building the library

add_library(tinygettext ${TINYGETTEXT_SOURCES})

## Add tinygettext dir to search path

include_directories(include/)

## Debug options

option(WERROR "Stops on first compiler warning in debug mode" OFF)
option(WARNINGS "Enable long list of warnings for compiler to check" ON)

add_definitions(-std=c++0x -O3 -Wall -Wextra -Weffc++ -pedantic)

if(WARNINGS)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_definitions(
      -Weverything

      # flags that we deliberately ignore
      -Wno-c++98-compat
      -Wno-padded
      -Wno-weak-vtables
      -Wno-documentation
      -Wno-documentation-unknown-command
      -Wno-exit-time-destructors
      -Wno-zero-as-null-pointer-constant
      )
  elseif(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(
      -Wctor-dtor-privacy
      -Wold-style-cast
      -Woverloaded-virtual
      -Wsign-promo -Wswitch-enum
      -Wcast-align  -Wcast-qual
      -Wdisabled-optimization
      -Wfloat-equal
      -Wformat=2
      -Winit-self
      -Winvalid-pch
      -Wmissing-format-attribute  -Wmissing-include-dirs -Wmissing-noreturn
      -Wpacked
      -Wredundant-decls
      -Wshadow
      -Wsign-conversion  -Wstack-protector
      -Wstrict-overflow=5
      -Wswitch-default  -Wswitch-enum
      -Wundef
      -Wconversion
      -Wstrict-null-sentinel
      -Wlogical-op
      -Wunsafe-loop-optimizations)
  endif()
endif(WARNINGS)

if(WERROR)
  add_definitions(-Werror)
endif(WERROR)

## Extra definitions

add_definitions(-DVERSION=${VERSION})

## Generate test executables in the right place

set(EXECUTABLE_OUTPUT_PATH ${tinygettext_BINARY_DIR}/test)

## Build tinygettext tests

foreach(TEST tinygettext_test po_parser_test)
  ## Add target for tinygettext test
  add_executable(${TEST} test/${TEST}.cpp)
  ## Link with tinygettext library
  target_link_libraries(${TEST} tinygettext)
  target_link_libraries(${TEST} ${ICONV_LIBRARY})
endforeach(TEST)

## Install tinygettext

# use standardized variable name
set(LIB_SUBDIR "lib${LIB_SUFFIX}"
	CACHE STRING "Subdirectory of prefix into which libraries are installed (e.g., lib32, lib64)")

## prepare tinygettext.pc
configure_file(tinygettext.pc.in tinygettext.pc @ONLY)

install(TARGETS tinygettext
	ARCHIVE DESTINATION ${LIB_SUBDIR}
	LIBRARY DESTINATION ${LIB_SUBDIR})
install(FILES ${TINYGETTEXT_HEADERS}
	DESTINATION include/tinygettext)
install(FILES ${tinygettext_BINARY_DIR}/tinygettext.pc
	DESTINATION ${LIB_SUBDIR}/pkgconfig)

# EOF #
