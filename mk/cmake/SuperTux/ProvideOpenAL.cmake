if(NOT EMSCRIPTEN)
  if(VCPKG_BUILD)
    find_package(OpenAL CONFIG REQUIRED)
  else()
    find_package(OpenAL REQUIRED)
  endif()
  include_directories(SYSTEM ${OPENAL_INCLUDE_DIR})
else()
  include_directories(${CMAKE_SOURCE_DIR}/mk/emscripten/AL)
endif()

mark_as_advanced(
  OPENAL_INCLUDE_DIR
  OPENAL_LIBRARY
  )

# EOF #
