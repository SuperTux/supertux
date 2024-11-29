#
# SuperTux
# Copyright (C) 2024 MatusGuy <matusguy@supertuxproject.org>
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

macro(git_run)
  cmake_parse_arguments(GIT_RUN "" "RESULT;OUTPUT" "COMMAND" ${ARGN})

  if(NOT GIT_FOUND)
    set(${GIT_RUN_OUTPUT} ${GIT_RUN_OUTPUT}-NOTFOUND)
    # Can't use a return command here because... Macro.
  else()
    execute_process(COMMAND ${GIT_EXECUTABLE} ${GIT_RUN_COMMAND}
                    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                    RESULT_VARIABLE ${GIT_RUN_RESULT}
                    OUTPUT_VARIABLE ${GIT_RUN_OUTPUT})

    if(${${GIT_RUN_RESULT}} EQUAL 0)
      string(REPLACE "\n" "" ${GIT_RUN_OUTPUT} "${${GIT_RUN_OUTPUT}}")
    else()
      message(STATUS "\"${GIT_EXECUTABLE} ${GIT_RUN_COMMAND}\" failed with result \"${${GIT_RUN_RESULT}}\".")
      set(${GIT_RUN_OUTPUT} ${GIT_RUN_OUTPUT}-NOTFOUND)
    endif()
  endif()
endmacro()

macro(git_project_version out is_release)
  if(NOT GIT_FOUND OR NOT EXISTS "${PROJECT_SOURCE_DIR}/.git")
    set(${out} ${out}-NOTFOUND)
    set(${is_release} NO)
    # Can't use a return command here because... Macro.
  else()
    # Tag
    git_run(COMMAND describe --tags --abbrev=0 OUTPUT _tag RESULT _result)

    # Commits since tag
    git_run(COMMAND rev-list ${_tag}..HEAD --count OUTPUT _tagn RESULT _result)

    if(_tagn STREQUAL "0")
      set(${is_release} YES)
      set(${out} "${_tag}")
    else()
      # Commit hash
      git_run(COMMAND rev-parse --short HEAD OUTPUT _hash RESULT _result)

      # Branch
      git_run(COMMAND rev-parse --abbrev-ref HEAD OUTPUT _branch RESULT _result)

      set(${is_release} NO)
      set(${out} "${_tag} (${_tagn}) - ${_hash} (${_branch})")
    endif()
  endif()
endmacro()
