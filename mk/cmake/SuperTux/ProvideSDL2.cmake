message(STATUS "Checking for SDL2")
if(WIN32)
  if(VCPKG_BUILD)
    find_package(SDL2 CONFIG REQUIRED)
    find_library(SDL2MAIN_LIBRARIES SDL2main)
  else()
    find_path(SDL2_INCLUDE_DIRS NAMES SDL.h PATHS "${DEPENDENCY_FOLDER}/include/SDL2")
    find_library(SDL2_LIBRARIES NAMES SDL2 PATHS "${DEPENDENCY_FOLDER}/lib")
    find_library(SDL2MAIN_LIBRARIES NAMES SDL2main PATHS "${DEPENDENCY_FOLDER}/lib")
  endif()
else()
  if(NOT EMSCRIPTEN)
    if(VCPKG_BUILD)
      find_package(SDL2 CONFIG REQUIRED)
      find_path(SDL2_INCLUDE_DIRS SDL2/SDL.h)
      set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIRS}/SDL2)
      find_library(SDL2_LIBRARIES SDL2)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
      # 32-bit
      find_library(SDL2_LIBRARIES SDL2)
      find_path(SDL2_INCLUDE_DIRS SDL2/SDL.h)
      set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIRS}/SDL2)
    else()
      # 64-bit
      include(FindPkgConfig)
      if(NOT PKG_CONFIG_FOUND)
        message(WARNING "Trying to find libraries using pkg_search_module, but pkg-config was not found. Install pkg-config to continue.")
      else()
        pkg_search_module(SDL2 REQUIRED sdl2>=2.0.1)
      endif()
    endif()
  endif()
endif()
set(HAVE_SDL TRUE)

message(STATUS "  SDL2_LIBRARIES: ${SDL2_LIBRARIES}")
message(STATUS "  SDL2_INCLUDE_DIRS: ${SDL2_INCLUDE_DIRS}")

add_library(LibSDL2 INTERFACE IMPORTED)
set_target_properties(LibSDL2 PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
  INTERFACE_LINK_LIBRARIES "${SDL2_LIBRARIES}")

add_library(LibSDL2main INTERFACE IMPORTED)
set_target_properties(LibSDL2main PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
  INTERFACE_LINK_LIBRARIES "${SDL2MAIN_LIBRARIES}")

mark_as_advanced(
  SDL2_INCLUDE_DIRS
  SDL2_LIBRARIES
  )

# EOF #
