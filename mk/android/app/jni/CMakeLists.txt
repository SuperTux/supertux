cmake_minimum_required(VERSION 3.6)

message("HELLO FROM CMAKE")
message("CMAKE CURDIR ${CMAKE_CURRENT_SOURCE_DIR}")

project(GAME)

#[[
# armeabi-v7a requires cpufeatures library
include(AndroidNdkModules)
android_ndk_import_module_cpufeatures()
]]

set(SDL2IMAGE_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL2TTF_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL2NET_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2NET_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2TTF_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_OPUS_SHARED ON CACHE BOOL "" FORCE)
set(SDL2MIXER_OPUS OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_FLAC OFF CACHE BOOL "" FORCE)
set(SDL2TTF_VENDORED ON CACHE BOOL "" FORCE)
set(SDL2MIXER_VENDORED ON CACHE BOOL "" FORCE)

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
function(subdirlist result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endfunction()

message("PROJ SRC DIR ${PROJECT_SOURCE_DIR}")

add_subdirectory(SDL_image)
add_subdirectory(SDL_ttf)
add_subdirectory(boost)
add_subdirectory(curl)
add_subdirectory(fmt)
add_subdirectory(glm)
add_subdirectory(vorbis)
add_subdirectory(openal-soft)
add_subdirectory(physfs)
add_subdirectory(sexp-cpp)
add_subdirectory(squirrel)
add_subdirectory(tinygettext)

add_subdirectory(supertux)
