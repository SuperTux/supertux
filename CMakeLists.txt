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

# Stop CMake from whining about 3.5 or future versions
set(CMAKE_WARN_DEPRECATED OFF)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "")

cmake_minimum_required(VERSION 3.22)

## Project name to use as command prefix.

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/mk/cmake)
if(ANDROID)
  include(SuperTux/Android)
endif()

project(SUPERTUX
        LANGUAGES C CXX
        VERSION 0.7.0)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0069 NEW)
endif()

### CMake configuration

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm64" AND MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_ARM64_ /DMY_CPU_LE")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D_ARM64_ /DMY_CPU_LE")
endif()

include(ExternalProject)
include(CheckCXXCompilerFlag)
include(CheckSymbolExists)

## Set version strings, detect git version, and other things related to versions
include(SuperTux/BuildVersion)

## For autopackage
set(APPDATADIR "${CMAKE_INSTALL_PREFIX}/share/games/supertux2")

set(BUILD_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")
set(BUILD_CONFIG_DATA_DIR "${CMAKE_BINARY_DIR}/data")

## Options for install
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

option(SUPERTUX_CCACHE "Whether to prefer ccache" OFF)
if(SUPERTUX_CCACHE)
  find_program(CCACHE_PATH ccache)
  if(CCACHE_PATH)
    message(STATUS "Using CCache: ${CCACHE_PATH}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PATH}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PATH}")
  endif()
endif()

if(EMSCRIPTEN)
  option(REMOVE_QUIT_BUTTON "Remove the option to quit the game (useful on mobile devices)" ON)
else()
  option(REMOVE_QUIT_BUTTON "Remove the option to quit the game (useful on mobile devices)" OFF)
endif()

if(IS_SUPERTUX_RELEASE)
  option(STEAM_BUILD "Prepare build for Steam" OFF)
endif()

option(SUPERTUX_LTO "Use link-time optimizations (Takes more RAM at compilation, gives smaller and faster executables)" OFF)

# Mobile builds
if(ANDROID)
  option(HIDE_NONMOBILE_OPTIONS "Hide options that are impractical on mobile devices (e. g. changing screen resolution)" ON)
else()
  option(HIDE_NONMOBILE_OPTIONS "Hide options that are impractical on mobile devices (e. g. changing screen resolution)" OFF)
endif()

## Determine whether static linking for SimpleSquirrel should be defaulted to
if(MSVC OR ANDROID)
  option(USE_STATIC_SIMPLESQUIRREL "Statically link the SimpleSquirrel library" ON)
else()
  option(USE_STATIC_SIMPLESQUIRREL "Statically link the SimpleSquirrel library" OFF)
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

if(MINGW)
  add_compile_options(-Wa,-mbig-obj)
endif()

# Fedora Linux stores the glm CMake config files in /usr/share/cmake/glm/.
# If you wanna know how this hack works, read this:
# https://cmake.org/cmake/help/v3.31/command/find_package.html#config-mode-search-procedure
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  list(APPEND CMAKE_PREFIX_PATH "/usr")
endif()

option(ENABLE_OPENGL "Enable OpenGL support" ON)

# CI builds should never rely on this, as system SDL2_TTF has no RAQM support, causing Arabic to not be rendered properly.
option(USE_SYSTEM_SDL2_TTF "Build SuperTux with system SDL2_TTF" OFF)

# Find dependencies
include(SuperTux/AddPackage)

add_package(TARGET PNG
  PKG PNG PKG_USE PNG::PNG PKG_CONFIG libpng REQUIRED)
add_package(TARGET ZLIB
  PKG ZLIB PKG_USE ZLIB::ZLIB PKG_CONFIG zlib REQUIRED)
add_package(TARGET PhysFS
  PKG PhysFS PKG_USE PhysFS::PhysFS CONFIG REQUIRED PKG_CONFIG physfs PhysicsFS
  PREFER_PKGCONFIG)
add_package(TARGET fmt
  PKG fmt PKG_USE fmt::fmt CONFIG REQUIRED PKG_CONFIG fmt)
add_package(TARGET glm
  PKG glm PKG_USE glm::glm CONFIG REQUIRED PKG_CONFIG glm)
add_package(TARGET Freetype
  PKG Freetype PKG_USE Freetype::Freetype CONFIG REQUIRED PKG_CONFIG freetype2)
if(NOT USE_SYSTEM_SDL2_TTF)
  # System-installed SDL2_TTF will not have RAQM support, so no need to look for that.
  add_package(TARGET RAQM
    PKG RAQM PKG_USE RAQM CONFIG PKG_CONFIG libraqm raqm)
endif()

if(NOT EMSCRIPTEN)
  add_package(TARGET SDL2
    PKG SDL2 PKG_USE SDL2::SDL2 CONFIG REQUIRED PKG_CONFIG sdl2 SDL2
    ${SUPERTUX_ANDROID_PREFER_PKGCONF})
  add_package(TARGET SDL2_image
    PKG SDL2_image PKG_USE SDL2_image::SDL2_image CONFIG REQUIRED PKG_CONFIG SDL2_image sdl2_image
    ${SUPERTUX_ANDROID_PREFER_PKGCONF})
  if (USE_SYSTEM_SDL2_TTF)
    add_package(TARGET SDL2_ttf
      PKG SDL2_ttf PKG_USE SDL2_ttf::SDL2_ttf CONFIG REQUIRED PKG_CONFIG SDL2_ttf sdl2_ttf)
  endif()
  add_package(TARGET libcurl
    PKG CURL PKG_USE CURL::libcurl PKG_CONFIG libcurl)
  add_package(TARGET OpenAL
    PKG OpenAL PKG_USE OpenAL::OpenAL CONFIG REQUIRED PKG_CONFIG OpenAL openal)
  add_package(TARGET Ogg
    PKG Ogg PKG_USE Ogg::ogg CONFIG REQUIRED PKG_CONFIG ogg)
  add_package(TARGET Vorbis
    PKG Vorbis PKG_USE Vorbis::vorbis CONFIG REQUIRED PKG_CONFIG vorbis)
  add_package(TARGET VorbisFile
    PKG Vorbis PKG_USE Vorbis::vorbisfile CONFIG REQUIRED PKG_CONFIG vorbisfile)
else()
  include(SuperTux/Emscripten)
endif()

if(TARGET RAQM AND NOT USE_SYSTEM_SDL2_TTF)
  message(STATUS "Compiling SDL_ttf with RAQM")
  set(SDL2TTF_RAQM ON)
endif()

if(USE_SYSTEM_SDL2_TTF)
  message(WARNING "Building with system-wide SDL_TTF will cause bi-directional (e.g. Arabic) scriptures to not be displayed properly.")
endif()

add_subdirectory(external/sexp-cpp EXCLUDE_FROM_ALL)
if(NOT USE_SYSTEM_SDL2_TTF)
  add_subdirectory(external/SDL_ttf EXCLUDE_FROM_ALL)
endif()
add_subdirectory(external/SDL_SavePNG EXCLUDE_FROM_ALL)
add_subdirectory(external/simplesquirrel EXCLUDE_FROM_ALL)
add_subdirectory(external/partio_zip EXCLUDE_FROM_ALL)
add_subdirectory(external/findlocale EXCLUDE_FROM_ALL)
add_subdirectory(external/obstack EXCLUDE_FROM_ALL)
add_subdirectory(external/tinygettext EXCLUDE_FROM_ALL)

if(ENABLE_DISCORD)
  add_subdirectory(external/discord-sdk EXCLUDE_FROM_ALL)
endif()

set(HAVE_OPENGL NO)
# OpenGL for Emscripten is added in Emscripten.cmake
if(ENABLE_OPENGL AND NOT EMSCRIPTEN)
  set(OpenGL_GL_PREFERENCE "LEGACY")
  find_package(OpenGL)

  if(OPENGL_FOUND)
    add_package(TARGET GLEW
      PKG GLEW PKG_USE GLEW::GLEW CONFIG REQUIRED PKG_CONFIG glew
      PREFER_PKGCONFIG)
  else()
    message(STATUS "OpenGL not found! OpenGL support is disabled.")
  endif()

  set(HAVE_OPENGL "${ADDPKG_GLEW_FOUND}")

  if(HAVE_OPENGL AND OPENGL_FOUND)
    message(STATUS "OpenGL & GLEW found! enabling OpenGL support.")
  elseif(OPENGL_FOUND)
    message(STATUS "Glew not found! OpenGL support is disabled.")
  endif()
endif()

## Build stuff
include(SuperTux/BuildDocumentation)
include(SuperTux/BuildMessagePot)

## Build list of sources for supertux binary
set(SUPERTUX_SOURCES_C ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB SUPERTUX_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*/*.cpp src/supertux/menu/*.cpp src/video/sdl/*.cpp src/video/null/*.cpp)
file(GLOB SUPERTUX_SOURCES_HXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*/*.hpp src/supertux/menu/*.hpp src/video/sdl/*.hpp src/video/null/*.hpp)
file(GLOB SUPERTUX_RESOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${PROJECT_BINARY_DIR}/tmp/*.rc")

if(HAVE_OPENGL)
  message(STATUS "Adding OpenGL sources")
  file(GLOB SUPERTUX_OPENGL_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/video/gl/*.cpp)
  file(GLOB SUPERTUX_OPENGL_SOURCES_HXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/video/gl/*.hpp)
  set(SUPERTUX_SOURCES_CXX ${SUPERTUX_SOURCES_CXX} ${SUPERTUX_OPENGL_SOURCES_CXX})
  set(SUPERTUX_SOURCES_HXX ${SUPERTUX_SOURCES_HXX} ${SUPERTUX_OPENGL_SOURCES_HXX})
endif()

## Sort source lists to have deterministic linking order
list(SORT SUPERTUX_SOURCES_C)
list(SORT SUPERTUX_SOURCES_CXX)
list(SORT SUPERTUX_RESOURCES)

include(SuperTux/CompileAmalgation)

## Add target for supertux binary
if(ANDROID)
  add_library(supertux2 SHARED ${CMAKE_BINARY_DIR}/version.h ${SUPERTUX_SOURCES_C} ${SUPERTUX_SOURCES_CXX} ${SUPERTUX_SOURCES_HXX} src/main.cpp)
else()
  add_executable(supertux2 ${SUPERTUX_WIN32_OPTION} ${CMAKE_BINARY_DIR}/version.h ${SUPERTUX_SOURCES_C} ${SUPERTUX_SOURCES_CXX} ${SUPERTUX_SOURCES_HXX} ${SUPERTUX_RESOURCES} src/main.cpp)
endif()

option(SUPERTUX_PCH "Pre-compile headers (faster compilation)" ${IS_SUPERTUX_RELEASE})

## Apply pre-compiled headers for faster compilation
if(SUPERTUX_PCH)
  message(STATUS "Setting pre-compiled headers")
  set(proj_headers
    "src/audio/sound_manager.hpp"
    "src/control/input_manager.hpp"
    "src/math/anchor_point.hpp"
    "src/math/vector.hpp"
    "src/math/size.hpp"
    "src/math/sizef.hpp"
    "src/math/util.hpp"
    "src/editor/object_option.hpp"
    "src/object/path_walker.hpp"
    "src/object/player.hpp"
    "src/object/spawnpoint.hpp"
    "src/object/camera.hpp"
    "src/sprite/sprite.hpp"
    "src/editor/object_settings.hpp"
    "src/badguy/badguy.hpp"
    "src/gui/menu.hpp"
    "src/gui/menu_manager.hpp"
    "src/util/log.hpp"
    "src/util/file_system.hpp"
    "src/util/reader_document.hpp"
    "src/util/reader_mapping.hpp"
    "src/video/surface.hpp"
    "src/video/video_system.hpp"
    "src/video/compositor.hpp"
    "src/video/drawing_context.hpp"
    "src/video/viewport.hpp"
    "src/editor/editor.hpp"
    "src/object/moving_sprite.hpp"
    "src/supertux/moving_object.hpp"
    "src/supertux/game_object.hpp"
    "src/supertux/sector_base.hpp"
    "src/supertux/sector.hpp"
    "src/supertux/level.hpp"
    "src/supertux/tile.hpp"
    "src/supertux/tile_manager.hpp"
    "src/supertux/world.hpp"
    "src/supertux/screen_manager.hpp"
    "src/supertux/game_object_manager.hpp"
  )
  list(TRANSFORM proj_headers PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

  target_precompile_headers(supertux2 PRIVATE
    # Standard library headers
    <string>
    <vector>
    <memory>
    <algorithm>
    <functional>
    <unordered_map>
    <fstream>
    # GLM library headers
    <glm/glm.hpp>
    <glm/ext.hpp>
    <glm/gtx/io.hpp>
    # PhysicsFS
    <physfs.h>
    # SDL
    <SDL.h>
    <SDL_ttf.h>
    <SDL_image.h>
    # FMT
    <fmt/format.h>
    "${proj_headers}"
  )
endif()

target_include_directories(supertux2 PUBLIC ${CMAKE_BINARY_DIR} src/)

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD|NetBSD")
  target_link_options(supertux2 PUBLIC -lexecinfo)
endif()

if(WIN32)
  include(SuperTux/Win32)
endif()

if(EMSCRIPTEN)
  target_link_options(supertux2 PUBLIC -sEXPORTED_FUNCTIONS=['_main','_set_resolution','_save_config','_onDownloadProgress','_onDownloadFinished','_onDownloadError','_onDownloadAborted','_getExceptionMessage'] PUBLIC -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap'])
endif()

set_target_properties(supertux2 PROPERTIES OUTPUT_NAME "supertux2")
set_target_properties(supertux2 PROPERTIES COMPILE_FLAGS "${SUPERTUX2_EXTRA_WARNING_FLAGS}")

# FIXME: As of writing, Emscripten fails with "wasm-ld: error: /path/to/emsdk/upstream/emscripten/cache/sysroot/lib/wasm32-emscripten/thinlto/libc.a(fileno.o): attempt to add bitcode file after LTO (fileno)"
# Seems to be a known issue: https://github.com/emscripten-core/emscripten/issues/20275
if(EMSCRIPTEN)
  set(SUPERTUX_LTO OFF CACHE BOOL "" FORCE)
endif()

if(SUPERTUX_LTO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT result)
  if(result)
    set_property(TARGET supertux2 PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
    set_property(TARGET squirrel PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
  endif()
else()
  set_property(TARGET supertux2 PROPERTY INTERPROCEDURAL_OPTIMIZATION OFF)
  set_property(TARGET squirrel PROPERTY INTERPROCEDURAL_OPTIMIZATION OFF)
endif()


# Include altivec wrapper on ppc
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^ppc.*")
  target_include_directories(supertux2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/ppc)
endif()

## Link supertux binary with squirrel and other libraries
if(USE_STATIC_SIMPLESQUIRREL)
  target_link_libraries(supertux2 PUBLIC simplesquirrel_static)
else()
  target_link_libraries(supertux2 PUBLIC simplesquirrel)
endif()

target_link_libraries(supertux2 PUBLIC
  tinygettext sexp SDL_SavePNG SDL2_ttf
  PartioZip OpenAL FindLocale obstack glm fmt PhysFS)
target_compile_definitions(supertux2 PUBLIC GLM_ENABLE_EXPERIMENTAL)
if(NOT EMSCRIPTEN)
  target_link_libraries(supertux2 PUBLIC
    # SDL2_image
    SDL2_image
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

  if(ADDPKG_CURL_FOUND)
    target_link_libraries(supertux2 PUBLIC libcurl)
  endif()

  if(HAVE_OPENGL)
    target_link_libraries(supertux2 PUBLIC OpenGL::GL GLEW)
  endif()

  if(ENABLE_DISCORD)
    target_link_libraries(supertux2 PUBLIC discord-rpc)
  endif()
endif()

if(ANDROID)
  target_link_libraries(supertux2 PUBLIC log)
  target_compile_definitions(supertux2 PUBLIC -D__ANDROID__ -DANDROID)
endif()

## Install stuff
include(SuperTux/BuildInstall)

## Create config.h now that INSTALL_SUBDIR_* have been set.
configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h)

# Make android data.zip
if(ANDROID)
  set(ANDROID_PROJECT_DIR "${PROJECT_SOURCE_DIR}/mk/android/" CACHE PATH
          "Path to the root of the root of the android project")
  if(NOT EXISTS "${ANDROID_PROJECT_DIR}/app/src/main/assets/data.zip")
    get_filename_component(DATADIR ${PROJECT_SOURCE_DIR}/data REALPATH)
    if(NOT EXISTS ${ANDROID_PROJECT_DIR}/app/src/main/assets)
      file(MAKE_DIRECTORY ${ANDROID_PROJECT_DIR}/app/src/main/assets)
    endif()
    file(GLOB DATA_CONTENTS RELATIVE ${DATADIR} ${DATADIR}/*)
    #file(ARCHIVE_CREATE OUTPUT ${ANDROID_PROJECT_DIR}/app/src/main/assets/data.zip PATHS ${DATA_CONTENTS} FORMAT zip)
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar "cfv"
            "${ANDROID_PROJECT_DIR}/app/src/main/assets/data.zip"
            --format=zip
            ${DATA_CONTENTS}

            WORKING_DIRECTORY ${DATADIR}
    )
  endif()
endif()

## Build tests
# TODO Finish porting tests to CTest
option(BUILD_TESTING "Enable tests" OFF)
message(STATUS "Build with tests: ${BUILD_TESTING}")
if(BUILD_TESTING)
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
