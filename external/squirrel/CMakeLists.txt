#
# SuperTux - squirrel library build script
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

## Add include/ to include directories

INCLUDE_DIRECTORIES(${SUPERTUX_SOURCE_DIR}/src/squirrel/include/)

## build list of source files

FILE(GLOB SQUIRREL_SOURCES squirrel/*.cpp sqstdlib/*.cpp sqstdlib/*.c)

## add additional compiler switches

ADD_DEFINITIONS(-include ${CMAKE_BINARY_DIR}/config.h)
# the squirrel sources are out of our control so don't be too pedantic about
# them
REMOVE_DEFINITIONS(-Werror -W)

## define a target for building the library

ADD_LIBRARY(squirrel ${SQUIRREL_SOURCES})
