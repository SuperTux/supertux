set(CMAKE_EXECUTABLE_SUFFIX .html)
set(IS_EMSCRIPTEN_BUILD ON)
set(SQ_DISABLE_INSTALLER YES)
set(SSQ_BUILD_INSTALL NO)

set(EM_USE_FLAGS "-sDISABLE_EXCEPTION_CATCHING=0 -fPIC")
# SDL3_image decodes real file bytes; browser preload plugins are unnecessary.
set(EM_LINK_FLAGS " -sINITIAL_MEMORY=134217728 -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=536870912 -sERROR_ON_UNDEFINED_SYMBOLS=1 --preload-file ${BUILD_CONFIG_DATA_DIR} --pre-js ${PROJECT_SOURCE_DIR}/mk/emscripten/storage.js -lidbfs.js")
if(ENABLE_OPENGL)
  set(EM_LINK_FLAGS "${EM_LINK_FLAGS} -sFULL_ES2")
  set(HAVE_OPENGL ON CACHE BOOL "")
  set(USE_OPENGLES2 ON CACHE BOOL "")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(EM_USE_FLAGS "${EM_USE_FLAGS} -fsanitize=undefined")
  set(EM_LINK_FLAGS "${EM_LINK_FLAGS} -fsanitize=undefined -sSAFE_HEAP=1 -sASSERTIONS=1")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EM_USE_FLAGS} ${EM_C_FLAGS}")
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  string(APPEND CMAKE_CXX_FLAGS " -Wno-lifetime-safety-intra-tu-suggestions")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EM_USE_FLAGS} ${EM_C_FLAGS}")
# Do not append these twice: a repeated --pre-js evaluates the storage initializer
# twice and tries to mount/hydrate the same directory concurrently.
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EM_LINK_FLAGS}")

add_library(OpenAL INTERFACE IMPORTED)
set_target_properties(OpenAL PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/mk/emscripten/AL"
  INTERFACE_LINK_LIBRARIES "-lopenal"
)
