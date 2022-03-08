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

project(SUPERTUX)


### CMake configuration

cmake_minimum_required(VERSION 3.1)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0008 NEW)
  cmake_policy(SET CMP0023 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/mk/cmake)
include(ConfigureFiles)
include(ExternalProject)
include(CheckCXXCompilerFlag)

## For autopackage
set(APPDATADIR "${CMAKE_INSTALL_PREFIX}/share/games/supertux2")

set(BUILD_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")
set(BUILD_CONFIG_DATA_DIR "${CMAKE_BINARY_DIR}/data")
## Check endianess

if(NOT EMSCRIPTEN)
  # FIXME: Any reason why we need this?
  include(TestBigEndian)
  test_big_endian(WORDS_BIGENDIAN)
endif(NOT EMSCRIPTEN)

## Add definitions
if(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
  add_definitions(-DRELEASE)
elseif(CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(-DDEBUG)
endif()

if(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX .html)
  set(IS_EMSCRIPTEN_BUILD ON)

  set(USE_FLAGS " -sUSE_SDL=2 -sFULL_ES2=1 -sUSE_SDL_IMAGE=2 -sUSE_FREETYPE=2 -sUSE_PTHREADS=1 -sDISABLE_EXCEPTION_CATCHING=0 --preload-file ${BUILD_CONFIG_DATA_DIR} -sTOTAL_MEMORY=67108864 -sALLOW_MEMORY_GROWTH=1 -sSDL2_IMAGE_FORMATS='[\"png\",\"jpg\"]' --use-preload-plugins")
  if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(USE_FLAGS "${USE_FLAGS} -fsanitize=undefined -sSAFE_HEAP=1 -sASSERTIONS=1 -sDEMANGLE_SUPPORT=1")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS} -sERROR_ON_UNDEFINED_SYMBOLS=0")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS} -sERROR_ON_UNDEFINED_SYMBOLS=0")
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${USE_FLAGS} -sERROR_ON_UNDEFINED_SYMBOLS=0 -lidbfs.js")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${USE_FLAGS} -sERROR_ON_UNDEFINED_SYMBOLS=0 -lidbfs.js")
endif()

# TODO: Add " OR ANDROID OR IOS" to this
if(EMSCRIPTEN OR UBUNTU_TOUCH OR ANDROID)
  option(REMOVE_QUIT_BUTTON "Remove the option to quit the game (useful on mobile devices)" ON)
else()
  option(REMOVE_QUIT_BUTTON "Remove the option to quit the game (useful on mobile devices)" OFF)
endif()

option(STEAM_BUILD "Prepare build for Steam" OFF)

set(SUPERTUX_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
set(VCPKG_BUILD OFF CACHE BOOL "Use dependencies installed via vcpkg (not dependency package)")
set(VCPKG_APPLOCAL_DEPS ${VCPKG_BUILD} BOOL)

if(WIN32)
  include(BuildWin32)
endif(WIN32)

# Detect mobile builds
option(UBUNTU_TOUCH "Compile the project for an Ubuntu Touch target" OFF)

# Mobile builds
if (UBUNTU_TOUCH OR ANDROID)
  option(HIDE_NONMOBILE_OPTIONS "Hide options that are impractical on mobile devices (e. g. changing screen resolution)" ON)
else()
  option(HIDE_NONMOBILE_OPTIONS "Hide options that are impractical on mobile devices (e. g. changing screen resolution)" OFF)
endif()

## Add lots of dependencies to compiler switches
include(BuildBoost)

include(BuildGlm)

find_package(PNG REQUIRED)

include(BuildFmt)
include(BuildSDL2)

option(IS_SUPERTUX_RELEASE "Build as official SuperTux release" OFF)
option(BUILD_TESTS "Build test cases" OFF)

include(BuildOpenAL)

find_package(OggVorbis REQUIRED)
include_directories(SYSTEM ${VORBIS_INCLUDE_DIR})

include(CheckSymbolExists)

include(BuildPhysfs)
include(BuildCurl)

find_package(ZLIB REQUIRED)

#find_package(ICONV REQUIRED)
#include_directories(SYSTEM ${ICONV_INCLUDE_DIR})
#link_libraries(${ICONV_LIBRARY})

include(BuildVersion)
include(BuildDoxygen)

## Check platform-dependent build options

include(ConfigureChecks)

# Include altivec wrapper on ppc
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^ppc.*")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/ppc)
endif()

include(BuildSquirrel)
include(BuildTinygettext)
include(BuildSDL2_ttf)

## Some additional include paths

include_directories(${CMAKE_BINARY_DIR}/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/external/findlocale/)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/external/obstack/)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/external/sexp-cpp/include/)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL_SavePNG/)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/external/partio_zip/)

include(BuildDiscord)

if(WIN32)
  add_definitions(-D_USE_MATH_DEFINES -DNOMINMAX)
  add_definitions(-DWIN32)
endif(WIN32)

## Lint

if(CMAKE_VERSION VERSION_GREATER 3.6)
  if (USE_CLANG_TIDY)
    find_program(
      CLANG_TIDY_EXE
      NAMES "clang-tidy"
      DOC "Path to clang-tidy executable"
      )

    if(NOT CLANG_TIDY_EXE)
      message(FATAL_ERROR "clang-tidy not found.")
    else()
      message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
      set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    endif()
  endif()
endif()

## Build list of sources for supertux binary

file(GLOB SUPERTUX_SOURCES_C RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} external/obstack/*.c external/findlocale/findlocale.c)

file(GLOB SUPERTUX_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*/*.cpp src/supertux/menu/*.cpp src/video/sdl/*.cpp src/video/null/*.cpp)
file(GLOB SUPERTUX_RESOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${PROJECT_BINARY_DIR}/tmp/*.rc")

option(USE_SYSTEM_SEXPCPP "Use preinstalled sexp-cpp if available" ON)
if(USE_SYSTEM_SEXPCPP)
  find_package(sexp QUIET)
endif()

if(TARGET sexp)
  message(STATUS "Found preinstalled sexp-cpp")
else()
  if(USE_SYSTEM_SEXPCPP)
    message(STATUS "Could NOT find sexp-cpp, using external/sexp-cpp fallback")
  endif()

  if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/sexp-cpp/CMakeLists.txt)
    message(FATAL_ERROR "sexp-cpp submodule is not checked out or ${CMAKE_CURRENT_SOURCE_DIR}/external/sexp-cpp/CMakeLists.txt is missing")
  endif()
  file(GLOB SEXP_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} external/sexp-cpp/src/*.cpp)
  add_library(sexp STATIC ${SEXP_SOURCES_CXX})
  target_compile_definitions(sexp PRIVATE -DSEXP_USE_LOCALE)
endif()

file(GLOB SAVEPNG_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} external/SDL_SavePNG/savepng.c)
add_library(savepng STATIC ${SAVEPNG_SOURCES_CXX})
target_include_directories(savepng SYSTEM PUBLIC ${SDL2_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS})
target_link_libraries(savepng PUBLIC ${PNG_LIBRARIES})

add_library(partio_zip_lib STATIC ${CMAKE_CURRENT_SOURCE_DIR}/external/partio_zip/zip_manager.cpp)
target_include_directories(partio_zip_lib PUBLIC ${ZLIB_INCLUDE_DIRS})
target_link_libraries(partio_zip_lib PUBLIC ${ZLIB_LIBRARIES})

if(HAVE_OPENGL)
  file(GLOB SUPERTUX_OPENGL_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/video/gl/*.cpp)
  set(SUPERTUX_SOURCES_CXX ${SUPERTUX_SOURCES_CXX} ${SUPERTUX_OPENGL_SOURCES})
endif(HAVE_OPENGL)

## Sort source lists to have deterministic linking order
list(SORT SUPERTUX_SOURCES_C)
list(SORT SUPERTUX_SOURCES_CXX)
list(SORT SUPERTUX_RESOURCES)

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.cpp)
  set(SUPERTUX_SOURCES_CXX ${SUPERTUX_SOURCES_CXX} ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.cpp)
endif(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.cpp)

## Compile everything at once (roughly equivalent to cat *.cpp | gcc)

option(COMPILE_AMALGATION "Compile all the files together at once (experimental)" OFF)
if(COMPILE_AMALGATION)
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp "// Includes all source files of the project\n")
  foreach(ST_FILE ${SUPERTUX_SOURCES_CXX})
    file(RELATIVE_PATH CXX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src "${CMAKE_CURRENT_SOURCE_DIR}/${ST_FILE}")
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp "#include \"${CXX_FILE}\"\n")
  endforeach(ST_FILE)
  ## Build instead of CXX sources
  set(SUPERTUX_SOURCES_CXX ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp)
else(COMPILE_AMALGATION)
  if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp)
    file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp)
  endif(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp)
endif(COMPILE_AMALGATION)

## Debug options

option(WERROR "Stop on first compiler warning" OFF)
option(WARNINGS "Enable long list of warnings for compiler to check" OFF)
option(USE_CLANG_TIDY "Run clang-tidy" OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g" CACHE STRING "Debug flags")
  set(CMAKE_C_FLAGS_DEBUG "-O0 -g" CACHE STRING "Debug flags")
  set(CMAKE_CXX_FLAGS_PROFILE "-pg" CACHE STRING "Profile flags")
  set(CMAKE_C_FLAGS_PROFILE "-pg" CACHE STRING "Profile flags")
  set(CMAKE_LD_FLAGS_PROFILE "-lgmon" CACHE STRING "Profile flags")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -funit-at-a-time")
  if(WERROR)
    add_definitions(-Werror)
  else(WERROR)
    remove_definitions(-Werror)
  endif(WERROR)
  if(WARNINGS)
    # temporarily disabled:
    #   -Wsign-conversion -Wfloat-equal -Wundef -Wswitch-default
    #   -Wswitch-enum -Wsign-promo -Wcast-qual -Woverloaded-virtual -Wmissing-format-attribute
    #   -Wpadded -Wabi -Winline -Wunsafe-loop-optimizations -Wstrict-overflow=5
    # fails on MinGW:
    #   -ansi
    if(CMAKE_COMPILER_IS_GNUCXX)
      string(CONCAT SUPERTUX2_EXTRA_WARNING_FLAGS
        "-fdiagnostics-show-option "
        "-pedantic "
        "-Wno-long-long "
        "-Wcast-align "
        "-Wdisabled-optimization "
        "-Winit-self -Winvalid-pch "
        "-Wmissing-include-dirs "
        "-Wmissing-noreturn "
        "-Wpacked -Wredundant-decls "
        "-Wformat=2 "
        "-Weffc++ "
        "-Wctor-dtor-privacy "
        "-Wno-unused-parameter "
        "-Wshadow "
        "-Wnon-virtual-dtor "
        "-Wcast-qual "
        "-Wold-style-cast "
        "-Wzero-as-null-pointer-constant "
        "-Wconversion "
        "-Wlogical-op "
        "-Wstrict-null-sentinel "
        "-Wsuggest-override "
        )
      if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 8)
        string(CONCAT SUPERTUX2_EXTRA_WARNING_FLAGS
          "${SUPERTUX2_EXTRA_WARNING_FLAGS} "
          "-Wint-in-bool-context "
          )
      endif()
      if(Boost_VERSION LESS 106500)
        string(CONCAT SUPERTUX2_EXTRA_WARNING_FLAGS
          "${SUPERTUX2_EXTRA_WARNING_FLAGS} "
          "-Wno-implicit-fallthrough ")
      endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      string(CONCAT SUPERTUX2_EXTRA_WARNING_FLAGS
        "${SUPERTUX2_EXTRA_WARNING_FLAGS} "
        "-Weverything "

        # flags that we deliberately ignore
        "-Wno-c++98-compat "
        "-Wno-c++98-compat-pedantic "
        "-Wno-float-equal "
        "-Wno-padded "
        "-Wno-weak-vtables "
        "-Wno-disabled-macro-expansion "
        "-Wno-documentation "
        "-Wno-reserved-id-macro "
        "-Wno-sign-conversion "

        # warnings that should probably be fixed in code
        "-Wno-documentation-unknown-command "
        "-Wno-inconsistent-missing-destructor-override "
        "-Wno-deprecated-dynamic-exception-spec "
        "-Wno-deprecated "
        "-Wno-switch-enum "
        "-Wno-covered-switch-default "
        "-Wno-exit-time-destructors "
        "-Wno-global-constructors "
        "-Wno-duplicate-enum "
        "-Wno-unreachable-code "
      )

      # This snipped adapted from AOMediaCodec cmakefile
      # The detection of cross compilation by -Wpoison-system-directories has false positives on macOS because
      # --sysroot is implicitly added. Turn the warning off.
      if(NOT DEFINED HAVE_POISON_SYSTEM_DIRECTORIES_WARNING)
          check_cxx_compiler_flag(-Wpoison-system-directories HAVE_POISON_SYSTEM_DIRECTORIES_WARNING)
      endif()
      if(HAVE_POISON_SYSTEM_DIRECTORIES_WARNING)
        string(CONCAT SUPERTUX2_EXTRA_WARNING_FLAGS
          "${SUPERTUX2_EXTRA_WARNING_FLAGS} "
          "-Wno-poison-system-directories ")
      endif()
    endif()
  endif(WARNINGS)
endif()

## If xgettext is available, generate messages.pot for sources

option(GENERATE_MESSAGESPOT "Generate messages.pot files" OFF)
if(GENERATE_MESSAGESPOT)
  mark_as_advanced(
    XGETTEXT_EXECUTABLE
    )
  find_program(XGETTEXT_EXECUTABLE xgettext)
  if(NOT XGETTEXT_EXECUTABLE)
    message(STATUS "Warning: xgettext not found - will not update messages.pot")
  else(NOT XGETTEXT_EXECUTABLE)
    message(STATUS "Found xgettext: ${XGETTEXT_EXECUTABLE}")

    set(MESSAGES_POT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/data/locale/messages.pot)
    add_custom_command(
      OUTPUT ${MESSAGES_POT_FILE}
      COMMAND ${XGETTEXT_EXECUTABLE}
      ARGS --keyword=_ --language=C++ --output=${MESSAGES_POT_FILE} ${SUPERTUX_SOURCES_CXX}
      DEPENDS ${SUPERTUX_SOURCES_CXX}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Generating POT file ${MESSAGES_POT_FILE}"
      )
    list(APPEND MESSAGES_POT_FILES ${MESSAGES_POT_FILE})

    file(GLOB SUPERTUX_LEVEL_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} data/levels/*)
    foreach(SUPERTUX_LEVEL_DIR ${SUPERTUX_LEVEL_DIRS})
      file(GLOB SUPERTUX_LEVELS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SUPERTUX_LEVEL_DIR}/*.stl ${SUPERTUX_LEVEL_DIR}/*.stwm ${SUPERTUX_LEVEL_DIR}/*.txt)

      ## Do not add target if SUPERTUX_LEVELS is empty. This is needed for cmake <=2.4
      if(SUPERTUX_LEVELS)
        set(MESSAGES_POT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${SUPERTUX_LEVEL_DIR}/messages.pot)
        add_custom_command(
          OUTPUT ${MESSAGES_POT_FILE}
          COMMAND ${XGETTEXT_EXECUTABLE}
          ARGS --keyword="_:1" --language=Lisp --force-po --sort-by-file "--output=${MESSAGES_POT_FILE}" ${SUPERTUX_LEVELS}
          DEPENDS ${SUPERTUX_LEVELS}
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMENT "Generating POT file ${MESSAGES_POT_FILE}"
          )
        list(APPEND MESSAGES_POT_FILES ${MESSAGES_POT_FILE})
      endif(SUPERTUX_LEVELS)

    endforeach(SUPERTUX_LEVEL_DIR)

    add_custom_target(
      supertux2-messages ALL
      DEPENDS ${MESSAGES_POT_FILES}
      )

  endif(NOT XGETTEXT_EXECUTABLE)
endif(GENERATE_MESSAGESPOT)

## Some additional compiler switches
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DMACOSX)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


## On Windows, add an icon

if(WIN32)
  #  set(SUPERTUX_SOURCES_C ${SUPERTUX_SOURCES_C} ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.rc)
  if(MINGW)
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o
      COMMAND ${CMAKE_RC_COMPILER} -I${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons -i${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.rc -o ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o)
    set(SUPERTUX_SOURCES_C ${SUPERTUX_SOURCES_C} ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o)
  endif(MINGW)
endif(WIN32)

## Build miniswig and generate miniswig wrapper

option(GENERATE_WRAPPER "Build miniswig and generate the wrapper" OFF)
if(GENERATE_WRAPPER)
  add_subdirectory(tools/miniswig)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.hpp
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${CMAKE_CXX_COMPILER}
    ARGS -x "c++" -E -CC -DSCRIPTING_API src/scripting/wrapper.interface.hpp -o ${CMAKE_CURRENT_BINARY_DIR}/miniswig.tmp -I${CMAKE_CURRENT_SOURCE_DIR}/src
    COMMAND tools/miniswig/miniswig
    ARGS --input miniswig.tmp --output-cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.cpp --output-hpp ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.hpp --module supertux --select-namespace scripting
    DEPENDS tools/miniswig/miniswig
    IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.interface.hpp
    )
  if(USE_CLANG_TIDY)
    set_target_properties(miniswig PROPERTIES CXX_CLANG_TIDY "")
  endif()
endif(GENERATE_WRAPPER)

## Generate supertux executable in the right place

#set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

## Add target for supertux binary

add_library(supertux2_c OBJECT ${SUPERTUX_SOURCES_C})
add_library(supertux2_lib STATIC ${CMAKE_BINARY_DIR}/version.h ${SUPERTUX_SOURCES_CXX} ${SUPERTUX_RESOURCES} $<TARGET_OBJECTS:supertux2_c>)

target_link_libraries(supertux2_lib PUBLIC glm::glm)
target_compile_definitions(supertux2_lib PUBLIC -DGLM_ENABLE_EXPERIMENTAL)

if(WIN32)
  add_executable(supertux2 WIN32 src/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.rc)
  target_link_libraries(supertux2 ${SDL2MAIN_LIBRARIES})
else()
  add_executable(supertux2 src/main.cpp)
endif(WIN32)
target_link_libraries(supertux2 supertux2_lib Boost::filesystem Boost::locale)

set_target_properties(supertux2_lib PROPERTIES OUTPUT_NAME supertux2_lib)
set_target_properties(supertux2_lib PROPERTIES COMPILE_FLAGS "${SUPERTUX2_EXTRA_WARNING_FLAGS}")
if(EMSCRIPTEN)
  target_link_options(supertux2 PUBLIC -sEXPORTED_FUNCTIONS=['_main','_set_resolution','_save_config','_onDownloadProgress','_onDownloadFinished','_onDownloadError','_onDownloadAborted','_getExceptionMessage'] PUBLIC -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap'] PUBLIC -sEXTRA_EXPORTED_RUNTIME_METHODS=['ccall','cwrap'])
endif(EMSCRIPTEN)

if(WIN32 AND NOT VCPKG_BUILD)
  ## Copy dlls on windows
  add_custom_command(TARGET supertux2_lib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${DEPENDENCY_FOLDER}/dll"
    $<TARGET_FILE_DIR:supertux2_lib>)
endif(WIN32 AND NOT VCPKG_BUILD)

## Link supertux binary with squirrel and other libraries

if(VCPKG_BUILD AND NOT EMSCRIPTEN)
  target_include_directories(supertux2_lib PUBLIC ${CURL_INCLUDE_DIRS})
endif()

target_include_directories(partio_zip_lib PUBLIC external/partio_zip/)

if (NOT EMSCRIPTEN)
  target_include_directories(supertux2_lib SYSTEM PUBLIC ${SDL2_INCLUDE_DIRS})
  target_include_directories(supertux2_lib SYSTEM PUBLIC ${SDL2IMAGE_INCLUDE_DIRS})
  if(VCPKG_BUILD AND NOT WIN32)
    target_link_libraries(supertux2_lib PRIVATE SDL2::SDL2main SDL2::SDL2 SDL2::SDL2_image)
  else()
    target_link_libraries(supertux2_lib PUBLIC ${SDL2_LIBRARIES})
    target_link_libraries(supertux2_lib PUBLIC ${SDL2IMAGE_LIBRARIES})
  endif()
endif()
target_include_directories(supertux2_lib SYSTEM PUBLIC ${SDL2_TTF_PREFIX}/include/SDL2)
target_link_libraries(supertux2_lib PUBLIC SDL2_ttf)

target_link_libraries(supertux2_lib PUBLIC squirrel::squirrel)
target_link_libraries(supertux2_lib PUBLIC squirrel::sqstdlib)
target_link_libraries(supertux2_lib PUBLIC tinygettext)
target_link_libraries(supertux2_lib PUBLIC sexp)
target_link_libraries(supertux2_lib PUBLIC savepng)
target_link_libraries(supertux2_lib PUBLIC partio_zip_lib)
if(ENABLE_DISCORD)
target_link_libraries(supertux2_lib PUBLIC discord-rpc)
endif(ENABLE_DISCORD)
if(NOT EMSCRIPTEN)
  if(VCPKG_BUILD)
    target_link_libraries(supertux2_lib PUBLIC OpenAL::OpenAL)
  else()
    target_link_libraries(supertux2_lib PUBLIC ${OPENAL_LIBRARY})
  endif()
else()
  link_libraries(-lopenal)
endif()
target_link_libraries(supertux2_lib PUBLIC ${OGGVORBIS_LIBRARIES})
target_link_libraries(supertux2_lib PUBLIC ${Boost_LIBRARIES})

# Mark fmt includes as SYSTEM to avoid warnings
get_target_property(FMT_INCLUDE_DIRS fmt::fmt INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(supertux2_lib SYSTEM PUBLIC ${FMT_INCLUDE_DIRS})
target_link_libraries(supertux2_lib PUBLIC fmt::fmt)

if(USE_SYSTEM_PHYSFS)
  target_link_libraries(supertux2_lib PUBLIC ${PHYSFS_LIBRARY})
else()
  add_dependencies(supertux2_lib physfs)
  target_link_libraries(supertux2_lib PUBLIC physfs_lib)
  if(APPLE)
    target_link_libraries(supertux2_lib PUBLIC "-framework CoreFoundation")
    target_link_libraries(supertux2_lib PUBLIC "-framework Foundation")
    target_link_libraries(supertux2_lib PUBLIC "-framework IOKit")
  endif()
endif()

if(HAVE_OPENGL)
  target_link_libraries(supertux2_lib PUBLIC ${OPENGL_LIBRARY})
  if(NOT ENABLE_OPENGLES2)
    if(GLBINDING_ENABLED)
      if(VCPKG_BUILD)
        target_link_libraries(supertux2_lib PUBLIC glbinding::glbinding glbinding::glbinding-aux)
      else(VCPKG_BUILD)
        target_link_libraries(supertux2_lib PUBLIC ${GLBINDING_LIBRARIES})
      endif(VCPKG_BUILD)
    else()
      target_link_libraries(supertux2_lib PUBLIC GLEW::GLEW)
    endif()
  endif()
endif(HAVE_OPENGL)

if(NOT EMSCRIPTEN)
  if(HAVE_LIBCURL)
    if(VCPKG_BUILD)
      target_link_libraries(supertux2_lib PUBLIC ${CURL_LIBRARIES})
    else()
      target_link_libraries(supertux2_lib PUBLIC ${CURL_LIBRARY})
    endif()
  endif(HAVE_LIBCURL)
endif(NOT EMSCRIPTEN)

if(BUILD_TESTS)
  find_package(Threads REQUIRED)
  find_package(GTest REQUIRED)

  # build SuperTux tests
  file(GLOB_RECURSE TEST_SUPERTUX_SOURCES tests/*.cpp)
  add_executable(test_supertux2 ${TEST_SUPERTUX_SOURCES})
  target_compile_options(test_supertux2 PRIVATE ${WARNINGS_CXX_FLAGS})
  target_link_libraries(test_supertux2
    GTest::GTest GTest::Main
    supertux2_lib
    ${CMAKE_THREAD_LIBS_INIT})

  # add 'make test' target, use 'make test ARGS="-V"' or 'ctest -V' for verbose
  enable_testing()
  add_test(NAME test_supertux2
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND test_supertux2)

  # Prepare coverage with gcov/lcov
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      target_compile_options(supertux2_lib PUBLIC -fprofile-arcs -ftest-coverage)
      target_link_options(supertux2_lib PUBLIC -lgcov --coverage)

      add_custom_target(coverage_clear_unix
        COMMAND find . -name "*.gcda" | xargs rm || true # FIXME: Make this platform-neutral
        COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/coverage.info
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/coverage)

      add_custom_target(coverage_unix
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target coverage_clear_unix
        COMMAND ${CMAKE_CTEST_COMMAND}
        COMMAND lcov --capture --directory ${CMAKE_CURRENT_BINARY_DIR} --exclude '/usr/*' --exclude '*/tests/*' --exclude '*/external/*' --output-file ${CMAKE_CURRENT_BINARY_DIR}/coverage.info
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/coverage
        COMMAND genhtml ${CMAKE_CURRENT_BINARY_DIR}/coverage.info -o ${CMAKE_CURRENT_BINARY_DIR}/coverage -t "${SUPERTUX_VERSION_STRING}")
    else(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      message(WARNING "Coverage for non-GNU compilers has yet to be implemented")
    endif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  else(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(WARNING "Building tests in a non-Debug build will NOT make files for coverage! Change CMAKE_BUILD_TYPE to Debug to prepare for coverage analysis.")
  endif(CMAKE_BUILD_TYPE STREQUAL "Debug")
endif()

## Install stuff

option(DISABLE_CPACK_BUNDLING "Build an .app bundle without CPack" OFF)
if(WIN32 AND NOT UNIX)

  set(INSTALL_SUBDIR_BIN "bin" CACHE STRING "Installation subdir for binaries")
  set(INSTALL_SUBDIR_SHARE "data" CACHE STRING "Installation subdir for data")
  set(INSTALL_SUBDIR_DOC "doc" CACHE STRING "Installation subdir for docs")

  if(VCPKG_BUILD)
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/" DESTINATION ${INSTALL_SUBDIR_BIN} FILES_MATCHING PATTERN "*.dll")
  else()
    install(FILES ${DLLS} DESTINATION ${INSTALL_SUBDIR_BIN})
  endif()

  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.png ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.ico DESTINATION ".")

  option(PACKAGE_VCREDIST "Package the VCREDIST libraries with the program" OFF)

  if(PACKAGE_VCREDIST)
    set(CMAKE_INSTALL_UCRT_LIBRARIES true)
    include(InstallRequiredSystemLibraries)
  endif()

else(WIN32 AND NOT UNIX)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND DISABLE_CPACK_BUNDLING)

    set(INFOPLIST_CFBUNDLEEXECUTABLE "supertux2")

    set(INSTALL_SUBDIR_BIN "SuperTux.app/Contents/MacOS" CACHE STRING "Installation subdir for binaries")
    set(INSTALL_SUBDIR_SHARE "SuperTux.app/Contents/Resources/data" CACHE STRING "Installation subdir for data")
    set(INSTALL_SUBDIR_DOC "SuperTux.app/Contents/Resources" CACHE STRING "Installation subdir for docs")

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/info.plist.in ${CMAKE_BINARY_DIR}/tools/darwin/info.plist)
    install(FILES ${CMAKE_BINARY_DIR}/tools/darwin/info.plist DESTINATION "SuperTux.app/Contents/")
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/receipt DESTINATION "SuperTux.app/Contents/_MASReceipt/")

    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.png ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.icns DESTINATION "SuperTux.app/Contents/Resources/")

  else(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND DISABLE_CPACK_BUNDLING)

    set(INSTALL_SUBDIR_BIN "games" CACHE STRING "Installation subdir for binaries")
    set(INSTALL_SUBDIR_SHARE "share/games/supertux2" CACHE STRING "Installation subdir for data")
    set(INSTALL_SUBDIR_DOC "share/doc/supertux2" CACHE STRING "Installation subdir for docs")

    if(UBUNTU_TOUCH)
      set(LINUX_DESKTOP_ICON "assets/supertux-256x256.png")
      # FIXME: The "install" folder is a folder managed by Clickable and shouldn't be hardcoded here
      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/supertux2.desktop.in "install/supertux2.desktop")
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/mk/clickable/supertux2.apparmor DESTINATION ".")
      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mk/clickable/manifest.json.in ${CMAKE_CURRENT_BINARY_DIR}/install/manifest.json)
      set(APPS "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/supertux2")
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux-256x256.png DESTINATION "assets")
    else(UBUNTU_TOUCH)
      set(LINUX_DESKTOP_ICON "supertux2")
      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/supertux2.desktop.in "supertux2.desktop")
      install(FILES ${CMAKE_BINARY_DIR}/supertux2.desktop DESTINATION "share/applications")
      set(APPS "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${INSTALL_SUBDIR_BIN}/supertux2")
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.png ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.xpm DESTINATION "share/pixmaps/")
    endif(UBUNTU_TOUCH)

    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/supertux2.svg DESTINATION "share/icons/hicolor/scalable/apps")

  endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND DISABLE_CPACK_BUNDLING)
endif(WIN32 AND NOT UNIX)

if(UBUNTU_TOUCH)
  install(TARGETS supertux2 DESTINATION ".")
else(UBUNTU_TOUCH)
  install(TARGETS supertux2 DESTINATION ${INSTALL_SUBDIR_BIN})
endif(UBUNTU_TOUCH)

if(WIN32)
  get_property(SQUIRREL_LIB_PATH TARGET squirrel::squirrel PROPERTY IMPORTED_LOCATION)
  get_property(SQSTDLIB_LIB_PATH TARGET squirrel::sqstdlib PROPERTY IMPORTED_LOCATION)
  if(NOT USE_SYSTEM_PHYSFS)
    get_property(PHYSFS_LIB_PATH TARGET physfs_lib PROPERTY IMPORTED_LOCATION)
  endif()
  install(FILES ${SQUIRREL_LIB_PATH} ${SQSTDLIB_LIB_PATH} ${PHYSFS_LIB_PATH} DESTINATION ${INSTALL_SUBDIR_BIN})
endif()

if(EMSCRIPTEN)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mk/emscripten/template.html.in ${CMAKE_CURRENT_BINARY_DIR}/template.html)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mk/emscripten/supertux2.png ${CMAKE_CURRENT_BINARY_DIR}/supertux2.png COPYONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mk/emscripten/supertux2_bkg.png ${CMAKE_CURRENT_BINARY_DIR}/supertux2_bkg.png COPYONLY)
endif(EMSCRIPTEN)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/INSTALL.md ${CMAKE_CURRENT_SOURCE_DIR}/README.md ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt ${CMAKE_CURRENT_SOURCE_DIR}/NEWS.md DESTINATION ${INSTALL_SUBDIR_DOC})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/credits.stxt DESTINATION ${INSTALL_SUBDIR_SHARE})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/supertux2.appdata.xml DESTINATION "share/metainfo" )

install(DIRECTORY
  ${CMAKE_CURRENT_SOURCE_DIR}/data/images
  ${CMAKE_CURRENT_SOURCE_DIR}/data/fonts
  ${CMAKE_CURRENT_SOURCE_DIR}/data/music
  ${CMAKE_CURRENT_SOURCE_DIR}/data/particles
  ${CMAKE_CURRENT_SOURCE_DIR}/data/scripts
  ${CMAKE_CURRENT_SOURCE_DIR}/data/shader
  ${CMAKE_CURRENT_SOURCE_DIR}/data/speech
  ${CMAKE_CURRENT_SOURCE_DIR}/data/sounds
  ${CMAKE_CURRENT_SOURCE_DIR}/data/locale
  DESTINATION ${INSTALL_SUBDIR_SHARE})

if(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/levels
    DESTINATION ${INSTALL_SUBDIR_SHARE}
    PATTERN "data/levels/test" EXCLUDE
    PATTERN "data/levels/test_old" EXCLUDE
    PATTERN "data/levels/incubator" EXCLUDE
    PATTERN "data/levels/misc/menu.stl.in" EXCLUDE)
else()
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/levels
    DESTINATION ${INSTALL_SUBDIR_SHARE}
    PATTERN "data/levels/misc/menu.stl.in" EXCLUDE)
endif()

install(FILES "${CMAKE_BINARY_DIR}/data/levels/misc/menu.stl" DESTINATION "${INSTALL_SUBDIR_SHARE}/levels/misc")

## Create config.h now that INSTALL_SUBDIR_* have been set.

configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h )

## Configure main menu logo
if(("${SUPERTUX_VERSION_STRING}" MATCHES "^v[0-9]+\\.[0-9]+\\.[0-9]+$") OR IS_SUPERTUX_RELEASE OR STEAM_BUILD)
  set(LOGO_FILE "logo_final.sprite")
else()
  set(LOGO_FILE "logo.sprite")
endif()
configure_file(data/levels/misc/menu.stl.in ${CMAKE_BINARY_DIR}/data/levels/misc/menu.stl )


## CPack/Installation-specific stuff

include(InstallRequiredSystemLibraries)
set(DIRS ${Boost_LIBRARY_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/external/squirrel ${CMAKE_CURRENT_BINARY_DIR}/external/tinygettext)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND NOT DISABLE_CPACK_BUNDLING)
  set(INFOPLIST_CFBUNDLEEXECUTABLE "SuperTux")

  find_package(PNG)
  foreach (_file ${PNG_LIBRARIES})
    get_filename_component(_resolvedFile "${_file}" REALPATH)
    get_filename_component(_name "${_file}" NAME)
    install(FILES ${_resolvedFile} DESTINATION "MacOS" RENAME ${_name})
  endforeach()
  find_package(JPEG)
  foreach (_file ${JPEG_LIBRARIES})
    get_filename_component(_resolvedFile "${_file}" REALPATH)
    get_filename_component(_name "${_file}" NAME)
    install(FILES ${_resolvedFile} DESTINATION "MacOS" RENAME ${_name})
  endforeach()

  install(CODE "
       if(\"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/\" MATCHES \".*\\\\.app.*\")
       include(BundleUtilities)
       fixup_bundle(\"${APPS}\"   \"\"   \"${DIRS}\")
       endif()
       ")

  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/info.plist.in" "${CMAKE_BINARY_DIR}/tools/darwin/info.plist")

  set(CPACK_BUNDLE_NAME "SuperTux")
  set(CPACK_BUNDLE_PLIST "${CMAKE_BINARY_DIR}/tools/darwin/info.plist")
  set(CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.icns")
  set(CPACK_BUNDLE_STARTUP_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/startup.sh")
  set(CPACK_DMG_VOLUME_NAME "SuperTux ${SUPERTUX_VERSION_STRING}")
  set(CPACK_DMG_DS_STORE_SETUP_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/dsstore_setup.scpt")
  set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/background.png")
endif()


set(CPACK_PACKAGE_NAME "SuperTux")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Jump'n'Run Game featuring Tux")
set(CPACK_PACKAGE_VENDOR "SuperTux Devel Team")
set(CPACK_PACKAGE_CONTACT "SuperTux Devel Team <supertux-devel@lists.lethargik.org>")
set(CPACK_SOURCE_IGNORE_FILES "/\\\\.git/;${CMAKE_BINARY_DIR};/\\\\..*")
set(CPACK_DEBIAN_PACKAGE_NAME "supertux2")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.5), libgcc1 (>= 1:4.1), libgl1-mesa-glx | libgl1, libogg0 (>= 1.1.3), libopenal0a, libphysfs-1.0-0, libsdl-image1.2 (>= 1.2.5), libsdl1.2debian (>= 1.2.10-1), libstdc++6 (>= 4.1.2), libvorbis0a (>= 1.1.2), libvorbisfile3 (>= 1.1.2), libcurl3 (>= 7.16)")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Classic 2D jump 'n run sidescroller with Tux\n SuperTux is a classic 2D jump 'n run sidescroller game in a similar\n style like the original SuperMario games. This release of SuperTux\n features 9 enemies, 26 playable levels, software and OpenGL rendering\n modes, configurable joystick and keyboard input, new music and\n completely redone graphics.\n .\n This is a development snapshot of SuperTux. It may suffer from\n critical bugs and has not been fully tested. \n .\n Homepage: http://supertux.lethargik.org/")
set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_RPM_PACKAGE_NAME "supertux2")
set(CPACK_RPM_PACKAGE_DESCRIPTION "Classic 2D jump 'n run sidescroller with Tux\n SuperTux is a classic 2D jump 'n run sidescroller game in a similar\n style like the original SuperMario games. This release of SuperTux\n features 9 enemies, 26 playable levels, software and OpenGL rendering\n modes, configurable joystick and keyboard input, new music and\n completely redone graphics.\n .\n This is a development snapshot of SuperTux. It may suffer from\n critical bugs and has not been fully tested. \n .\n Homepage: http://supertux.lethargik.org/")
set(CPACK_RPM_PACKAGE_LICENSE "GNU General Public License (GPL)")
set(CPACK_RPM_PACKAGE_GROUP "Amusements/Games/Action/Arcade")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR ${SUPERTUX_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${SUPERTUX_VERSION_MINOR})

if(SUPERTUX_VERSION_TWEAK)
  set(CPACK_PACKAGE_VERSION_PATCH "${SUPERTUX_VERSION_PATCH}.${SUPERTUX_VERSION_TWEAK}")
else()
  set(CPACK_PACKAGE_VERSION_PATCH ${SUPERTUX_VERSION_PATCH})
endif(SUPERTUX_VERSION_TWEAK)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${SUPERTUX_VERSION_STRING}-${SUPERTUX_SYSTEM_NAME}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${SUPERTUX_VERSION_STRING}-Source")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "SuperTux")
set(CPACK_PACKAGE_EXECUTABLES "supertux2" "SuperTux")
if(WIN32 AND NOT UNIX)
  set(CPACK_NSIS_INSTALLED_ICON_NAME "supertux2")
  set(CPACK_NSIS_DISPLAY_NAME "SuperTux")
  set(CPACK_NSIS_COMPRESSOR "/SOLID lzma")
  set(CPACK_WIX_PROGRAM_MENU_FOLDER "")
  set(CPACK_WIX_UPGRADE_GUID "93E16F4E-0A68-422A-8ADC-47BE5B9433B2")
  set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.ico")

  set(CPACK_WIX_UI_BANNER "${CMAKE_CURRENT_SOURCE_DIR}/mk/wix/banner.png")
  set(CPACK_WIX_UI_DIALOG "${CMAKE_CURRENT_SOURCE_DIR}/mk/wix/dialog.png")
  set(CPACK_GENERATOR WIX)
else(WIN32 AND NOT UNIX)
endif(WIN32 AND NOT UNIX)
include(CPack)

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

mark_as_advanced(
  OGG_LIBRARY
  VORBISENC_LIBRARY
  VORBISFILE_LIBRARY
  VORBIS_INCLUDE_DIR
  VORBIS_LIBRARY
  )
