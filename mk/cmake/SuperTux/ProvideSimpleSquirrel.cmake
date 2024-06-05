if(EMSCRIPTEN)
  set(SSQ_BUILD_INSTALL OFF)
endif()

add_subdirectory("${CMAKE_SOURCE_DIR}/external/simplesquirrel/")

if(MSVC)
  add_library(LibSimpleSquirrel ALIAS simplesquirrel_static)
else()
  add_library(LibSimpleSquirrel ALIAS simplesquirrel)
endif()

# Set variables
set(SQUIRREL_PREFIX ${CMAKE_BINARY_DIR}/external/simplesquirrel/libs/squirrel)
