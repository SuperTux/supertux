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

cmake_minimum_required(VERSION 3.14)

## Project name to use as command prefix.

project(SUPERTUX)

### CMake configuration

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Stop CMake from whining about 3.5 or future versions that other FindXXXXX.cmake files (seen from vcpkg) use
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

if(${PLATFORM} MATCHES "arm64")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_ARM64_ /DMY_CPU_LE")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D_ARM64_ /DMY_CPU_LE")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/mk/cmake)
include(ConfigureFiles)
include(ExternalProject)
include(CheckCXXCompilerFlag)
include(CheckSymbolExists)

## For autopackage
set(APPDATADIR "${CMAKE_INSTALL_PREFIX}/share/games/supertux2")

set(BUILD_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")
set(BUILD_CONFIG_DATA_DIR "${CMAKE_BINARY_DIR}/data")

## Add definitions
if(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
  add_definitions(-DRELEASE) # FIXME Unnecessary?
elseif(CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(-DDEBUG)
endif()

# Options for install
if(WIN32 AND NOT UNIX)
  set(INSTALL_SUBDIR_BIN "bin" CACHE STRING "Installation subdir for binaries")
  set(INSTALL_SUBDIR_SHARE "data" CACHE STRING "Installation subdir for data")
  set(INSTALL_SUBDIR_DOC "doc" CACHE STRING "Installation subdir for docs")
else()
  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND DISABLE_CPACK_BUNDLING)
    set(INSTALL_SUBDIR_BIN "SuperTux.app/Contents/MacOS" CACHE STRING "Installation subdir for binaries")
    set(INSTALL_SUBDIR_SHARE "SuperTux.app/Contents/Resources/data" CACHE STRING "Installation subdir for data")
    set(INSTALL_SUBDIR_DOC "SuperTux.app/Contents/Resources" CACHE STRING "Installation subdir for docs")
  else()
    set(INSTALL_SUBDIR_BIN "games" CACHE STRING "Installation subdir for binaries")
    set(INSTALL_SUBDIR_SHARE "share/games/supertux2" CACHE STRING "Installation subdir for data")
    set(INSTALL_SUBDIR_DOC "share/doc/supertux2" CACHE STRING "Installation subdir for docs")
  endif()
endif()

if(EMSCRIPTEN)
  include(SuperTux/Emscripten)
endif()

if(EMSCRIPTEN OR UBUNTU_TOUCH OR ANDROID)
  option(REMOVE_QUIT_BUTTON "Remove the option to quit the game (useful on mobile devices)" ON)
else()
  option(REMOVE_QUIT_BUTTON "Remove the option to quit the game (useful on mobile devices)" OFF)
endif()

option(STEAM_BUILD "Prepare build for Steam" OFF)
option(IS_SUPERTUX_RELEASE "Build as official SuperTux release" OFF)

set(SUPERTUX_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
set(VCPKG_BUILD OFF CACHE BOOL "Use dependencies installed via vcpkg (not dependency package)")
set(VCPKG_APPLOCAL_DEPS ${VCPKG_BUILD} BOOL)

# Detect mobile builds
option(UBUNTU_TOUCH "Compile the project for an Ubuntu Touch target" OFF)

# Mobile builds
if(UBUNTU_TOUCH OR ANDROID)
  option(HIDE_NONMOBILE_OPTIONS "Hide options that are impractical on mobile devices (e. g. changing screen resolution)" ON)
else()
  option(HIDE_NONMOBILE_OPTIONS "Hide options that are impractical on mobile devices (e. g. changing screen resolution)" OFF)
endif()

# Configure main menu logo
if(("${SUPERTUX_VERSION_STRING}" MATCHES "^v[0-9]+\\.[0-9]+\\.[0-9]+$") OR IS_SUPERTUX_RELEASE OR STEAM_BUILD)
  set(LOGO_FILE "logo.png")
else()
  set(LOGO_FILE "logo_dev.png")
endif()

if(WIN32)
  include(SuperTux/Win32)
endif()

## Check platform-dependent build options
include(ConfigureChecks)

## Some additional compiler switches
include(SuperTux/ClangTidy)
include(SuperTux/WarningFlags)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/lib")
endif()

## Add lots of dependencies to compiler switches
#include(SuperTux/ProvideTinygettext)
#include(SuperTux/ProvideSDL2_ttf) # add back?

find_package(PkgConfig)

# Find dependencies
find_package(SDL2 CONFIG REQUIRED)
#find_package(PNG CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(CURL)
find_package(OpenAL CONFIG REQUIRED)
find_package(Ogg CONFIG)
find_package(Vorbis CONFIG)
if (NOT (Ogg_FOUND OR Vorbis_FOUND))
	message("OGG (or Vorbis) Not Found! Trying backup")
	if (PkgConfig_FOUND)
		pkg_check_modules(Ogg REQUIRED IMPORTED_TARGET ogg)
		pkg_check_modules(Vorbis REQUIRED IMPORTED_TARGET vorbis vorbisfile)
		#pkg_check_modules(VorbisFile REQUIRED IMPORTED_TARGET vorbisfile)
		
	else()
		message(FATAL_ERROR "Could not find OGG. Giving up")
	endif()
endif()
find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(PhysFS CONFIG REQUIRED)

include(SuperTux/ProvideSavePNG)
include(SuperTux/ProvideSimpleSquirrel)
include(SuperTux/ProvideDiscord)
include(SuperTux/ProvideSexpcpp)
include(SuperTux/ProvidePartioZip)
include(SuperTux/ProvideOpenGL)
include(SuperTux/ProvideTinygettext)

## Build stuff
include(SuperTux/BuildVersion)
include(SuperTux/BuildDocumentation)
include(SuperTux/BuildMessagePot)

## Build list of sources for supertux binary
file(GLOB SUPERTUX_SOURCES_C RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} external/obstack/*.c external/findlocale/findlocale.c)

file(GLOB SUPERTUX_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*/*.cpp src/supertux/menu/*.cpp src/video/sdl/*.cpp src/video/null/*.cpp)
file(GLOB SUPERTUX_RESOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${PROJECT_BINARY_DIR}/tmp/*.rc")

if(HAVE_OPENGL)
  file(GLOB SUPERTUX_OPENGL_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/video/gl/*.cpp)
  set(SUPERTUX_SOURCES_CXX ${SUPERTUX_SOURCES_CXX} ${SUPERTUX_OPENGL_SOURCES})
endif()

## Sort source lists to have deterministic linking order
list(SORT SUPERTUX_SOURCES_C)
list(SORT SUPERTUX_SOURCES_CXX)
list(SORT SUPERTUX_RESOURCES)

include(SuperTux/CompileAmalgation)

## Generate supertux executable in the right place
# Is this needed? - Swagtoy
#set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
#set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

## Add target for supertux binary

add_executable(supertux2 WIN32 ${CMAKE_BINARY_DIR}/version.h ${SUPERTUX_SOURCES_C} ${SUPERTUX_SOURCES_CXX} ${SUPERTUX_RESOURCES} src/main.cpp)
# TODO Is this even needed?
# $<$<BOOL:WIN32>:{CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.rc>
target_include_directories(supertux2 PUBLIC ${CMAKE_BINARY_DIR} src/)

set_target_properties(supertux2 PROPERTIES OUTPUT_NAME "supertux2")
set_target_properties(supertux2 PROPERTIES COMPILE_FLAGS "${SUPERTUX2_EXTRA_WARNING_FLAGS}")

#if(WIN32 AND NOT VCPKG_BUILD)
#if(NOT MINGW)
  ## Copy dlls on windows
#  add_custom_command(TARGET supertux2_lib POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#    "${DEPENDENCY_FOLDER}/dll"
#    $<TARGET_FILE_DIR:supertux2_lib>)
#endif()
#endif()

## Some additional include paths
target_include_directories(supertux2 SYSTEM PUBLIC
  external/findlocale/
  external/obstack/
  )

# Include altivec wrapper on ppc
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^ppc.*")
  target_include_directories(supertux2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/ppc)
endif()

## Link supertux binary with squirrel and other libraries

target_link_libraries(supertux2 PUBLIC LibSimpleSquirrel)
target_link_libraries(supertux2 PUBLIC LibTinygettext)
target_link_libraries(supertux2 PUBLIC LibSexp)
target_link_libraries(supertux2 PUBLIC LibSavePNG)
target_link_libraries(supertux2 PUBLIC LibPartioZip)
target_link_libraries(supertux2 PUBLIC OpenAL::OpenAL)
target_link_libraries(supertux2 PUBLIC glm::glm)
target_compile_definitions(supertux2 PUBLIC GLM_ENABLE_EXPERIMENTAL)
target_link_libraries(supertux2 PUBLIC fmt::fmt)
target_link_libraries(supertux2 PUBLIC $<IF:$<TARGET_EXISTS:PhysFS::PhysFS>,PhysFS::PhysFS,PhysFS::PhysFS-static>)

if(NOT EMSCRIPTEN)
  target_link_libraries(supertux2 PUBLIC
      # SDL2_image
      $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
	  # SDL2_ttf
	  $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
	  # SDL2 main (windows?)
	  $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
	  # SDL2
      $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>)
  target_link_libraries(supertux2 PUBLIC
  	$<IF:$<TARGET_EXISTS:Ogg::ogg>,Ogg:ogg,PkgConfig::Ogg>
	$<IF:$<TARGET_EXISTS:Vorbis::vorbis>,Vorbis::vorbis Vorbis::vorbisfile,
		PkgConfig::Vorbis>
  )
  target_link_libraries(supertux2 PUBLIC CURL::libcurl)
endif()

if(HAVE_OPENGL)
  target_link_libraries(supertux2 PUBLIC LibOpenGL)
endif()

if(ENABLE_DISCORD)
  target_link_libraries(supertux2 PUBLIC LibDiscord)
endif()

## Install stuff
include(SuperTux/BuildInstall)

## Create config.h now that INSTALL_SUBDIR_* have been set.
configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h )

## Build tests
# TODO Move to CMake Tests
#include(SuperTux/BuildTests)

## CPack/Installation-specific stuff
include(SuperTux/BuildCPack)

# move some config clutter to the advanced section
mark_as_advanced(
  INSTALL_SUBDIR_BIN
  INSTALL_SUBDIR_SHARE
  INSTALL_SUBDIR_DOC
  )

mark_as_advanced(
  CMAKE_BACKWARDS_COMPATIBILITY
  CMAKE_BUILD_TYPE
  CMAKE_INSTALL_PREFIX
  EXECUTABLE_OUTPUT_PATH
  LIBRARY_OUTPUT_PATH
  CMAKE_OSX_ARCHITECTURES
  CMAKE_OSX_SYSROOT
  )

mark_as_advanced(
  APPDATADIR
  )

# EOF #
