if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

if(LINUX)
  option(USE_PKGCONFIG "Also try to look for dependencies using pkg-config." ON)
else()
  option(USE_PKGCONFIG "Also try to look for dependencies using pkg-config." OFF)
endif()

if(USE_PKGCONFIG)
  find_package(PkgConfig QUIET)
  set(USE_PKGCONFIG ${PKG_CONFIG_FOUND})
endif()

# How nice of you.
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Add a subfolder from ${PROJECT_SOURCE_DIR}/external/
# and link a target with the same name. Example:
#
#   target_external_dependencies(target SDL2 SDL2_image ...)
#
macro(target_external_dependencies tar subdir)
  set(deps "${ARGN}")

  set(oldbuildtype ${CMAKE_BUILD_TYPE})

  message(STATUS "Adding ${subdir}")
  add_subdirectory(${PROJECT_SOURCE_DIR}/${subdir} EXCLUDE_FROM_ALL)

  foreach(dep ${deps})
    set(deptar "${dep}")

    if(${deptar}_FOUND)
      message(STATUS "Already found ${deptar} earlier. Skipping.")
      continue()
    endif()

    message(STATUS "Linking ${deptar} from ${subdir}")

    string(TOUPPER ${deptar} UPPERDEP)

    set(${deptar}_FOUND OFF CACHE STRING "")
    set(${UPPERDEP}_FOUND OFF CACHE STRING "")

    if(NOT TARGET ${deptar})
      message(WARNING "Could NOT find ${deptar} in ${subdir}. Skipping.")
      continue()
    endif()

    set(${deptar}_FOUND ON CACHE STRING "")
    set(${UPPERDEP}_FOUND ON CACHE STRING "")

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
    set(${deptar}_INCLUDE_DIR ${DEP_INCLUDES} CACHE STRING "")
    set(${deptar}_INCLUDE_DIRS ${DEP_INCLUDES} CACHE STRING "")
    set(${UPPERDEP}_INCLUDE_DIR ${DEP_INCLUDES} CACHE STRING "")
    set(${UPPERDEP}_INCLUDE_DIRS ${DEP_INCLUDES} CACHE STRING "")

    if(${DEP_TYPE} STREQUAL "INTERFACE_LIBRARY" OR ${DEP_TYPE} STREQUAL "OBJECT_LIBRARY")
      target_link_libraries(${tar} PRIVATE ${deptar})
    else()
      set(DEP_OUT_PATH "$<TARGET_FILE:${deptar}>")

      # Try all names, again
      set(${deptar}_LIBRARY ${DEP_OUT_PATH} CACHE STRING "")
      set(${deptar}_LIBRARIES ${DEP_OUT_PATH} CACHE STRING "")
      set(${UPPERDEP}_LIBRARY ${DEP_OUT_PATH} CACHE STRING "")
      set(${UPPERDEP}_LIBRARIES ${DEP_OUT_PATH} CACHE STRING "")

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

# Link a library by first finding pre-built versions,
# only using external/ as fallback. Example:
#
#   target_dependencies(target SDL external/SDL2 SDL2)
#
function(target_dependencies tar dep subdir)
  set(extdeps ${ARGN})

  message(STATUS "Adding ${dep} to ${tar}")

  string(TOUPPER ${dep} UPPERDEP)
  find_package(${dep} QUIET)

  if(NOT (${${dep}_FOUND}) OR NOT (${${UPPERDEP}_FOUND}))
    if(USE_PKGCONFIG)
      message(STATUS "Could not find ${dep} with find_package. Falling back to pkg-config.")

      pkg_search_module(${dep} ${dep})
      if("${${dep}_MODULE_NAME} " STREQUAL " ")
        message(STATUS "Could not find ${dep} in pkg-config. Falling back to external/.")

        target_external_dependencies(${tar} ${subdir} ${extdeps})
        return()
      else()
        message(STATUS "Successfully found ${dep} with pkg-config (${${dep}_MODULE_NAME})")
      endif()
    else()
      message(STATUS "Could not find ${dep} with find_package. Falling back to external/.")

      target_external_dependencies(${tar} ${subdir} ${extdeps})
      return()
    endif()
  else()
    message(STATUS "Successfully found ${dep} with find_package")
  endif()

  # try all names
  string(TOUPPER ${dep} UPPERDEP)
  set(libraries ${${dep}_LIBRARY} ${${dep}_LIBRARIES} ${${UPPERDEP}_LIBRARY} ${${UPPERDEP}_LIBRARIES})
  message(STATUS "- ${dep} libraries: ${libraries}")

  target_link_libraries(${tar} PUBLIC ${dep})
  target_link_libraries(${tar} PUBLIC ${libraries})

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
endfunction()
