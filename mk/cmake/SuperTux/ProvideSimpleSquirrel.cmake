if(EMSCRIPTEN)
  set(SQ_DISABLE_INSTALLER ON)
endif()

  add_subdirectory("${PROJECT_SOURCE_DIR}/external/simplesquirrel/")

if(MSVC)
  add_library(LibSimpleSquirrel ALIAS simplesquirrel_static)
else()
  add_library(LibSimpleSquirrel ALIAS simplesquirrel)
endif()

# Set variables
set(SQUIRREL_PREFIX ${PROJECT_BINARY_DIR}/external/simplesquirrel/libs/squirrel)
