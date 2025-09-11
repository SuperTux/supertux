# FIXME: Very janky development enviroment!
cmake_minimum_required(VERSION 3.6)

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

message("ANDROID_ABI: ${ANDROID_ABI}")
message("VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")
message("CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")

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

function(message lvl msg)
  if(lvl STREQUAL "FATAL_ERROR")
    _message(FATAL_ERROR "${msg}")
  endif()

  _message("${msg}")
endfunction()

add_subdirectory(supertux)
