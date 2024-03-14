if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

if(LINUX)
  option(USE_PKGCONFIG "Also try to look for dependencies using pkg-config." ON)
  if(USE_PKGCONFIG)
    find_package(PkgConfig)
  endif()
endif()

# How nice of you.
#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Use a library from a subfolder in ${PROJECT_SOURCE_DIR}/external
# whose code is all in the root directory. Example:
#
#   target_use_minilib(target findlocale SDL_SavePNG ...)
#
macro(target_use_minilib tar)
  set(deps "${ARGN}")
  foreach(dep ${deps})
    message(STATUS "Using ${dep} minilib")

    set(subdir ${PROJECT_SOURCE_DIR}/external/${dep})
    if(NOT EXISTS ${subdir})
      message(WARNING "Could NOT find ${dep} minilib in ${subdir}. Skipping.")
      continue()
    endif()

    file(GLOB depsrc ${subdir}/*.cpp ${subdir}/*.c ${subdir}/*.hpp ${subdir}/*.h)
    target_sources(${tar} PRIVATE ${depsrc})
    #get_target_property(srcs ${tar} SOURCES)
    message("${subdir} BAH ${tar}")
    target_include_directories(${tar} PRIVATE ${subdir})
  endforeach()
endmacro()

# Link various targets that come from a subfolder in
# ${PROJECT_SOURCE_DIR}/external. Example:
#
#   target_external_dependencies_from_folder(target external/squirrel squirrel sqstdlib)
#
macro(target_external_dependencies_from_folder tar folder)
  cmake_parse_arguments(DEPS_ARGS "STATIC" "" "" ${ARGN})
  set(deps "${DEPS_ARGS_UNPARSED_ARGUMENTS}")

  set(subdir ${PROJECT_SOURCE_DIR}/${folder})
  set(oldbuildtype ${CMAKE_BUILD_TYPE})

  message(STATUS "Adding ${subdir}")
  add_subdirectory(${subdir} EXCLUDE_FROM_ALL)

  foreach(dep ${deps})
    set(deptar "${dep}")

    if(DEPS_ARGS_STATIC)
      set(deptar ${deptar}-static)
    endif()

    if(${deptar}_FOUND)
      message(STATUS "Already found ${deptar} earlier. Skipping.")
      continue()
    endif()

    message(STATUS "Linking ${deptar} from ${subdir}")

    if(DEPS_ARGS_STATIC AND NOT TARGET ${deptar})
      message(STATUS "Could not find ${deptar} in ${subdir}. Falling back to non-static version.")
      set(deptar ${dep})
    endif()

    string(TOUPPER ${deptar} UPPERDEP)

    set(${deptar}_FOUND OFF)
    set(${UPPERDEP}_FOUND OFF)

    if(NOT TARGET ${deptar})
      message(WARNING "Could NOT find ${deptar} in ${subdir}. Skipping.")
      continue()
    endif()

    set(${deptar}_FOUND ON)
    set(${UPPERDEP}_FOUND ON)

    get_target_property(DEP_INCLUDES ${deptar} INCLUDE_DIRECTORIES)
    if("${DEP_INCLUDES}" STREQUAL "DEP_INCLUDES-NOTFOUND")
      # Try to check interface include directories
      get_target_property(DEP_INCLUDES ${deptar} INTERFACE_INCLUDE_DIRECTORIES)
      if("${DEP_INCLUDES}" STREQUAL "DEP_INCLUDES-NOTFOUND")
        message(WARNING "Could NOT find include directories for ${deptar}.")
      endif()
    endif()

    get_target_property(DEP_TYPE ${deptar} TYPE)
    target_include_directories(${tar} PUBLIC ${DEP_INCLUDES})

    string(TOUPPER ${deptar} UPPERDEP)

    # Try all names
    set(${deptar}_INCLUDE_DIR ${DEP_INCLUDES})
    set(${deptar}_INCLUDE_DIRS ${DEP_INCLUDES})
    set(${UPPERDEP}_INCLUDE_DIR ${DEP_INCLUDES})
    set(${UPPERDEP}_INCLUDE_DIRS ${DEP_INCLUDES})

    if(${DEP_TYPE} STREQUAL "INTERFACE_LIBRARY" OR ${DEP_TYPE} STREQUAL "OBJECT_LIBRARY")
      target_link_libraries(${tar} PRIVATE ${deptar})
    else()
      set(DEP_OUT_PATH "$<TARGET_FILE:${deptar}>")

      # Try all names, again
      set(${deptar}_LIBRARY ${deptar})
      set(${deptar}_LIBRARIES ${deptar})
      set(${UPPERDEP}_LIBRARY ${deptar})
      set(${UPPERDEP}_LIBRARIES ${deptar})

      #target_link_directories(${tar} PRIVATE $<TARGET_FILE_DIR:${deptar}>)
      target_link_libraries(${tar} PRIVATE ${deptar})
    endif()

  add_dependencies(${tar} ${deptar})
  message(STATUS
    "${deptar} has been added.\n"
    "-- - ${deptar}_LIBRARY: ${${deptar}_LIBRARY}\n"
    "-- - ${deptar}_INCLUDE_DIR: ${${deptar}_INCLUDE_DIR}"
  )
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
    if(DEPS_ARGS_STATIC)
      target_external_dependencies_from_folder(${tar} external/${dep} STATIC ${dep})
    else()
      target_external_dependencies_from_folder(${tar} external/${dep} ${dep})
    endif()
  endforeach()
endmacro()

# Link various targets by first finding pre-built versions,
# using external/ as fallback. Example:
#
#   target_dependencies(target SDL2 SDL2_image sexp tinygettext ...)
#
macro(target_dependencies tar)
  set(deps ${ARGN})
  message(STATUS "Adding ${deps} to ${tar}")
  foreach(dep ${deps})
    message(STATUS "Linking ${dep}")
    string(TOUPPER ${dep} UPPERDEP)
    find_package(${dep} QUIET)

    if(NOT "${${dep}_FOUND}" STREQUAL "" OR NOT "${${UPPERDEP}_FOUND}" STREQUAL "")
      if(USE_PKGCONFIG)
        message(STATUS "Could not find ${dep} with find_package. Falling back to pkg-config.")

        pkg_search_module(${dep} ${dep})
        if("${${dep}_MODULE_NAME} " STREQUAL " ")
          message(STATUS "Could not find ${dep} in pkg-config. Falling back to external/.")

          target_external_dependencies(${tar} ${dep})
          continue()
        else()
          message(STATUS "Successfully found ${dep} with pkg-config (${${dep}_MODULE_NAME})")
        endif()
      else()
        message(STATUS "Could not find ${dep} with find_package. Falling back to external/.")

        target_external_dependencies(${tar} ${dep})
        continue()
      endif()
    else()
      message(STATUS "Successfully found ${dep} with find_package")
    endif()

    # try all names
    string(TOUPPER ${dep} UPPERDEP)
    set(libraries ${${dep}_LIBRARY} ${${dep}_LIBRARIES} ${${UPPERDEP}_LIBRARY} ${${UPPERDEP}_LIBRARIES})
    message(STATUS "- ${dep} libraries: ${libraries}")

    target_link_libraries(${tar} PUBLIC ${dep})

    #[[
    if("${libraries}" MATCHES "^ *$")
      target_link_libraries(${tar} PUBLIC ${dep})
    else()
      message("TEST")
      target_link_libraries(${tar} PUBLIC ${libraries})
    endif()
    #]]

    set(includes ${${dep}_INCLUDE_DIR} ${${dep}_INCLUDE_DIRS} ${${UPPERDEP}_INCLUDE_DIR} ${${UPPERDEP}_INCLUDE_DIRS} ${${dep}_INCLUDEDIR})

    target_include_directories(${tar} PUBLIC ${includes})

    message(STATUS "- ${dep} includes: ${includes}")
  endforeach()
endmacro()
