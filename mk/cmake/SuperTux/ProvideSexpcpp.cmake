option(USE_SYSTEM_SEXPCPP "Use preinstalled sexp-cpp if available" ON)

if(USE_SYSTEM_SEXPCPP)
  find_package(sexp QUIET)
endif()

if(TARGET sexp)
  message(STATUS "Found preinstalled sexp-cpp")
else()
  if(USE_SYSTEM_SEXPCPP)
    message(STATUS "Could NOT find sexp-cpp, using external/sexp-cpp fallback")
  endif()

  if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/sexp-cpp/CMakeLists.txt)
    message(FATAL_ERROR "sexp-cpp submodule is not checked out or ${CMAKE_CURRENT_SOURCE_DIR}/external/sexp-cpp/CMakeLists.txt is missing")
  endif()

  file(GLOB SEXP_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} external/sexp-cpp/src/*.cpp)

  add_library(sexp STATIC ${SEXP_SOURCES_CXX})
  target_compile_definitions(sexp PRIVATE -DSEXP_USE_LOCALE)
  target_include_directories(sexp SYSTEM PUBLIC external/sexp-cpp/include/)
endif()

# EOF #
