# AddPackage.cmake - Portable find_package approach
#
# Copyright (C) 2024 Hyland B. <me@ow.swag.toys>
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

# This monstrosity essentially adds a complex package wrapping
# function, but it's honestly impressive. It essentially tries to
# default to find_package, however, this may not always work,
# especially on *NIX machines or anything weird like that. So, being
# clever, it then tries to go to PkgConfig, which (generally speaking)
# tends to produce more accurate results on *NIX
# machines. Unfortunately, this may not always work (and sometimes by
# design), so then it finally falls back to Supertux's old, weird
# Provide files. And if that fails, then that means the package
# doesn't exist or wasn't installed.

# Usage:
#
#   add_package(TARGET SDL2  <-- The output target, a "target alias" (sometimes)
#      PKG SDL2              <-- The find_package package, in this case it gets SDL2
#                                 (might look similar to the output target)
#      PKG_USE SDL2::SDL2    <-- Specific "namespace" from the package we want to alias
#      CONFIG                <-- (optional) Passed to find_package if its a CONFIG
#      REQUIRED              <-- (optional) NOT passed to find_package, just a check to
#                                  throw an error if we've exhausted all options.
#      PKG_CONFIG sdl2 sdl2_ttf   <-- (optional, recommended) List of packages for PkgConfig
#      PROVIDES ProvideSDL2       <-- (optional) Fallback to just look at the provided file. 
#   )                                  Undecided if I should fall back to a FindXXXX.cmake yet
	
find_package(PkgConfig)
macro(add_package)
  cmake_parse_arguments(addpackage_args
	  "CONFIG;REQUIRED" "TARGET;PROVIDES;PKG;PKG_USE" "PKG_CONFIG"
	  ${ARGN}
	)

  # Note: We don't pass "REQUIRED" here because we choose to fallback if it doesn't exist.
  #       Later, however, we do choose to throw an error based on this flag.
  set(addpackage_fp_args "")
  if (${addpackage_args_CONFIG})
	string(APPEND addpackage_fp_args "CONFIG")
  endif()
  
  find_package(${addpackage_args_PKG} ${addpackage_fp_args})
  
  if(${addpackage_args_PKG}_FOUND)
  	# See if its an alias (Is this needed?)
	get_target_property(addpackage_pkg_alias_check ${addpackage_args_PKG_USE} ALIASED_TARGET)
	if (addpackage_pkg_alias_check STREQUAL "addpackage_pkg_alias_check-NOTFOUND")
		add_library(${addpackage_args_TARGET} ALIAS ${addpackage_args_PKG_USE})
	else()
		message(STATUS "Package \"${addpackage_args_PKG}\" is an alias. Realiasing it.")
		# "unalias" it, aka just export the "alias" as the new target, so a re-alias, really...
		get_target_property(${addpackage_args_TARGET} ${addpackage_args_PKG_USE} ALIASED_TARGET)
	endif()
  else()
	#message(STATUS "CMake Package \"${addpackage_args_PKG}\" doesn't exist, so falling back to PkgConfig")
	
	if (PkgConfig_FOUND)
	  if (${addpackage_args_REQUIRED})
		list(APPEND addpackage_args_pkg_config_args REQUIRED)
	  endif()
	  pkg_check_modules(${addpackage_args_TARGET} ${addpackage_args_pkg_config_args} ${addpackage_args_PKG_CONFIG})
	elseif(addpackage_args_REQUIRED)
	  # TODO look in Provides
	  message(FATAL_ERROR "Package \"${addpackage_args_TARGET}\" couldn't be found, but it's required.\nI don't know what to do. Is it installed?")
	endif()
  endif()
endmacro(add_package)
