find_package(glm QUIET)

if(glm_FOUND)
  if(TARGET glm::glm)
    message(STATUS "Found glm")
    add_library(LibGlm ALIAS glm::glm)
  else()
    # fallback for old glm version in EmuELEC and Nix
    message(STATUS "Found glm: ${GLM_INCLUDE_DIR}")
    add_library(LibGlm INTERFACE IMPORTED)
    set_target_properties(LibGlm PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "$<TARGET_PROPERTY:glm,INTERFACE_INCLUDE_DIRECTORIES>")
  endif()
else()
  # fallback for old glm version in UBPorts
  find_path(GLM_INCLUDE_DIR
    NAMES glm/glm.hpp
    PATHS ${GLM_ROOT_DIR}/include)
  if(NOT GLM_INCLUDE_DIR)
    message(FATAL_ERROR "glm library missing")
  endif()

  message(STATUS "Found glm: ${GLM_INCLUDE_DIR}")
  add_library(LibGlm INTERFACE IMPORTED)
  set_target_properties(LibGlm PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GLM_INCLUDE_DIR})
endif()

# EOF #
