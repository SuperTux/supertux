if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

if(LINUX)
  option(USE_PKGCONFIG "Also try to look for dependencies using pkg-config." ON)
  if(USE_PKGCONFIG)
    find_package(PkgConfig)
  endif()
endif()

# Link various targets that come from a subfolder in
# ${PROJECT_SOURCE_DIR}/external/. Example:
#
#   target_external_dependencies_from_folder(target squirrel squirrel sqstdlib)
#
macro(target_external_dependencies_from_folder tar folder)
  set(deps ${ARGN})

  set(subdir ${PROJECT_SOURCE_DIR}/external/${folder})
  add_subdirectory(${subdir})

  foreach(dep ${deps})
    if(NOT TARGET ${dep})
      message(WARNING "Could NOT find ${dep} in ${subdir}. Skipping.")
      continue()
    endif()
  
    get_target_property(DEP_INCLUDES ${dep} INCLUDE_DIRECTORIES)
    target_include_directories(${tar} PUBLIC ${DEP_INCLUDES})
    target_link_libraries(${tar} PUBLIC ${dep})
    add_dependencies(${tar} ${dep})
  endforeach()
endmacro()

# Add a subfolder from ${PROJECT_SOURCE_DIR}/external/
# and link a target with the same name. Example:
#
#   target_external_dependencies(target SDL2 SDL2_image ...)
#
# You can also use a static library if available:
#
#   target_external_dependencies(target STATIC SDL2 SDL2_image ...)
#
macro(target_external_dependencies tar)
  cmake_parse_arguments(DEPS_ARGS "STATIC" "" "" ${ARGN})
  set(deps ${DEPS_ARGS_UNPARSED_ARGUMENTS})

  foreach(dep ${deps})
    set(deptar "${dep}")

    if(DEPS_ARGS_STATIC)
      set(deptar ${deptar}-static)
    endif()

    set(oldbuildtype ${CMAKE_BUILD_TYPE})
    set(CMAKE_BUILD_TYPE Release)

    set(subdir ${PROJECT_SOURCE_DIR}/external/${dep})
    add_subdirectory(${subdir})

    set(CMAKE_BUILD_TYPE ${oldbuildtype})

    if(DEPS_ARGS_STATIC AND NOT TARGET ${deptar})
      message(VERBOSE "Could not find ${deptar} in ${subdir}. Falling back to non-static version.")
      set(deptar ${dep})
    endif()

    if(NOT TARGET ${deptar})
      message(WARNING "Could NOT find ${deptar} in ${subdir}. Skipping.")
      continue()
    endif()

    get_target_property(DEP_INCLUDES ${dep} INCLUDE_DIRECTORIES)
    if("${DEP_INCLUDES}" STREQUAL "DEP_INCLUDES-NOTFOUND")
      get_target_property(DEP_INCLUDES ${dep} INTERFACE_INCLUDE_DIRECTORIES)
      if("${DEP_INCLUDES}" STREQUAL "DEP_INCLUDES-NOTFOUND")
        message(WARNING "Could NOT find include directories for ${deptar}.")
      endif()
    endif()
    target_include_directories(${tar} PUBLIC ${DEP_INCLUDES})
    target_link_libraries(${tar} PUBLIC ${dep})
    add_dependencies(${tar} ${dep})
  endforeach()
endmacro()

# Link various targets by first finding pre-built versions,
# using external/ as fallback. Example:
#
#   target_dependencies(target SDL2 SDL2_image sexp tinygettext ...)
#
macro(target_dependencies tar)
  set(deps ${ARGN})
  foreach(dep ${deps})
    find_package(${dep} QUIET)

    if(NOT ${${dep}_FOUND})
      if(USE_PKGCONFIG)
        message(VERBOSE "Could not find ${dep} with find_package. Falling back to pkg-config.")

        pkg_search_module(${dep} REQUIRED ${dep})
        if(NOT ${${dep}_FOUND})
          message(VERBOSE "Could not find ${dep} in pkg-config. Falling back to external/.")
        else()
          continue()
        endif()
      else()
        message(VERBOSE "Could not find ${dep} with find_package. Falling back to external/.")
      endif()

      target_external_dependencies(${tar} ${dep})
      continue()
    endif()

    target_link_libraries(${tar} PUBLIC ${dep})

    # try both names
    target_include_directories(${tar} PUBLIC ${${dep}_INCLUDE_DIR} ${${dep}_INCLUDE_DIRS})
  endforeach()
endmacro()
