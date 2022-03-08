option(USE_SYSTEM_SQUIRREL "Use preinstalled squirrel if available" ON)
if(USE_SYSTEM_SQUIRREL)
  find_package(squirrel QUIET)
endif()

if(TARGET squirrel::squirrel)
  message(STATUS "Found preinstalled squirrel")
else()
  if(USE_SYSTEM_SQUIRREL)
    message(STATUS "Could NOT find squirrel, using external/squirrel fallback")
  endif()

  if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/squirrel/CMakeLists.txt)
    message(FATAL_ERROR "squirrel submodule is not checked out or ${CMAKE_CURRENT_SOURCE_DIR}/external/squirrel/CMakeLists.txt is missing")
  endif()

  if(CMAKE_CROSSCOMPILING)
    set(SQUIRREL_MULTIARCH_DIR "${CMAKE_LIBRARY_ARCHITECTURE}/")
  else()
    set(SQUIRREL_MULTIARCH_DIR "")
  endif()

  set(SQUIRREL_PREFIX ${CMAKE_BINARY_DIR}/squirrel/ex)
  ExternalProject_Add(squirrel_project
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/external/squirrel/"
    BUILD_BYPRODUCTS
    "${SQUIRREL_PREFIX}/lib/${SQUIRREL_MULTIARCH_DIR}${CMAKE_STATIC_LIBRARY_PREFIX}sqstdlib_static${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${SQUIRREL_PREFIX}/lib/${SQUIRREL_MULTIARCH_DIR}${CMAKE_STATIC_LIBRARY_PREFIX}squirrel_static${CMAKE_STATIC_LIBRARY_SUFFIX}"
    CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    -DCMAKE_INSTALL_PREFIX=${SQUIRREL_PREFIX}
    -DINSTALL_INC_DIR=include
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON)

  if(WIN32)
    add_library(squirrel::squirrel SHARED IMPORTED)
    set_target_properties(squirrel::squirrel PROPERTIES IMPORTED_LOCATION "${SQUIRREL_PREFIX}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}squirrel${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set_target_properties(squirrel::squirrel PROPERTIES IMPORTED_IMPLIB "${SQUIRREL_PREFIX}/lib/squirrel${CMAKE_LINK_LIBRARY_SUFFIX}")
    add_library(squirrel::sqstdlib SHARED IMPORTED)
    set_target_properties(squirrel::sqstdlib PROPERTIES IMPORTED_LOCATION "${SQUIRREL_PREFIX}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}sqstdlib${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set_target_properties(squirrel::sqstdlib PROPERTIES IMPORTED_IMPLIB "${SQUIRREL_PREFIX}/lib/sqstdlib${CMAKE_LINK_LIBRARY_SUFFIX}")

    #For debug run purposes
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/mk/msvc/run_supertux.bat.in" "${CMAKE_CURRENT_BINARY_DIR}/run_supertux.bat")
  else()
    add_library(squirrel::squirrel STATIC IMPORTED)
    set_target_properties(squirrel::squirrel PROPERTIES IMPORTED_LOCATION "${SQUIRREL_PREFIX}/lib/${SQUIRREL_MULTIARCH_DIR}${CMAKE_STATIC_LIBRARY_PREFIX}squirrel_static${CMAKE_STATIC_LIBRARY_SUFFIX}")
    add_library(squirrel::sqstdlib STATIC IMPORTED)
    set_target_properties(squirrel::sqstdlib PROPERTIES IMPORTED_LOCATION "${SQUIRREL_PREFIX}/lib/${SQUIRREL_MULTIARCH_DIR}${CMAKE_STATIC_LIBRARY_PREFIX}sqstdlib_static${CMAKE_STATIC_LIBRARY_SUFFIX}")
  endif()

  # Pre-create directory so that cmake doesn't complain about its non-existance
  file(MAKE_DIRECTORY ${SQUIRREL_PREFIX}/include)

  target_include_directories(squirrel::squirrel SYSTEM INTERFACE ${SQUIRREL_PREFIX}/include)
  add_dependencies(squirrel::squirrel squirrel_project)
endif()

# EOF #
