if(EMSCRIPTEN)
  set(SQ_DISABLE_INSTALLER ON)
endif()

if(ANDROID)
  include(ExternalProject)

  if(NOT EXISTS ${PROJECT_SOURCE_DIR}/external/simplesquirrel/CMakeLists.txt)
    message(FATAL_ERROR "simplesquirrel submodule is not checked out or ${PROJECT_SOURCE_DIR}/external/physfs/CMakeLists.txt is missing")
  endif()

  set(SSQ_PREFIX ${CMAKE_BINARY_DIR}/simplesquirrel)
  ExternalProject_Add(simplesquirrel_project
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/external/simplesquirrel/"
    BUILD_BYPRODUCTS
    "${SSQ_PREFIX}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}simplesquirrel${CMAKE_SHARED_LIBRARY_SUFFIX}"
    "${SSQ_PREFIX}/lib${LIB_SUFFIX}/simplesquirrel${CMAKE_LINK_LIBRARY_SUFFIX}"
    "${SSQ_PREFIX}/lib${LIB_SUFFIX}/${CMAKE_STATIC_LIBRARY_PREFIX}simplesquirrel${CMAKE_STATIC_LIBRARY_SUFFIX}"
    CMAKE_ARGS
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_INSTALL_PREFIX=${SSQ_PREFIX})

  # Pre-create directory so that cmake doesn't complain about its non-existance
  file(MAKE_DIRECTORY "${SSQ_PREFIX}/include/")
  add_library(LibSimpleSquirrel STATIC IMPORTED)
  set_target_properties(LibSimpleSquirrel PROPERTIES
    IMPORTED_LOCATION "${SSQ_PREFIX}/lib${LIB_SUFFIX}/${CMAKE_STATIC_LIBRARY_PREFIX}simplesquirrel${CMAKE_STATIC_LIBRARY_SUFFIX}"
    INTERFACE_INCLUDE_DIRECTORIES "${SSQ_PREFIX}/include/")

  add_dependencies(LibSimpleSquirrel simplesquirrel_project)
else()
  add_subdirectory("${PROJECT_SOURCE_DIR}/external/simplesquirrel")
  if(MSVC)
    add_library(LibSimpleSquirrel ALIAS simplesquirrel_static)
  else()
    add_library(LibSimpleSquirrel ALIAS simplesquirrel)
  endif()
  set(SQUIRREL_PREFIX ${PROJECT_BINARY_DIR}/external/simplesquirrel/libs/squirrel)
endif()

# Set variables
