# FIXME: Very janky development enviroment!
cmake_minimum_required(VERSION 3.6)

message("HELLO FROM CMAKE")

project(GAME)

# armeabi-v7a requires cpufeatures library
include(AndroidNdkModules)
android_ndk_import_module_cpufeatures()

macro(cache_set var value type)
  set(${var} ${value} CACHE ${type} "" FORCE)
endmacro()

function(dump_cmake_variables)
  message("bagingi")
  get_cmake_property(_variableNames VARIABLES)
  list(SORT _variableNames)
  foreach(_variableName ${_variableNames})
    message("${_variableName}=${${_variableName}}")
  endforeach()
endfunction()

message("MARTY: ${CMAKE_BINARY_DIR}")

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

cache_set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v" STRING)
cache_set(CMAKE_VERBOSE_MAKEFILE ON BOOL)

cache_set(SDL2_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/SDL/include PATH)
cache_set(SDL2IMAGE_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/SDL_image PATH)
cache_set(PNG_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/libpng PATH)
cache_set(PNG_LIBRARY png STRING)
cache_set(PNG_SHARED ON BOOL)
add_subdirectory(libpng)

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
cache_set(SDL2_DISABLE_UNINSTALL ON BOOL)
cache_set(SDL2_LIBRARIES SDL2 STRING)
cache_set(SDL2IMAGE_PNG ON BOOL)
cache_set(SDL2IMAGE_PNG_VENDORED ON BOOL)
add_subdirectory(SDL)
add_subdirectory(SDL_image)
add_subdirectory(SDL_ttf)

cache_set(OPENSSL_ROOT_DIR "${PROJECT_SOURCE_DIR}/openssl/" STRING)
cache_set(OPENSSL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/openssl/include" STRING)
cache_set(OPENSSL_CRYPTO_LIBRARY "${PROJECT_SOURCE_DIR}/openssl/crypto" STRING)
cache_set(OPENSSL_SSL_LIBRARY "${PROJECT_SOURCE_DIR}/openssl/ssl" STRING)
add_subdirectory(openssl-cmake)

# watch this backfire terribly
cache_set(CURL_USE_OPENSSL ON BOOL)
cache_set(CURL_BUILD_EXE OFF BOOL)
#cache_set(HTTP_ONLY ON BOOL)
cache_set(BUILD_TESTING OFF BOOL)
cache_set(CURL_ENABLE_EXPORT_TARGET OFF BOOL)
cache_set(CURL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/curl/include" STRING)
cache_set(CURL_LIBRARY libcurl STRING)
add_subdirectory(curl)

cache_set(USE_SYSTEM_TINYGETTEXT ON BOOL)
cache_set(TINYGETTEXT_LIBRARY tinygettext STRING)
cache_set(TINYGETTEXT_WITH_SDL ON BOOL)
cache_set(HAVE_SDL ON BOOL)
cache_set(TINYGETTEXT_PATH "${PROJECT_SOURCE_DIR}/tinygettext/" STRING)
add_subdirectory(supertux/external/tinygettext)

cache_set(OPENAL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/openal-soft/include/AL" STRING)
cache_set(OPENAL_LIBRARY OpenAL STRING)
add_subdirectory(openal-soft)

cache_set(OGG_LIBRARY ogg STRING)
cache_set(OGG_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/ogg/include" STRING)
add_subdirectory(ogg)

cache_set(VORBIS_LIBRARY vorbis STRING)
cache_set(VORBIS_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/vorbis/include;${PROJECT_SOURCE_DIR}/ogg/include" STRING)
cache_set(VORBISFILE_LIBRARY vorbisfile STRING)
add_subdirectory(vorbis)

cache_set(PHYSFS_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/physfs/src/" STRING)
cache_set(BUILD_TESTS OFF BOOL)
cache_set(PHYSFS_BUILD_SHARED ON BOOL)
cache_set(PHYSFS_BUILD_TEST OFF BOOL)
cache_set(PHYSFS_BUILD_WX_TEST OFF BOOL)
add_subdirectory(supertux/external/physfs)

cache_set(USE_SYSTEM_PHYSFS ON BOOL)

cache_set(glm_FOUND ON BOOL)
cache_set(USE_SYSTEM_GLM OFF BOOL)
add_subdirectory(supertux/external/glm)

add_subdirectory(supertux)

#[[
add_dependencies(supertux2
    SDL2
    SDL2_image
    png
    SDL2_ttf
    ssl
    crypto
    libcurl
    fmt
    glm
    ogg
    vorbis
    OpenAL
    physfs-static
    physfs
    sexp
    squirrel
    sqstdlib
    tinygettext
)
]]
