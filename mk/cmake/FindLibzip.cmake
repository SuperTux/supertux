include(FindPkgConfig)

pkg_check_modules(PC_LIBZIP libzip)

find_path(LIBZIP_INCLUDE_DIRS NAMES zip.h HINTS ${PC_LIBZIP_INCLUDE_DIRS}
  ${PC_LIBZIP_INCLUDEDIR})

find_library(LIBZIP_LIBRARIES NAMES zip HINTS ${PC_LIBZIP_LIBRARY_DIRS}
  ${PC_LIBZIP_LIBDIR})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(LIBZIP DEFAULT_MSG LIBZIP_INCLUDE_DIRS
  LIBZIP_LIBRARIES)
