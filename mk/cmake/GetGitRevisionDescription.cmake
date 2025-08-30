#
# SuperTux
# Copyright (C) 2024-2025 MatusGuy
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

find_package(Git QUIET)

# Thanks CMake! This macro sets the same value for the current scope
# and the parent scope, because apparently only setting it for
# the parent scope doesn't work.
macro(parent_set var val)
  set(${var} ${val})
  set(${var} ${val} PARENT_SCOPE)
endmacro()

macro(git_run)
  cmake_parse_arguments(GIT_RUN "" "RESULT;OUTPUT" "COMMAND" ${ARGN})

  if(NOT GIT_FOUND)
    parent_set(${GIT_RUN_OUTPUT} ${GIT_RUN_OUTPUT}-NOTFOUND)
    parent_set(${GIT_RUN_RESULT} 1)
    # Can't use a return command here because... Macro.
  else()
    execute_process(COMMAND ${GIT_EXECUTABLE} ${GIT_RUN_COMMAND}
                    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                    RESULT_VARIABLE _result
                    OUTPUT_VARIABLE _output)

    if(_result EQUAL 0)
      string(REPLACE "\n" "" _output "${_output}")
    else()
      message(STATUS "\"${GIT_EXECUTABLE} ${GIT_RUN_COMMAND}\" failed with result \"${_result}\".")
      set(_output ${GIT_RUN_OUTPUT}-NOTFOUND)
    endif()

    parent_set(${GIT_RUN_OUTPUT} ${_output})
    parent_set(${GIT_RUN_RESULT} ${_result})
  endif()
endmacro()

function(git_project_version is_release)
  if(NOT GIT_FOUND OR NOT EXISTS "${PROJECT_SOURCE_DIR}/.git")
    parent_set(${out} ${out}-NOTFOUND)
    return()
  endif()

  # Tag
  git_run(COMMAND describe --tags --abbrev=0 OUTPUT GIT_TAG RESULT GIT_TAG_RESULT)

  if(is_release)
    return()
  endif()

  # Commit hash
  git_run(COMMAND rev-parse --short HEAD OUTPUT GIT_HASH RESULT GIT_HASH_RESULT)

  # Branch
  git_run(COMMAND rev-parse --abbrev-ref HEAD OUTPUT GIT_BRANCH RESULT GIT_BRANCH_RESULT)
endfunction()
