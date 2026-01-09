# AddPackage.cmake - Portable find_package approach
#
# Copyright (C) 2024 Hyland B. <me@ow.swag.toys>
# Copyright (C) 2024-2025 MatusGuy <martusguy@proton.me>
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
# tends to produce more accurate results on *NIX machines.

# Usage:
#
#   add_package(TARGET SDL2       <-- The output target, a "target alias" (sometimes).
#      PKG SDL2                   <-- The find_package package, in this case it gets SDL2
#                                     (might look similar to the output target).
#      PKG_USE SDL2::SDL2         <-- Specific target from the package we want to alias.
#      CONFIG                     <-- (optional) Passed to find_package if its a CONFIG.
#      REQUIRED                   <-- (optional) NOT passed to find_package, just a check to
#                                     throw an error if we've exhausted all options.
#      PKG_CONFIG sdl2 sdl2_ttf   <-- (optional, recommended) List of packages for pkg-config.
#      PREFER_PKGCONFIG           <-- (optional) If the host machine is running a unix-like,
#   )                                 skip the find_package call and use pkg-config.

# NOTE: The PREFER_PKGCONFIG option gets ignored if the ${PKG}_PREFER_FIND_PACKAGE variable is true,
# with PKG being the name of the package passed to the PKG argument in the add_package command.

find_package(PkgConfig)
function(add_package)
  cmake_parse_arguments(addpackage_args
    "CONFIG;REQUIRED;PREFER_PKGCONFIG" "TARGET;PROVIDES;PKG;PKG_USE" "PKG_CONFIG"
    ${ARGN}
  )

  if(TARGET ${addpackage_args_TARGET})
    message(STATUS "Target \"${addpackage_args_TARGET}\" already exists. Skipping")
    set("ADDPKG_${addpackage_args_PKG}_FOUND" TRUE PARENT_SCOPE)
    return()
  endif()

  if((NOT UNIX AND (NOT MINGW)) OR ${addpackage_args_PKG}_PREFER_FIND_PACKAGE)
    set(addpackage_args_PREFER_PKGCONFIG NO)
  endif()

  if(NOT addpackage_args_PREFER_PKGCONFIG)
    message(STATUS "Looking for ${addpackage_args_PKG} using find_package.")

    # NOTE: We don't pass "REQUIRED" here because we choose to fallback if it doesn't exist.
    #       Later, however, we do choose to throw an error based on this flag.
    set(addpackage_fp_args "")
    if(addpackage_args_CONFIG)
      string(APPEND addpackage_fp_args "CONFIG")
    endif()

    find_package(${addpackage_args_PKG} ${addpackage_fp_args} QUIET)
  endif()

  if(TARGET ${addpackage_args_TARGET})
    message(STATUS "Package \"${addpackage_args_PKG}\" was found successfully!")
    set("ADDPKG_${addpackage_args_PKG}_FOUND" TRUE PARENT_SCOPE)
    return()
  endif()

  # PKG_FOUND is not as reliable as I thought it would be.
  # It sometimes marks true without adding the proposed target and other times
  # marks false even when adding the propsed target!
  if((TARGET ${addpackage_args_PKG_USE}) AND NOT addpackage_args_PREFER_PKGCONFIG)
    get_target_property(addpackage_pkg_alias_check ${addpackage_args_PKG_USE} ALIASED_TARGET)
    if(addpackage_pkg_alias_check STREQUAL "addpackage_pkg_alias_check-NOTFOUND")
      add_library(${addpackage_args_TARGET} ALIAS ${addpackage_args_PKG_USE})
    else()
      message(STATUS "Package \"${addpackage_args_PKG_USE}\" is an alias. Realiasing it.")
      # "unalias" it, aka just export the "alias" as the new target, so a re-alias, really...
      get_target_property(_real_target ${addpackage_args_PKG_USE} ALIASED_TARGET)
      add_library(${addpackage_args_TARGET} ALIAS ${_real_target})
    endif()

    message(STATUS "Package \"${addpackage_args_PKG}\" was found successfully!")
    set("ADDPKG_${addpackage_args_PKG}_FOUND" TRUE PARENT_SCOPE)
  else()
    message(STATUS "CMake Package \"${addpackage_args_PKG}\" doesn't exist, so falling back to PkgConfig")

    if(PkgConfig_FOUND)
      set(addpackage_args_pkg_config_args)
      if(addpackage_args_REQUIRED)
        list(APPEND addpackage_args_pkg_config_args REQUIRED)
      endif()

      list(APPEND addpackage_args_pkg_config_args IMPORTED_TARGET GLOBAL)

      pkg_search_module(${addpackage_args_TARGET} ${addpackage_args_pkg_config_args} ${addpackage_args_PKG_CONFIG})

      if(NOT ${addpackage_args_TARGET}_FOUND)
        if(addpackage_args_REQUIRED)
          message(FATAL_ERROR "Package \"${addpackage_args_TARGET}\" couldn't be found with pkg-config, but it's required.\n"
                              "I don't know what to do. Is it installed?\n"
                              "Tried: ${addpackage_args_PKG_CONFIG}")
          set("ADDPKG_${addpackage_args_PKG}_FOUND" FALSE PARENT_SCOPE)
        else()
          message(STATUS "Package \"${addpackage_args_TARGET}\" couldn't be found with pkg-config.\n"
                         "Tried: ${addpackage_args_PKG_CONFIG}")
          set("ADDPKG_${addpackage_args_PKG}_FOUND" FALSE PARENT_SCOPE)
          return()
        endif()
      endif()

      add_library(${addpackage_args_TARGET} ALIAS PkgConfig::${addpackage_args_TARGET})
      message(STATUS "Package \"${addpackage_args_PKG}\" was found successfully using pkg-config!")
      set("ADDPKG_${addpackage_args_PKG}_FOUND" TRUE PARENT_SCOPE)
    elseif(addpackage_args_REQUIRED)
      message(FATAL_ERROR "Package \"${addpackage_args_TARGET}\" couldn't be found, but it's required.\nI don't know what to do. Is it installed?")
      set("ADDPKG_${addpackage_args_PKG}_FOUND" FALSE PARENT_SCOPE)
    endif()
  endif()
endfunction()
