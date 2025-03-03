set(CMAKE_EXECUTABLE_SUFFIX .html)
set(IS_EMSCRIPTEN_BUILD ON)

set(EM_USE_FLAGS "-sDISABLE_EXCEPTION_CATCHING=0 -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS='[\"png\",\"jpg\"]' -sUSE_SDL_TTF=2 -lopenal -sUSE_VORBIS=1")
if(ENABLE_OPENGL)
  set(EM_USE_FLAGS "${EM_USE_FLAGS} -sFULL_ES2=1")
endif()

set(EM_LINK_FLAGS " -sINITIAL_MEMORY=134217728 -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=536870912 -sERROR_ON_UNDEFINED_SYMBOLS=0 --preload-file ${BUILD_CONFIG_DATA_DIR} --use-preload-plugins -lidbfs.js")

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(EM_USE_FLAGS "${EM_USE_FLAGS} -fsanitize=undefined")
  set(EM_LINK_FLAGS "${EM_LINK_FLAGS} -fsanitize=undefined -sSAFE_HEAP=1 -sASSERTIONS=1 -sDEMANGLE_SUPPORT=1")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EM_USE_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EM_USE_FLAGS}")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${EM_USE_FLAGS} ${EM_LINK_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${EM_USE_FLAGS} ${EM_LINK_FLAGS}")

target_link_options(supertux2 PUBLIC -sEXPORTED_FUNCTIONS=['_main','_set_resolution','_save_config','_onDownloadProgress','_onDownloadFinished','_onDownloadError','_onDownloadAborted','_getExceptionMessage'] PUBLIC -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap'])
