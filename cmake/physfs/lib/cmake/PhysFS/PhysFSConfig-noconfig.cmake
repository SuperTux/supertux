#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "physfs-static" for configuration ""
set_property(TARGET physfs-static APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(physfs-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "C"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libphysfs.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS physfs-static )
list(APPEND _IMPORT_CHECK_FILES_FOR_physfs-static "${_IMPORT_PREFIX}/lib/libphysfs.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
