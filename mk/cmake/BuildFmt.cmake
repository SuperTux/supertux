option(USE_SYSTEM_FMT "Use preinstalled fmt if available, must be 8.0.0 or newer" ON)
if(USE_SYSTEM_FMT)
  find_package(fmt 8.0.0 QUIET)
endif()

if(TARGET fmt::fmt)
  message(STATUS "Found fmt")
else()
  if(USE_SYSTEM_FMT)
    message(STATUS "Could NOT find fmt, using external/fmt fallback")
  endif()

  add_subdirectory(external/fmt EXCLUDE_FROM_ALL)
endif()

# EOF #
