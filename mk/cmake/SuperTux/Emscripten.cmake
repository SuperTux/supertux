set(CMAKE_EXECUTABLE_SUFFIX .html)
set(IS_EMSCRIPTEN_BUILD ON)
set(SQ_DISABLE_INSTALLER YES)
set(SSQ_BUILD_INSTALL NO)

set(EM_USE_FLAGS "-sDISABLE_EXCEPTION_CATCHING=0 -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS='[\"png\",\"jpg\"]' -sUSE_VORBIS=1")
set(EM_LINK_FLAGS " -sINITIAL_MEMORY=134217728 -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=536870912 -sERROR_ON_UNDEFINED_SYMBOLS=0 --preload-file ${BUILD_CONFIG_DATA_DIR} --use-preload-plugins -lidbfs.js")
if(ENABLE_OPENGL)
  set(EM_USE_FLAGS "${EM_USE_FLAGS} -sFULL_ES2=1")
  set(HAVE_OPENGL YES)
  set(USE_OPENGLES2 YES)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(EM_USE_FLAGS "${EM_USE_FLAGS} -fsanitize=undefined")
  set(EM_LINK_FLAGS "${EM_LINK_FLAGS} -fsanitize=undefined -sSAFE_HEAP=1 -sASSERTIONS=1 -sDEMANGLE_SUPPORT=1")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EM_USE_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EM_USE_FLAGS}")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${EM_USE_FLAGS} ${EM_LINK_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${EM_USE_FLAGS} ${EM_LINK_FLAGS}")

add_library(OpenAL INTERFACE IMPORTED)
set_target_properties(OpenAL PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/mk/emscripten/AL"
  INTERFACE_LINK_LIBRARIES "-lopenal"
)

add_library(SDL2 INTERFACE IMPORTED)
set_target_properties(SDL2 PROPERTIES
  INTERFACE_COMPILE_OPTIONS "-sUSE_SDL=2"
  INTERFACE_LINK_LIBRARIES "-sUSE_SDL=2"
)
