#
# SuperTux
# Copyright (C) 2025 MatusGuy
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

include(GetGitRevisionDescription)
set(_is_release_default YES)
if(GIT_ENABLED)
  git_is_tag(_is_release_default)
  git_project_version()
endif()

option(IS_SUPERTUX_RELEASE "Build as official SuperTux release" ${_is_release_default})
if(IS_SUPERTUX_RELEASE)
  message(STATUS "Building SuperTux in Release mode!")
endif()

if(GIT_ENABLED)
  configure_file("${CMAKE_CURRENT_LIST_DIR}/version-git.cmake.in"
                 "${PROJECT_SOURCE_DIR}/version.cmake" @ONLY)
elseif(NOT EXISTS "${PROJECT_SOURCE_DIR}/version.cmake")
  # If version.cmake exists, then it likely has some useful information
  # that PROJECT_VERSION doesn't.
  configure_file("${CMAKE_CURRENT_LIST_DIR}/version.cmake.in"
                 "${PROJECT_SOURCE_DIR}/version.cmake" @ONLY)
endif()
include("${PROJECT_SOURCE_DIR}/version.cmake")

message(STATUS "Git version checking results:")
if(GIT_ENABLED)
  message(STATUS "  GIT_TAG: ${GIT_TAG}")
  message(STATUS "  GIT_HASH: ${GIT_HASH}")
  message(STATUS "  GIT_BRANCH: ${GIT_BRANCH}")
endif()
message(STATUS "  SUPERTUX_PACKAGE_VERSION: ${SUPERTUX_PACKAGE_VERSION}")
message(STATUS "  SUPERTUX_PACKAGE_VERSION_TAG: ${SUPERTUX_PACKAGE_VERSION_TAG}")
message(STATUS "  SUPERTUX_VERSION_STRING: ${SUPERTUX_VERSION_STRING}")

configure_file(version.h.in ${CMAKE_BINARY_DIR}/version.h)

set_source_files_properties(${CMAKE_BINARY_DIR}/version.h
  PROPERTIES GENERATED true)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/supertux/main.cpp
  PROPERTIES OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/version.h")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/supertux/title_screen.cpp
  PROPERTIES OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/version.h")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/addon/addon_manager.cpp
  PROPERTIES OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/version.h")

# EOF #
