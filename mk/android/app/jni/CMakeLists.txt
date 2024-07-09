cmake_minimum_required(VERSION 3.6)

project(GAME)

# armeabi-v7a requires cpufeatures library
# Normal versions of CMake don't provide
# AndroidNdkModules.cmake. With that said:

# Copyright (c) 2014, Pavel Rojtberg
# All rights reserved.

macro(android_ndk_import_module_cpufeatures)
  if(ANDROID)
    include_directories(${ANDROID_NDK}/sources/android/cpufeatures)
    add_library(cpufeatures ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)
    target_link_libraries(cpufeatures dl)
  endif()
endmacro()

android_ndk_import_module_cpufeatures()

add_subdirectory(supertux)
