set(Boost_ADDITIONAL_VERSIONS "1.41" "1.41.0")

if(WIN32)
  option(ENABLE_BOOST_STATIC_LIBS "Use boost static libraries" ON)
else()
  option(ENABLE_BOOST_STATIC_LIBS "Use boost static libraries" OFF)
endif()

if(ENABLE_BOOST_STATIC_LIBS)
  set(Boost_USE_STATIC_LIBS TRUE)
else()
  set(Boost_USE_STATIC_LIBS FALSE)
endif()

find_package(Boost REQUIRED COMPONENTS filesystem system date_time locale)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

mark_as_advanced(
  Boost_INCLUDE_DIR
  Boost_LIBRARIES
  )

# EOF #
