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
