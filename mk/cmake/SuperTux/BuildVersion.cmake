include(GetGitRevisionDescription)

if(NOT IS_SUPERTUX_RELEASE AND NOT STEAM_BUILD)
  set(SUPERTUX_PACKAGE_VERSION dev)
  if(EXISTS "${PROJECT_SOURCE_DIR}/.git")
    git_get_hash(hash branch)

    if(branch)
      string(APPEND SUPERTUX_PACKAGE_VERSION " ${branch}")
    endif()

    if(hash)
      string(APPEND SUPERTUX_PACKAGE_VERSION " ${hash}")
    endif()
  endif()
else()
  set(SUPERTUX_PACKAGE_VERSION v${PROJECT_VERSION})
endif()

# Configure main menu logo
if(IS_SUPERTUX_RELEASE OR STEAM_BUILD)
  set(LOGO_FILE "logo.png")
else()
  set(LOGO_FILE "logo_dev.png")
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
