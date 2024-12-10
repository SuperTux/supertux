include(GetGitRevisionDescription)

git_project_version(SUPERTUX_PACKAGE_VERSION)

if(NOT IS_SUPERTUX_RELEASE AND SUPERTUX_PACKAGE_VERSION)
  string(PREPEND SUPERTUX_PACKAGE_VERSION "dev ")
  set(SUPERTUX_VERSION_STRING "${GIT_BRANCH}-${GIT_HASH}")
endif()

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
