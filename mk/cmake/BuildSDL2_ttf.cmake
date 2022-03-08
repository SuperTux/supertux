option(USE_SYSTEM_SDL2_TTF "Use preinstalled SDL2_ttf if available" ON)
if(USE_SYSTEM_SDL2_TTF)
  find_package(SDL2_ttf QUIET)
endif()

if(TARGET SDL2_ttf)
  message(STATUS "Found preinstalled SDL2_ttf")
else()
  message(STATUS "Could NOT find SDL2_ttf, using external/SDL_ttf fallback")

  ## external/SDL_ttf with patches
  if (NOT EMSCRIPTEN)
    if (VCPKG_BUILD)
      find_package(freetype CONFIG REQUIRED)
    else()
      find_package(Freetype REQUIRED)
    endif()
  endif(NOT EMSCRIPTEN)

  find_package(RAQM)

  set(SDL2_TTF_PREFIX ${CMAKE_BINARY_DIR}/SDL2_ttf)
  ExternalProject_Add(SDL2_ttf_project
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/external/SDL_ttf/"
    BUILD_BYPRODUCTS "${SDL2_TTF_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2_ttf${CMAKE_STATIC_LIBRARY_SUFFIX}"
    CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_INSTALL_PREFIX=${SDL2_TTF_PREFIX}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DWITH_RAQM=${RAQM_FOUND}
    -DRAQM_LIBRARIES=${RAQM_LIBRARY}
    -DRAQM_INCLUDE_DIR=${RAQM_INCLUDE_DIR}
    -DBUILD_SHARED_LIBS=OFF
    -DEMSCRIPTEN=${EMSCRIPTEN})

  # Pre-create directory so that cmake doesn't complain about its non-existance
  file(MAKE_DIRECTORY ${SDL2_TTF_PREFIX}/include/SDL2)

  add_library(SDL2_ttf STATIC IMPORTED)
  set_target_properties(SDL2_ttf PROPERTIES
    IMPORTED_LOCATION "${SDL2_TTF_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2_ttf${CMAKE_STATIC_LIBRARY_SUFFIX}"
    INTERFACE_INCLUDE_DIRECTORIES "${SDL2_TTF_PREFIX}/include/SDL2")

  if(RAQM_FOUND)
    find_package(FriBidi REQUIRED)
    find_package(HarfBuzz REQUIRED)
    target_link_libraries(SDL2_ttf INTERFACE
      ${HARFBUZZ_LIBRARY}
      ${FRIBIDI_LIBRARY}
      ${RAQM_LIBRARY})
  endif()

  if(VCPKG_BUILD)
    target_link_libraries(SDL2_ttf INTERFACE freetype)
  else()
    target_link_libraries(SDL2_ttf INTERFACE ${FREETYPE_LIBRARIES})
  endif()

  add_dependencies(SDL2_ttf SDL2_ttf_project)
endif()

mark_as_advanced(
  SDL2TTF_INCLUDE_DIRS
  SDL2TTF_LIBRARIES
  )

# EOF #
