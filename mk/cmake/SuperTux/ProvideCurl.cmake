## The curl headers are in the standard include path in dependencies
## so it's not required to add them here
option(HAVE_LIBCURL "Use libcurl" ON)

if(EMSCRIPTEN)
  set(HAVE_LIBCURL OFF)
elseif(HAVE_LIBCURL)
  if(ANDROID)
    find_library(CURL libcurl)
  else()
    find_package(CURL REQUIRED)
  endif()

  add_library(LibCurl INTERFACE IMPORTED)
  set_target_properties(LibCurl PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}")
  if(WIN32)
    target_link_libraries(LibCurl INTERFACE ${CURL_LIBRARY})
  else()
    set_target_properties(LibCurl PROPERTIES
      INTERFACE_LINK_LIBRARIES "${CURL_LIBRARY}")
  endif()
endif()

# EOF #
