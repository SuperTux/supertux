cmake_minimum_required(VERSION 3.6)

message("HELLO FROM CMAKE")

project(GAME)

#[[
# armeabi-v7a requires cpufeatures library
include(AndroidNdkModules)
android_ndk_import_module_cpufeatures()
]]

macro(cache_set var value type)
  set(${var} ${value} CACHE ${type} "" FORCE)
endmacro()


cache_set(SDL2IMAGE_INSTALL OFF BOOL)
cache_set(SDL2TTF_INSTALL OFF BOOL)
cache_set(SDL2NET_INSTALL OFF BOOL)
cache_set(SDL2MIXER_INSTALL OFF BOOL)
cache_set(SDL2IMAGE_SAMPLES OFF BOOL)
cache_set(SDL2NET_SAMPLES OFF BOOL)
cache_set(SDL2TTF_SAMPLES OFF BOOL)
cache_set(SDL2MIXER_SAMPLES OFF BOOL)
cache_set(SDL2MIXER_OPUS_SHARED ON BOOL)
cache_set(SDL2MIXER_OPUS OFF BOOL)
cache_set(SDL2MIXER_FLAC OFF BOOL)
cache_set(SDL2TTF_VENDORED ON BOOL)
cache_set(SDL2MIXER_VENDORED ON BOOL)

# watch this backfire terribly
cache_set(CURL_USE_OPENSSL OFF BOOL)

cache_set(OGG_LIBRARY ogg STRING)
cache_set(OGG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/ogg/include/ogg PATH)
cache_set(VORBIS_LIBRARY vorbis STRING)
cache_set(VORBIS_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/vorbis/include/vorbis PATH)

option(BUILD_DIGIVICE "" ON)

function(dump_cmake_variables)
  message("bagingi")
  get_cmake_property(_variableNames VARIABLES)
  list(SORT _variableNames)
  foreach(_variableName ${_variableNames})
    message("${_variableName}=${${_variableName}}")
  endforeach()
endfunction()

# Compilation of companion libraries
macro(subdirlist result curdir)
  file(GLOB children ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${child})
      list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

subdirlist(DEPS ${PROJECT_SOURCE_DIR})
message("DEPS: ${DEPS}")
list(APPEND CMAKE_PREFIX_PATH ${DEPS})
message("CMAKE PREFIX PATH: ${CMAKE_PREFIX_PATH}")
foreach(DEP ${DEPS})
  message("DEP: ${DEP}")
  add_subdirectory(${DEP})
endforeach()

#[[
add_subdirectory(SDL)
add_subdirectory(SDL_image)
add_subdirectory(SDL_ttf)
add_subdirectory(boost)
add_subdirectory(curl)
add_subdirectory(fmt)
add_subdirectory(glm)
add_subdirectory(ogg)
dump_cmake_variables()
add_subdirectory(vorbis)
add_subdirectory(openal-soft)
add_subdirectory(physfs)
add_subdirectory(sexp-cpp)
add_subdirectory(squirrel)
add_subdirectory(tinygettext)

add_subdirectory(supertux)
]]
