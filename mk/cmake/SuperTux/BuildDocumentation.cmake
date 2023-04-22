## Configure doxygen
# Adapted from https://tty1.net/blog/2014/cmake-doxygen_en.html
# add a target to generate API documentation with Doxygen
option(BUILD_DOCUMENTATION "Build API documentation using Doxygen" ${DOXYGEN_FOUND})
option(BUILD_SCRIPTING_DOCUMENTATION "Build scripting API documentation using Doxygen" ${DOXYGEN_FOUND})

# CMakeLists, including this file from a different directory, can define PROJECT_SOURCE_DIR as the path to the SuperTux source.
if(NOT DEFINED PROJECT_SOURCE_DIR)
  # If not set, use the current source directory.
  set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(BUILD_DOCUMENTATION)
  find_package(Doxygen REQUIRED)

  set(doxyfile_in ${PROJECT_SOURCE_DIR}/Doxyfile.in)
  set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/docs/Doxyfile)

  configure_file(${doxyfile_in} ${doxyfile} @ONLY)

  add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)
endif()

if(BUILD_SCRIPTING_DOCUMENTATION)
  find_package(Doxygen REQUIRED)

  set(doxyfile_in ${PROJECT_SOURCE_DIR}/mk/doxygen/Doxyfile_scripting.in)
  set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/docs/scripting/Doxyfile)

  configure_file(${doxyfile_in} ${doxyfile} @ONLY)

  add_custom_target(scripting_doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating scripting API documentation with Doxygen"
    VERBATIM)
endif()

# EOF #
