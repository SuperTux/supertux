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

cmake_minimum_required(VERSION 3.19)

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

if(CMAKE_BUILD_TYPE MATCHES "Rel")
  set(SUPERTUX_RELEASE_DEFAULT TRUE)
else()
  set(SUPERTUX_RELEASE_DEFAULT FALSE)
endif()
option(IS_SUPERTUX_RELEASE "Build as official SuperTux release" ${SUPERTUX_RELEASE_DEFAULT})

option(STEAM_BUILD "Prepare build for Steam" OFF)

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

## Check platform-dependent build options
include(ConfigureChecks)

## Some additional compiler switches
include(SuperTux/ClangTidy)
include(SuperTux/WarningFlags)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/lib")
endif()

# Fedora Linux stores the glm CMake config files in /usr/share/cmake/glm/.
# If you wanna know how this hack works, read this:
# https://cmake.org/cmake/help/v3.31/command/find_package.html#config-mode-search-procedure
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  list(APPEND CMAKE_PREFIX_PATH "/usr")
endif()

## Add lots of dependencies to compiler switches
include(SuperTux/AddPackage)

# Find dependencies
add_package(TARGET SDL2
  PKG SDL2 PKG_USE SDL2::SDL2 CONFIG REQUIRED PKG_CONFIG sdl2 SDL2)
add_package(TARGET PNG
  PKG PNG PKG_USE PNG::PNG PKG_CONFIG libpng REQUIRED) # lol
add_package(TARGET SDL2_image
  PKG SDL2_image PKG_USE SDL2_image::SDL2_image CONFIG REQUIRED PKG_CONFIG SDL2_image sdl2_image)
add_package(TARGET SDL2_ttf
  PKG SDL2_ttf PKG_USE SDL2_ttf::SDL2_ttf CONFIG REQUIRED PKG_CONFIG SDL2_ttf sdl2_ttf)
add_package(TARGET LibCURL
  PKG CURL PKG_USE CURL::libcurl PKG_CONFIG libcurl)
add_package(TARGET OpenAL
  PKG OpenAL PKG_USE OpenAL::OpenAL CONFIG REQUIRED PKG_CONFIG OpenAL openal)
add_package(TARGET Ogg
  PKG Ogg PKG_USE Ogg::ogg CONFIG REQUIRED PKG_CONFIG ogg)
add_package(TARGET Vorbis
  PKG Vorbis PKG_USE Vorbis::vorbis CONFIG REQUIRED PKG_CONFIG vorbis)
add_package(TARGET VorbisFile
  PKG Vorbis PKG_USE Vorbis::vorbisfile CONFIG REQUIRED PKG_CONFIG vorbisfile)
add_package(TARGET PhysFS
  PKG PhysFS PKG_USE PhysFS::PhysFS CONFIG REQUIRED PKG_CONFIG physfs
  PREFER_PKGCONFIG)
add_package(TARGET ZLIB
  PKG ZLIB PKG_USE ZLIB::ZLIB PKG_CONFIG zlib REQUIRED)
add_package(TARGET fmt
  PKG fmt PKG_USE fmt::fmt CONFIG REQUIRED PKG_CONFIG fmt)
add_package(TARGET glm
  PKG glm PKG_USE glm::glm CONFIG REQUIRED PKG_CONFIG glm)

add_subdirectory(external/sexp-cpp EXCLUDE_FROM_ALL)
add_subdirectory(external/SDL_SavePNG EXCLUDE_FROM_ALL)
add_subdirectory(external/simplesquirrel EXCLUDE_FROM_ALL)
add_subdirectory(external/partio_zip EXCLUDE_FROM_ALL)
add_subdirectory(external/findlocale EXCLUDE_FROM_ALL)
add_subdirectory(external/obstack EXCLUDE_FROM_ALL)

# Workaround tinygettext wanting Iconv
set(HAVE_SDL YES CACHE BOOL "Use SDL_iconv instead of system iconv")
add_subdirectory(external/tinygettext EXCLUDE_FROM_ALL)

if(ENABLE_DISCORD)
  add_subdirectory(external/discord-sdk EXCLUDE_FROM_ALL)
endif()

option(ENABLE_OPENGL "Enable OpenGL support" ON)
set(HAVE_OPENGL NO)
if(ENABLE_OPENGL)
  set(OpenGL_GL_PREFERENCE "LEGACY")
  find_package(OpenGL)

  if(OPENGL_FOUND)
    find_package(GLEW REQUIRED)
  endif()

  if(GLEW_FOUND)
    set(HAVE_OPENGL YES)
    message(STATUS "OpenGL & GLEW found, enabling OpenGL support.")
  endif()
endif()

#include(SuperTux/ProvideOpenGL)

## Build stuff
include(SuperTux/BuildVersion)
include(SuperTux/BuildDocumentation)
include(SuperTux/BuildMessagePot)

## Build list of sources for supertux binary
set(SUPERTUX_SOURCES_C ${CMAKE_CURRENT_SOURCE_DIR})

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

# Set windowed mode when building in release
# because the console is very useful for debugging on Windows.
set(SUPERTUX_WIN32_OPTION)
if(IS_SUPERTUX_RELEASE OR STEAM_BUILD)
  set(SUPERTUX_WIN32_OPTION WIN32)
endif()

## Add target for supertux binary
add_executable(supertux2 ${SUPERTUX_WIN32_OPTION} ${CMAKE_BINARY_DIR}/version.h ${SUPERTUX_SOURCES_C} ${SUPERTUX_SOURCES_CXX} ${SUPERTUX_RESOURCES} src/main.cpp)
target_compile_features(supertux2 PUBLIC cxx_std_17)

if(IS_SUPERTUX_RELEASE OR STEAM_BUILD)
  target_compile_definitions(supertux2 PUBLIC SUPERTUX_RELEASE)
endif()

# TODO Is this even needed?
# $<$<BOOL:WIN32>:{CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.rc>
target_include_directories(supertux2 PUBLIC ${CMAKE_BINARY_DIR} src/)

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD|NetBSD")
  target_link_options(supertux2 PUBLIC -lexecinfo)
endif()

if(WIN32)
  include(SuperTux/Win32)
endif()

set_target_properties(supertux2 PROPERTIES OUTPUT_NAME "supertux2")
set_target_properties(supertux2 PROPERTIES COMPILE_FLAGS "${SUPERTUX2_EXTRA_WARNING_FLAGS}")

# Include altivec wrapper on ppc
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^ppc.*")
  target_include_directories(supertux2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/ppc)
endif()

## Link supertux binary with squirrel and other libraries
if(MSVC)
  target_link_libraries(supertux2 PUBLIC simplesquirrel_static)
else()
  target_link_libraries(supertux2 PUBLIC simplesquirrel)
endif()
target_link_libraries(supertux2 PUBLIC
  tinygettext sexp SDL_SavePNG
  PartioZip OpenAL FindLocale obstack glm fmt PhysFS)
target_compile_definitions(supertux2 PUBLIC GLM_ENABLE_EXPERIMENTAL)

if(NOT EMSCRIPTEN)
  target_link_libraries(supertux2 PUBLIC
    # SDL2_image
    SDL2_image
    # SDL2_ttf
    SDL2_ttf
    # SDL2 main (windows?)
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    # SDL2
    SDL2
  )
  target_link_libraries(supertux2 PUBLIC
    Ogg
    Vorbis
    VorbisFile
  )
  target_link_libraries(supertux2 PUBLIC LibCURL)
endif()

if(HAVE_OPENGL)
  target_link_libraries(supertux2 PUBLIC OpenGL::GL GLEW::GLEW)
endif()

if(ENABLE_DISCORD)
  target_link_libraries(supertux2 PUBLIC discord-rpc)
endif()

## Install stuff
include(SuperTux/BuildInstall)

## Create config.h now that INSTALL_SUBDIR_* have been set.
configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h)

## Build tests
# TODO Finish porting tests to CTest
set(BUILD_TESTING OFF)
if(BUILD_TESTING)
  #include(SuperTux/BuildTests)
  set(CMAKE_CTEST_ARGUMENTS "--output-on-failure")
  enable_testing()
  add_subdirectory(tests)
endif()

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
