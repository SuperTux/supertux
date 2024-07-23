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

function(git_run cmd output result)
  if(NOT GIT_FOUND)
    set(${output} ${output}-NOTFOUND)
    return()
  endif()

  execute_process(COMMAND "${GIT_EXECUTABLE} ${cmd}"
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                  RESULT_VARIABLE ${result}
                  OUTPUT_VARIABLE ${output})

  if(${result} EQUAL 0)
    string(REPLACE "\n" "" ${output} ${${output}})
  else()
    set(${output} ${output}-NOTFOUND)
  endif()
endfunction()

function(git_project_version out is_release)
  if(NOT GIT_FOUND OR NOT EXISTS "${PROJECT_SOURCE_DIR}/.git")
    set(${out} ${out}-NOTFOUND)
    set(${is_release} NO)
    return()
  endif()

  # Tag
  git_run("describe --tags --abbrev=0" _tag _result)

  # Commits since tag
  git_run("rev-list ${_tag}..HEAD --count" _tagn _result)

  if(_tagn STREQUAL "0")
    set(${is_release} YES)
    set(${out} "${_tag}")
  else()
    # Commit hash
    git_run("rev-parse --short HEAD" _hash _result)

    # Branch
    git_run("rev-parse --abbrev-ref HEAD" _branch _result)

    set(${is_release} NO)
    set(${out} "${_tag} (${_tagn}) - ${_hash} (${_branch})")
  endif()
endfunction()
