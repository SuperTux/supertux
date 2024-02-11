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
# ${PROJECT_SOURCE_DIR}. Example:
#
#   target_external_dependencies_from_folder(target external/squirrel squirrel sqstdlib)
#
macro(target_external_dependencies_from_folder tar folder)
  cmake_parse_arguments(DEPS_ARGS "STATIC" "" "" ${ARGN})
  set(deps "${DEPS_ARGS_UNPARSED_ARGUMENTS}")

  set(subdir ${PROJECT_SOURCE_DIR}/${folder})
  set(oldbuildtype ${CMAKE_BUILD_TYPE})
  set(CMAKE_BUILD_TYPE Release)

  message(STATUS "Adding ${subdir}")
  add_subdirectory(${subdir} EXCLUDE_FROM_ALL)

  set(CMAKE_BUILD_TYPE ${oldbuildtype})

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

    if(${DEP_TYPE} STREQUAL "INTERFACE_LIBRARY")
      target_link_libraries(${tar} PRIVATE ${deptar})
    else()
      #get_target_property(DEP_OUT_NAME ${deptar} OUTPUT_NAME_${CMAKE_BUILD_TYPE})
      #get_target_property(DEP_OUT_DIR ${deptar} OUTPUT_DIRECTORY_${CMAKE_BUILD_TYPE})
      #message("AAAA ${deptar}")
      set(DEP_OUT_PATH "$<TARGET_FILE:${deptar}>")

      # Try all names, again
      set(${deptar}_LIBRARY "$<TARGET_FILE:${deptar}>")
      set(${deptar}_LIBRARIES "$<TARGET_FILE:${deptar}>")
      set(${UPPERDEP}_LIBRARY "$<TARGET_FILE:${deptar}>")
      set(${UPPERDEP}_LIBRARIES "$<TARGET_FILE:${deptar}>")
      #message("BBBB ${${UPPERDEP}_LIBRARY}")

      target_link_directories(${tar} PRIVATE $<TARGET_FILE_DIR:${deptar}>)
      target_link_libraries(${tar} PRIVATE
        # CMake generator expressions is my favourite programming language.
        #$<$<NOT:$<STREQUAL:$<TARGET_FILE_BASE_NAME:${deptar}>, >>:
        $<IF:$<BOOL:$<TARGET_FILE_PREFIX:${deptar}>>,
          $<TARGET_FILE_BASE_NAME:${deptar}>
        ,
          :$<TARGET_FILE_BASE_NAME:${deptar}>$<TARGET_FILE_SUFFIX:${deptar}>
        >
        #>
      )

      #[[
      add_custom_command(TARGET ${tar}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${deptar}>
        $<TARGET_FILE_DIR:${tar}>
      )
      ]]
    endif()

  add_dependencies(${tar} ${deptar})
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
    find_package(${dep} QUIET)

    if(NOT ${${dep}_FOUND})
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
    message(STATUS "${dep} libraries: ${libraries}")

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

    message(STATUS "${dep} includes: ${includes}")
  endforeach()
endmacro()
