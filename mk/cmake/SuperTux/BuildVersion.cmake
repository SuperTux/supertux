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

set(SUPERTUX_PACKAGE_VERSION "" CACHE STRING "Display version name. Will show in the window title of the program, for example.")
set(SUPERTUX_VERSION_STRING "" CACHE STRING "Brief version string. Will show in the name of the archive or the cURL user agent, for example.")

if(SUPERTUX_PACKAGE_VERSION AND SUPERTUX_VERSION_STRING)
  set(SUPERTUX_CUSTOM_VERSION YES)
else()
  include(GetGitRevisionDescription)
  git_project_version(${IS_SUPERTUX_RELEASE})
endif()

if(NOT SUPERTUX_PACKAGE_VERSION)
  if(IS_SUPERTUX_RELEASE)
    set(SUPERTUX_PACKAGE_VERSION "${GIT_TAG}")
  else()
    set(SUPERTUX_PACKAGE_VERSION "dev ")

    if(NOT GIT_TAG_RESULT EQUAL 0)
      string(APPEND SUPERTUX_PACKAGE_VERSION "${GIT_HASH} (${GIT_BRANCH})")
    else()
      string(APPEND SUPERTUX_PACKAGE_VERSION "${GIT_TAG} - ${GIT_HASH} (${GIT_BRANCH})")
    endif()
  endif()
endif()

if(NOT SUPERTUX_VERSION_STRING)
  if(IS_SUPERTUX_RELEASE)
    set(SUPERTUX_VERSION_STRING "${GIT_TAG}")
  else()
    set(SUPERTUX_VERSION_STRING "${GIT_BRANCH}-${GIT_HASH}")
  endif()
endif()

message(STATUS "Git version checking results:")
if(NOT SUPERTUX_CUSTOM_VERSION)
  message(STATUS "  GIT_TAG: ${GIT_TAG}")
  message(STATUS "  GIT_HASH: ${GIT_HASH}")
  message(STATUS "  GIT_BRANCH: ${GIT_BRANCH}")
endif()
message(STATUS "  SUPERTUX_PACKAGE_VERSION: ${SUPERTUX_PACKAGE_VERSION}")
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
