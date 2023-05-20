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

cache_set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -v" STRING)

cache_set(SDL2_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/SDL/include PATH)
cache_set(SDL2IMAGE_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/SDL_image PATH)
cache_set(PNG_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/libpng PATH)

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
cache_set(CURL_ENABLE_EXPORT_TARGET OFF BOOL)
cache_set(CURL_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/curl/include" STRING)

cache_set(PHYSFS_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/physfs/src/" STRING)

cache_set(TINYGETTEXT_WITH_SDL ON BOOL)
cache_set(TINYGETTEXT_PATH "${PROJECT_SOURCE_DIR}/tinygettext/" STRING)

cache_set(OPENAL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/openal-soft/include/AL" STRING)

subdirlist(BOOST_LIBS "${PROJECT_SOURCE_DIR}/boost/libs")
foreach(BOOST_LIB ${BOOST_LIBS})
  set(BOOST_LIB_INCLUDE ${BOOST_LIB}/include)
  if(NOT EXISTS ${BOOST_LIB_INCLUDE})
    continue()
  endif()
  cache_set(Boost_INCLUDE_DIR "${Boost_INCLUDE_DIR};${BOOST_LIB_INCLUDE}" STRING)
endforeach()
message("Boost includes: ${Boost_INCLUDE_DIR}")

cache_set(OGG_LIBRARY ogg STRING)
cache_set(OGG_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/ogg/include" STRING)
cache_set(VORBIS_LIBRARY vorbis STRING)
cache_set(VORBIS_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/vorbis/include;${PROJECT_SOURCE_DIR}/ogg/include" STRING)

cache_set(BUILD_TESTS OFF BOOL)
cache_set(PHYSFS_BUILD_TEST OFF BOOL)
cache_set(PHYSFS_BUILD_WX_TEST OFF BOOL)

cache_set(USE_SYSTEM_PHYSFS ON BOOL)
cache_set(USE_SYSTEM_TINYGETTEXT ON BOOL)

option(BUILD_DIGIVICE "" ON)

# Compilation of companion libraries
#[[
subdirlist(DEPS ${PROJECT_SOURCE_DIR})
cache_set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${PROJECT_SOURCE_DIR};${DEPS}" STRING)
foreach(DEP ${DEPS})
  if(${DEP} STREQUAL "supertux")
    continue()
  endif()

  message("DEP: ${DEP}")
  add_subdirectory(${DEP})
endforeach()
add_subdirectory(supertux)
#]]

add_subdirectory(SDL)
add_subdirectory(SDL_image)
add_subdirectory(SDL_ttf)
add_subdirectory(boost)
add_subdirectory(curl)
add_subdirectory(fmt)
add_subdirectory(glm)
add_subdirectory(ogg)
add_subdirectory(vorbis)
add_subdirectory(openal-soft)
add_subdirectory(physfs)
add_subdirectory(sexp-cpp)
add_subdirectory(squirrel)
add_subdirectory(tinygettext)

add_subdirectory(supertux)
