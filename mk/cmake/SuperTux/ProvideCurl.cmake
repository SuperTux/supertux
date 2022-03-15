## The curl headers are in the standard include path in dependencies
## so it's not required to add them here

if(NOT EMSCRIPTEN)
  find_package(CURL REQUIRED)

  if(WIN32 AND VCPKG_BUILD)
    list(LENGTH CURL_LIBRARY CURL_LIBRARY_LENGTH)
    if(CURL_LIBRARY_LENGTH EQUAL 1)
      set(CURL_LIBRARY_DEBUG_LIB   ${CURL_LIBRARY})
      set(CURL_LIBRARY_RELEASE_LIB ${CURL_LIBRARY_DEBUG_LIB}/../../../lib/libcurl.lib)
      get_filename_component(CURL_LIBRARY_RELEASE_LIB ${CURL_LIBRARY_RELEASE_LIB} REALPATH)
      unset(CURL_LIBRARY CACHE)
      unset(CURL_LIBRARY)
      unset(CURL_LIBRARIES  CACHE)
      unset(CURL_LIBRARIES)
      set(CURL_LIBRARY "debug;${CURL_LIBRARY_DEBUG_LIB};optimized;${CURL_LIBRARY_RELEASE_LIB}")
      set(CURL_LIBRARIES ${CURL_LIBRARY})
    endif()
  endif()

  add_library(LibCurl INTERFACE IMPORTED)
  set_target_properties(LibCurl PROPERTIES
    INTERFACE_LINK_LIBRARIES "${CURL_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}")

  set(HAVE_LIBCURL TRUE)
endif()

# EOF #
