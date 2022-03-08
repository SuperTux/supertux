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
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/mk/cmake)
include(ConfigureFiles)
include(ExternalProject)
include(CheckCXXCompilerFlag)
include(CheckSymbolExists)

## For autopackage
set(APPDATADIR "${CMAKE_INSTALL_PREFIX}/share/games/supertux2")

set(BUILD_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")
set(BUILD_CONFIG_DATA_DIR "${CMAKE_BINARY_DIR}/data")
## Check endianess

if(NOT EMSCRIPTEN)
  # FIXME: Any reason why we need this?
  include(TestBigEndian)
  test_big_endian(WORDS_BIGENDIAN)
endif()

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
option(IS_SUPERTUX_RELEASE "Build as official SuperTux release" OFF)
option(BUILD_TESTS "Build test cases" OFF)

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

if(WIN32)
  include(SuperTux/Win32)
endif()

## Add lots of dependencies to compiler switches
include(SuperTux/ProvideBoost)
include(SuperTux/ProvideGlm)
include(SuperTux/ProvideFmt)
include(SuperTux/ProvideSDL2)
include(SuperTux/ProvideOpenAL)
include(SuperTux/ProvideOggVorbis)
include(SuperTux/ProvidePhysfs)
include(SuperTux/ProvideCurl)

include(SuperTux/BuildVersion)
include(SuperTux/BuildDocumentation)

## Check platform-dependent build options
include(ConfigureChecks)

# Include altivec wrapper on ppc
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^ppc.*")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/ppc)
endif()

include(SuperTux/ProvideSquirrel)
include(SuperTux/ProvideTinygettext)
include(SuperTux/ProvideSDL2_ttf)

## Some additional include paths

include_directories(${CMAKE_BINARY_DIR}/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/external/findlocale/)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/external/obstack/)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/external/sexp-cpp/include/)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL_SavePNG/)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/external/partio_zip/)

include(SuperTux/ProvideDiscord)

if(WIN32)
  add_definitions(-D_USE_MATH_DEFINES -DNOMINMAX)
  add_definitions(-DWIN32)
endif()

## Lint
include(SuperTux/ClangTidy)

## Build list of sources for supertux binary

file(GLOB SUPERTUX_SOURCES_C RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} external/obstack/*.c external/findlocale/findlocale.c)

file(GLOB SUPERTUX_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*/*.cpp src/supertux/menu/*.cpp src/video/sdl/*.cpp src/video/null/*.cpp)
file(GLOB SUPERTUX_RESOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${PROJECT_BINARY_DIR}/tmp/*.rc")

include(SuperTux/ProvideSexpcpp)
include(SuperTux/ProvideSavePNG)
include(SuperTux/ProvidePartioZip)

if(HAVE_OPENGL)
  file(GLOB SUPERTUX_OPENGL_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/video/gl/*.cpp)
  set(SUPERTUX_SOURCES_CXX ${SUPERTUX_SOURCES_CXX} ${SUPERTUX_OPENGL_SOURCES})
endif()

## Sort source lists to have deterministic linking order
list(SORT SUPERTUX_SOURCES_C)
list(SORT SUPERTUX_SOURCES_CXX)
list(SORT SUPERTUX_RESOURCES)

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.cpp)
  set(SUPERTUX_SOURCES_CXX ${SUPERTUX_SOURCES_CXX} ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.cpp)
endif()

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
else()
  if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp)
    file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp)
  endif()
endif()

## Debug options
include(SuperTux/WarningFlags)

## If xgettext is available, generate messages.pot for sources
include(SuperTux/BuildMessagePot)

## Some additional compiler switches
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DMACOSX)
endif()

## On Windows, add an icon

if(WIN32)
  #  set(SUPERTUX_SOURCES_C ${SUPERTUX_SOURCES_C} ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.rc)
  if(MINGW)
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o
      COMMAND ${CMAKE_RC_COMPILER} -I${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons -i${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.rc -o ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o)
    set(SUPERTUX_SOURCES_C ${SUPERTUX_SOURCES_C} ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o)
  endif()
endif()

## Build miniswig and generate miniswig wrapper
include(SuperTux/BuildMiniswigWrapper)

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
endif()
target_link_libraries(supertux2 supertux2_lib Boost::filesystem Boost::locale)

set_target_properties(supertux2_lib PROPERTIES OUTPUT_NAME supertux2_lib)
set_target_properties(supertux2_lib PROPERTIES COMPILE_FLAGS "${SUPERTUX2_EXTRA_WARNING_FLAGS}")
if(EMSCRIPTEN)
  target_link_options(supertux2 PUBLIC -sEXPORTED_FUNCTIONS=['_main','_set_resolution','_save_config','_onDownloadProgress','_onDownloadFinished','_onDownloadError','_onDownloadAborted','_getExceptionMessage'] PUBLIC -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap'] PUBLIC -sEXTRA_EXPORTED_RUNTIME_METHODS=['ccall','cwrap'])
endif()

if(WIN32 AND NOT VCPKG_BUILD)
  ## Copy dlls on windows
  add_custom_command(TARGET supertux2_lib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${DEPENDENCY_FOLDER}/dll"
    $<TARGET_FILE_DIR:supertux2_lib>)
endif()

## Link supertux binary with squirrel and other libraries
target_include_directories(partio_zip_lib PUBLIC external/partio_zip/)

if(NOT EMSCRIPTEN)
  target_include_directories(supertux2_lib SYSTEM PUBLIC ${SDL2_INCLUDE_DIRS})
  target_include_directories(supertux2_lib SYSTEM PUBLIC ${SDL2IMAGE_INCLUDE_DIRS})
  if(VCPKG_BUILD AND NOT WIN32)
    target_link_libraries(supertux2_lib PRIVATE SDL2::SDL2main SDL2::SDL2 SDL2::SDL2_image)
  else()
    target_link_libraries(supertux2_lib PUBLIC ${SDL2_LIBRARIES})
    target_link_libraries(supertux2_lib PUBLIC ${SDL2IMAGE_LIBRARIES})
  endif()
endif()

target_link_libraries(supertux2_lib PUBLIC SDL2_ttf)
target_link_libraries(supertux2_lib PUBLIC squirrel::squirrel)
target_link_libraries(supertux2_lib PUBLIC squirrel::sqstdlib)
target_link_libraries(supertux2_lib PUBLIC tinygettext)
target_link_libraries(supertux2_lib PUBLIC sexp)
target_link_libraries(supertux2_lib PUBLIC savepng)
target_link_libraries(supertux2_lib PUBLIC partio_zip_lib)
target_link_libraries(supertux2_lib PUBLIC OpenAL)
target_link_libraries(supertux2_lib PUBLIC ${OGGVORBIS_LIBRARIES})
target_link_libraries(supertux2_lib PUBLIC ${Boost_LIBRARIES})
target_link_libraries(supertux2_lib PUBLIC fmt::fmt)
target_link_libraries(supertux2_lib PUBLIC physfs)

if(ENABLE_DISCORD)
  target_link_libraries(supertux2_lib PUBLIC discord-rpc)
endif()

if(HAVE_OPENGL)
  target_link_libraries(supertux2_lib PUBLIC ${OPENGL_LIBRARY})
  if(NOT ENABLE_OPENGLES2)
    if(GLBINDING_ENABLED)
      if(VCPKG_BUILD)
        target_link_libraries(supertux2_lib PUBLIC glbinding::glbinding glbinding::glbinding-aux)
      else()
        target_link_libraries(supertux2_lib PUBLIC ${GLBINDING_LIBRARIES})
      endif()
    else()
      target_link_libraries(supertux2_lib PUBLIC GLEW::GLEW)
    endif()
  endif()
endif()

if(NOT EMSCRIPTEN)
  target_link_libraries(supertux2_lib PUBLIC CURL::libcurl)
endif()

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
    else()
      message(WARNING "Coverage for non-GNU compilers has yet to be implemented")
    endif()
  else()
    message(WARNING "Building tests in a non-Debug build will NOT make files for coverage! Change CMAKE_BUILD_TYPE to Debug to prepare for coverage analysis.")
  endif()
endif()

## Install stuff
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

else()
  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND DISABLE_CPACK_BUNDLING)

    set(INFOPLIST_CFBUNDLEEXECUTABLE "supertux2")

    set(INSTALL_SUBDIR_BIN "SuperTux.app/Contents/MacOS" CACHE STRING "Installation subdir for binaries")
    set(INSTALL_SUBDIR_SHARE "SuperTux.app/Contents/Resources/data" CACHE STRING "Installation subdir for data")
    set(INSTALL_SUBDIR_DOC "SuperTux.app/Contents/Resources" CACHE STRING "Installation subdir for docs")

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/info.plist.in ${CMAKE_BINARY_DIR}/tools/darwin/info.plist)
    install(FILES ${CMAKE_BINARY_DIR}/tools/darwin/info.plist DESTINATION "SuperTux.app/Contents/")
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/receipt DESTINATION "SuperTux.app/Contents/_MASReceipt/")

    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.png ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.icns DESTINATION "SuperTux.app/Contents/Resources/")

  else()

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
    else()
      set(LINUX_DESKTOP_ICON "supertux2")
      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/supertux2.desktop.in "supertux2.desktop")
      install(FILES ${CMAKE_BINARY_DIR}/supertux2.desktop DESTINATION "share/applications")
      set(APPS "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${INSTALL_SUBDIR_BIN}/supertux2")
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.png ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.xpm DESTINATION "share/pixmaps/")
    endif()

    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/supertux2.svg DESTINATION "share/icons/hicolor/scalable/apps")

  endif()
endif()

if(UBUNTU_TOUCH)
  install(TARGETS supertux2 DESTINATION ".")
else()
  install(TARGETS supertux2 DESTINATION ${INSTALL_SUBDIR_BIN})
endif()

if(WIN32)
  get_property(SQUIRREL_LIB_PATH TARGET squirrel::squirrel PROPERTY IMPORTED_LOCATION)
  get_property(SQSTDLIB_LIB_PATH TARGET squirrel::sqstdlib PROPERTY IMPORTED_LOCATION)
  install(FILES ${SQUIRREL_LIB_PATH} ${SQSTDLIB_LIB_PATH} DESTINATION ${INSTALL_SUBDIR_BIN})

  get_property(PHYSFS_LIB_PATH TARGET physfs PROPERTY IMPORTED_LOCATION)
  install(FILES ${PHYSFS_LIB_PATH} DESTINATION ${INSTALL_SUBDIR_BIN})
endif()

if(EMSCRIPTEN)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mk/emscripten/template.html.in ${CMAKE_CURRENT_BINARY_DIR}/template.html)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mk/emscripten/supertux2.png ${CMAKE_CURRENT_BINARY_DIR}/supertux2.png COPYONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mk/emscripten/supertux2_bkg.png ${CMAKE_CURRENT_BINARY_DIR}/supertux2_bkg.png COPYONLY)
endif()

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
