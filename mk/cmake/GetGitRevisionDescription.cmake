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

function(git_get_hash hash branch)
  if(GIT_FOUND)
    return()
  endif()

  # Commit hash
  execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                  OUTPUT_VARIABLE ${hash})

  # Branch
  execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                  OUTPUT_VARIABLE ${branch})
endfunction()
