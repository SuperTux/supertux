include(GetGitRevisionDescription)

git_project_version(SUPERTUX_PACKAGE_VERSION ${IS_SUPERTUX_RELEASE})

if(NOT IS_SUPERTUX_RELEASE AND SUPERTUX_PACKAGE_VERSION)
  string(PREPEND SUPERTUX_PACKAGE_VERSION "dev ")
  set(SUPERTUX_VERSION_STRING "${GIT_BRANCH}-${GIT_HASH}")
else()
  set(SUPERTUX_VERSION_STRING "${GIT_TAG}")
endif()

message(STATUS "Git version checking results:")
message(STATUS "  GIT_TAG: ${GIT_TAG}")
message(STATUS "  GIT_HASH: ${GIT_HASH}")
message(STATUS "  GIT_BRANCH: ${GIT_BRANCH}")
message(STATUS "  SUPERTUX_PACKAGE_VERSION: ${SUPERTUX_PACKAGE_VERSION}")
message(STATUS "  SUPERTUX_VERSION_STRING: ${SUPERTUX_VERSION_STRING}")

configure_file(version.h.in ${CMAKE_BINARY_DIR}/version.h)

set_source_files_properties(${CMAKE_BINARY_DIR}/version.h
  PROPERTIES GENERATED true)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/supertux/main.cpp
  PROPERTIES OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/version.h")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/supertux/title_screen.cpp
  PROPERTIES OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/version.h")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/addon/addon_manager.cpp
  PROPERTIES OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/version.h")

# EOF #
