cmake_minimum_required(VERSION 3.13.4)

project(SuperTux VERSION 0.6.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# This should be set only with windows Visual Studio generator builds
string(TOLOWER "${CMAKE_GENERATOR_PLATFORM}" PLATFORM)
if(${PLATFORM} MATCHES "arm64")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_ARM64_ /DMY_CPU_LE")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D_ARM64_ /DMY_CPU_LE")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/mk/cmake)
include(ConfigureFiles)
include(ExternalProject)

## For autopackage
set(APPDATADIR "${CMAKE_INSTALL_PREFIX}/share/games/supertux2")

set(BUILD_DATA_DIR "${PROJECT_SOURCE_DIR}/data")
set(BUILD_CONFIG_DATA_DIR "${PROJECT_BINARY_DIR}/data")

## Add definitions
if(CMAKE_BUILD_TYPE MATCHES Rel)
  option(IS_SUPERTUX_RELEASE "Build as official SuperTux release" ON)
  add_definitions(-DRELEASE)
else()
  option(IS_SUPERTUX_RELEASE "Build as official SuperTux release" OFF)
  add_definitions(-DDEBUG)
endif()

# Options for install
if(WIN32)
  set(INSTALL_SUBDIR_BIN "bin" CACHE STRING "Installation subdir for binaries")
  set(INSTALL_SUBDIR_SHARE "data" CACHE STRING "Installation subdir for data")
  set(INSTALL_SUBDIR_DOC "doc" CACHE STRING "Installation subdir for docs")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND DISABLE_CPACK_BUNDLING)
  set(INSTALL_SUBDIR_BIN "SuperTux.app/Contents/MacOS" CACHE STRING "Installation subdir for binaries")
  set(INSTALL_SUBDIR_SHARE "SuperTux.app/Contents/Resources/data" CACHE STRING "Installation subdir for data")
  set(INSTALL_SUBDIR_DOC "SuperTux.app/Contents/Resources" CACHE STRING "Installation subdir for docs")
else()
  set(INSTALL_SUBDIR_BIN "games" CACHE STRING "Installation subdir for binaries")
  set(INSTALL_SUBDIR_SHARE "share/games/supertux2" CACHE STRING "Installation subdir for data")
  set(INSTALL_SUBDIR_DOC "share/doc/supertux2" CACHE STRING "Installation subdir for docs")
endif()

if(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX .html)
  set(IS_EMSCRIPTEN_BUILD ON)

  set(EM_USE_FLAGS "-sDISABLE_EXCEPTION_CATCHING=0")
  set(EM_LINK_FLAGS " -sTOTAL_MEMORY=67108864 -sALLOW_MEMORY_GROWTH=1 -sERROR_ON_UNDEFINED_SYMBOLS=0 --preload-file ${BUILD_CONFIG_DATA_DIR} --use-preload-plugins -lidbfs.js")
  if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(EM_USE_FLAGS "${EM_USE_FLAGS} -fsanitize=undefined")
    set(EM_LINK_FLAGS "${EM_LINK_FLAGS} -fsanitize=undefined -sSAFE_HEAP=1 -sASSERTIONS=1 -sDEMANGLE_SUPPORT=1")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EM_USE_FLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EM_USE_FLAGS}")
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${EM_USE_FLAGS} ${EM_LINK_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${EM_USE_FLAGS} ${EM_LINK_FLAGS}")
endif()


# TODO: Add " OR ANDROID OR IOS" to this
if(EMSCRIPTEN OR UBUNTU_TOUCH OR ANDROID)
  option(REMOVE_QUIT_BUTTON "Remove the option to quit the game (useful on mobile devices)" ON)
else()
  option(REMOVE_QUIT_BUTTON "Remove the option to quit the game (useful on mobile devices)" OFF)
endif()

option(STEAM_BUILD "Prepare build for Steam" ${IS_SUPERTUX_RELEASE})

# Detect mobile builds
option(UBUNTU_TOUCH "Compile the project for an Ubuntu Touch target" OFF)

# Mobile builds
if(UBUNTU_TOUCH OR ANDROID)
  option(HIDE_NONMOBILE_OPTIONS "Hide options that are impractical on mobile devices (e. g. changing screen resolution)" ON)
else()
  option(HIDE_NONMOBILE_OPTIONS "Hide options that are impractical on mobile devices (e. g. changing screen resolution)" OFF)
endif()

if(WIN32)
  ## Enable multi-processor compilation (faster)
  if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  endif()
  
  ## And shut up about unsafe stuff
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  
  ## And fill in executable metadata
  configure_file("${PROJECT_SOURCE_DIR}/mk/msvc/icon_rc.template" "${PROJECT_BINARY_DIR}/tmp/icon.rc")
  
  ## And add a few defines
  add_definitions(-D_USE_MATH_DEFINES -DNOMINMAX -DWIN32)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DMACOSX)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/lib")
endif()

## Build stuff
include(SuperTux/BuildVersion)
include(SuperTux/BuildDocumentation)
include(SuperTux/BuildMessagePot)
include(SuperTux/BuildMiniswigWrapper)

## Globbing
file(GLOB SUPERTUX_RC ${PROJECT_BINARY_DIR}/tmp/*.rc)
file(GLOB_RECURSE SUPERTUX_SRC src/*.cpp src/*.hpp)
list(REMOVE_ITEM SUPERTUX_SRC src/main.cpp)

# TODO: that "HAVE_OPENGL" thingy

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/src/scripting/wrapper.cpp)
  list(APPEND SUPERTUX_SRC ${PROJECT_SOURCE_DIR}/src/scripting/wrapper.cpp)
endif()

## On Windows, add an icon
if(WIN32 AND MINGW)
  add_custom_command(
    OUTPUT ${PROJECT_BINARY_DIR}/supertux_rc.o
    COMMAND ${CMAKE_RC_COMPILER} -I${PROJECT_SOURCE_DIR}/data/images/engine/icons -i${PROJECT_SOURCE_DIR}/data/images/engine/icons/supertux.rc -o ${PROJECT_BINARY_DIR}/supertux_rc.o)
  list(APPEND SUPERTUX_SRC ${PROJECT_BINARY_DIR}/supertux_rc.o)
endif()

## Generate supertux executable in the right place

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

## Add target for supertux binary

add_library(supertux2_lib STATIC ${SUPERTUX_SRC})
target_include_directories(supertux2_lib PUBLIC src/)
if(WIN32)
  add_executable(supertux2 WIN32 ${} ${PROJECT_SOURCE_DIR}/data/images/engine/icons/supertux.rc)
else()
  add_executable(supertux2)
endif()

target_sources(supertux2 PUBLIC src/main.cpp)
target_link_libraries(supertux2 PUBLIC supertux2_lib)

#[=[
# Copy DLLs on Windows.
# If this doesn't execute, you don't have a great enough
# CMake version and will have to copy the DLLs manually.
# Sorry.
# ~ MatusGuy
if(WIN32)
  add_custom_command(TARGET supertux2_lib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_RUNTIME_DLLS:supertux2>
    $<TARGET_FILE_DIR:supertux2>
    COMMAND_EXPAND_LISTS
  )
endif()
#]=]

## Install stuff
include(SuperTux/BuildInstall)

## Create config.h now that INSTALL_SUBDIR_* have been set.
configure_file(config.h.cmake ${PROJECT_BINARY_DIR}/config.h )

## Configure main menu logo
if(IS_SUPERTUX_RELEASE OR STEAM_BUILD)
  set(LOGO_FILE "logo_final.sprite")
else()
  set(LOGO_FILE "logo.sprite")
endif()
configure_file(data/levels/misc/menu.stl.in ${PROJECT_BINARY_DIR}/data/levels/misc/menu.stl)

## Build tests
include(SuperTux/BuildTests)

## CPack/Installation-specific stuff
include(SuperTux/BuildCPack)

# move some config clutter to the advanced section
mark_as_advanced(
  INSTALL_SUBDIR_BIN
  INSTALL_SUBDIR_SHARE
  INSTALL_SUBDIR_DOC

  CMAKE_BACKWARDS_COMPATIBILITY
  CMAKE_BUILD_TYPE
  CMAKE_INSTALL_PREFIX
  EXECUTABLE_OUTPUT_PATH
  LIBRARY_OUTPUT_PATH
  CMAKE_OSX_ARCHITECTURES
  CMAKE_OSX_SYSROOT

  APPDATADIR
)

# EOF #