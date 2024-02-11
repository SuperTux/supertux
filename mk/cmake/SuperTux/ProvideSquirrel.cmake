option(USE_SYSTEM_SQUIRREL "Use preinstalled squirrel if available" ON)

if(USE_SYSTEM_SQUIRREL)
  find_package(squirrel 3.2 QUIET)
endif()

if(TARGET squirrel::squirrel)
  message(STATUS "Found preinstalled squirrel")

  add_library(LibSquirrel ALIAS squirrel::squirrel)
  add_library(LibSqstdlib ALIAS squirrel::sqstdlib)

  # Define these for usage by SimpleSquirrel
  get_property(SQUIRREL_INCLUDE_DIR TARGET LibSquirrel PROPERTY INCLUDE_DIRECTORIES)
  get_property(SQUIRREL_LIBRARIES TARGET LibSquirrel PROPERTY IMPORTED_LOCATION)
  get_property(SQSTDLIB_LIBRARIES TARGET LibSqstdlib PROPERTY IMPORTED_LOCATION)
else()
  if(USE_SYSTEM_SQUIRREL)
    message(STATUS "Could NOT find squirrel, using external/squirrel fallback")
  endif()

  if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/squirrel/CMakeLists.txt)
    message(FATAL_ERROR "squirrel submodule is not checked out or ${CMAKE_CURRENT_SOURCE_DIR}/external/squirrel/CMakeLists.txt is missing")
  endif()

  if(CMAKE_CROSSCOMPILING AND NOT EMSCRIPTEN)
    set(SQUIRREL_MULTIARCH_DIR "${CMAKE_LIBRARY_ARCHITECTURE}/")
  else()
    set(SQUIRREL_MULTIARCH_DIR "")
  endif()

  # Cannot build Debug version when using gcc on arm64 because an incompatible flag is being passed in
  if(CMAKE_COMPILER_IS_GNUCXX AND "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "aarch64" AND "${CMAKE_BUILD_TYPE}" MATCHES "Debug")
    set (BUILD_TYPE "RelWithDebInfo")
  else()
    set (BUILD_TYPE ${CMAKE_BUILD_TYPE})
  endif()
  
  set(SQUIRREL_PREFIX ${CMAKE_BINARY_DIR}/squirrel/ex)

  # Define these for usage by SimpleSquirrel
  set(SQUIRREL_INCLUDE_DIR "${SQUIRREL_PREFIX}/include")
  set(SQUIRREL_LIBRARIES "${SQUIRREL_PREFIX}/lib/${SQUIRREL_MULTIARCH_DIR}${CMAKE_STATIC_LIBRARY_PREFIX}squirrel_static${CMAKE_STATIC_LIBRARY_SUFFIX}")
  set(SQSTDLIB_LIBRARIES "${SQUIRREL_PREFIX}/lib/${SQUIRREL_MULTIARCH_DIR}${CMAKE_STATIC_LIBRARY_PREFIX}sqstdlib_static${CMAKE_STATIC_LIBRARY_SUFFIX}")

  ExternalProject_Add(squirrel_project
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/external/squirrel/"
    BUILD_BYPRODUCTS ${SQUIRREL_LIBRARIES} ${SQSTDLIB_LIBRARIES}
    CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    -DCMAKE_INSTALL_PREFIX=${SQUIRREL_PREFIX}
    -DCMAKE_INSTALL_LIBDIR=lib
    -DINSTALL_INC_DIR=include
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON)

  if(WIN32)
    add_library(LibSquirrel SHARED IMPORTED)
    set_target_properties(LibSquirrel PROPERTIES
      IMPORTED_LOCATION "${SQUIRREL_PREFIX}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}squirrel${CMAKE_SHARED_LIBRARY_SUFFIX}"
      IMPORTED_IMPLIB "${SQUIRREL_PREFIX}/lib/squirrel${CMAKE_LINK_LIBRARY_SUFFIX}"
      INTERFACE_INCLUDE_DIRECTORIES ${SQUIRREL_INCLUDE_DIR})

    add_library(LibSqstdlib SHARED IMPORTED)
    set_target_properties(LibSqstdlib PROPERTIES
      IMPORTED_LOCATION "${SQUIRREL_PREFIX}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}sqstdlib${CMAKE_SHARED_LIBRARY_SUFFIX}"
      IMPORTED_IMPLIB "${SQUIRREL_PREFIX}/lib/sqstdlib${CMAKE_LINK_LIBRARY_SUFFIX}"
      INTERFACE_INCLUDE_DIRECTORIES ${SQUIRREL_INCLUDE_DIR})

    #For debug run purposes
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/mk/msvc/run_supertux.bat.in" "${CMAKE_CURRENT_BINARY_DIR}/run_supertux.bat")
  else()
    add_library(LibSquirrel STATIC IMPORTED)
    set_target_properties(LibSquirrel PROPERTIES
      IMPORTED_LOCATION ${SQUIRREL_LIBRARIES}
      INTERFACE_INCLUDE_DIRECTORIES ${SQUIRREL_INCLUDE_DIR})

    add_library(LibSqstdlib STATIC IMPORTED)
    set_target_properties(LibSqstdlib PROPERTIES
      IMPORTED_LOCATION ${SQSTDLIB_LIBRARIES}
      INTERFACE_INCLUDE_DIRECTORIES ${SQUIRREL_INCLUDE_DIR})
  endif()

  # Pre-create directory so that cmake doesn't complain about its non-existance
  file(MAKE_DIRECTORY ${SQUIRREL_PREFIX}/include)

  add_dependencies(LibSquirrel squirrel_project)
  add_dependencies(LibSqstdlib squirrel_project)

  if(WIN32)
    get_property(SQUIRREL_LIBRARIES TARGET LibSquirrel PROPERTY IMPORTED_LOCATION)
    get_property(SQSTDLIB_LIBRARIES TARGET LibSqstdlib PROPERTY IMPORTED_LOCATION)
    install(FILES ${SQUIRREL_LIBRARIES} ${SQSTDLIB_LIBRARIES} DESTINATION "${INSTALL_SUBDIR_BIN}")
  endif()
endif()

# EOF #
