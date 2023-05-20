# tinygettext - A gettext replacement that works directly on .po files
# Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>,
#               2020 Ingo Ruhnke <grumbel@gmail.com>
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

cmake_minimum_required(VERSION 3.0)

include(mk/cmake/TinyCMMC.cmake)

project(tinygettext VERSION "0.2.0")

file(GLOB TINYGETTEXT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
file(GLOB TINYGETTEXT_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} include/tinygettext/*.hpp)


add_library(tinygettext ${TINYGETTEXT_SOURCES})
set_target_properties(tinygettext PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  PUBLIC_HEADER "${TINYGETTEXT_HEADERS}")
target_include_directories(tinygettext PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_options(tinygettext PRIVATE ${TINYCMMC_WARNINGS_CXX_FLAGS})
target_compile_definitions(tinygettext PRIVATE VERSION=${TINYGETTEXT_VERSION})

if(TINYGETTEXT_WITH_SDL)
  if(ANDROID)
    find_library(SDL2 SDL2)
    target_compile_definitions(tinygettext PUBLIC TINYGETTEXT_WITH_SDL)
    target_link_libraries(tinygettext PUBLIC SDL2)
  else()
    # lol no windows
    find_package(PkgConfig REQUIRED)
    pkg_search_module(SDL2 REQUIRED sdl2 IMPORTED_TARGET)
    target_compile_definitions(tinygettext PUBLIC TINYGETTEXT_WITH_SDL)
    target_link_libraries(tinygettext PUBLIC PkgConfig::SDL2)
  endif()
else()
  find_package(Iconv REQUIRED)
  target_link_libraries(tinygettext PUBLIC Iconv::Iconv)
endif()

if(WIN32)
  target_compile_definitions(tinygettext PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

configure_file(tinygettext.pc.in tinygettext.pc @ONLY)

if(ANDROID)
  return()
endif()

install(TARGETS tinygettext
  EXPORT tinygettext-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
install(FILES ${tinygettext_BINARY_DIR}/tinygettext.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# CMake integration
tinycmmc_export_and_install_library(tinygettext)

if(BUILD_TESTS)
  foreach(TEST tinygettext_test po_parser_test)
    add_executable(${TEST} test/${TEST}.cpp)
    set_target_properties(${TEST} PROPERTIES
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED ON
      CXX_EXTENSIONS OFF)
    target_compile_options(${TEST} PRIVATE ${TINYCMMC_WARNINGS_CXX_FLAGS})
    if(WIN32)
      target_compile_definitions(${TEST} PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
    target_link_libraries(${TEST} PRIVATE tinygettext)
  endforeach(TEST)
endif()

# EOF #
