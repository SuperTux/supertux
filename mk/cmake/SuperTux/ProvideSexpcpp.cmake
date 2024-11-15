option(USE_SYSTEM_SEXPCPP "Use preinstalled sexp-cpp if available" ON)

if(USE_SYSTEM_SEXPCPP)
  find_package(sexp QUIET)
endif()

if(TARGET sexp)
  message(STATUS "Found preinstalled sexp-cpp")
  add_library(LibSexp ALIAS sexp)
elseif(TARGET sexp::sexp)
  message(STATUS "Found preinstalled sexp-cpp")
  add_library(LibSexp ALIAS sexp::sexp)
else()
  if(USE_SYSTEM_SEXPCPP)
    message(STATUS "Could NOT find sexp-cpp, using external/sexp-cpp fallback")
  endif()

  if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/sexp-cpp/CMakeLists.txt)
    message(FATAL_ERROR "sexp-cpp submodule is not checked out or ${CMAKE_CURRENT_SOURCE_DIR}/external/sexp-cpp/CMakeLists.txt is missing")
  endif()

  set(SEXP_PREFIX ${CMAKE_BINARY_DIR}/sexp-cpp)
  ExternalProject_Add(sexp_project
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/external/sexp-cpp/"
    BUILD_BYPRODUCTS
    "${SEXP_PREFIX}/include"
    CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    -DCMAKE_INSTALL_PREFIX=${SEXP_PREFIX}
    -DBUILD_TESTS=OFF
    -DBUILD_BENCHMARKS=OFF)

  add_library(LibSexp INTERFACE IMPORTED)

  file(MAKE_DIRECTORY ${SEXP_PREFIX}/include)

  set_target_properties(LibSexp PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${INTERFACE_INCLUDE_DIRECTORIES};${SEXP_PREFIX}/include/"
    INTERFACE_COMPILE_DEFINITIONS "${INTERFACE_COMPILE_DEFINITIONS};SEXP_USE_LOCALE")

  add_dependencies(LibSexp sexp_project) 
endif()

# EOF #
