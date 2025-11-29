# Install script for directory: /home/runner/work/supertux/supertux/external/tinygettext

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/runner/work/supertux/supertux/_codeql_build_dir/external/tinygettext/libtinygettext.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/tinygettext" TYPE FILE FILES
    "/home/runner/work/supertux/supertux/external/tinygettext/include/tinygettext/dictionary.hpp"
    "/home/runner/work/supertux/supertux/external/tinygettext/include/tinygettext/dictionary_manager.hpp"
    "/home/runner/work/supertux/supertux/external/tinygettext/include/tinygettext/file_system.hpp"
    "/home/runner/work/supertux/supertux/external/tinygettext/include/tinygettext/iconv.hpp"
    "/home/runner/work/supertux/supertux/external/tinygettext/include/tinygettext/language.hpp"
    "/home/runner/work/supertux/supertux/external/tinygettext/include/tinygettext/log.hpp"
    "/home/runner/work/supertux/supertux/external/tinygettext/include/tinygettext/log_stream.hpp"
    "/home/runner/work/supertux/supertux/external/tinygettext/include/tinygettext/plural_forms.hpp"
    "/home/runner/work/supertux/supertux/external/tinygettext/include/tinygettext/po_parser.hpp"
    "/home/runner/work/supertux/supertux/external/tinygettext/include/tinygettext/tinygettext.hpp"
    "/home/runner/work/supertux/supertux/external/tinygettext/include/tinygettext/unix_file_system.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/runner/work/supertux/supertux/_codeql_build_dir/external/tinygettext/tinygettext.pc")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/runner/work/supertux/supertux/_codeql_build_dir/external/tinygettext/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
