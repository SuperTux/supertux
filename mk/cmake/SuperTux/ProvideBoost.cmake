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

if(EMSCRIPTEN)
  add_library(LibBoost ALIAS Boost::boost)
else()
  add_library(LibBoost INTERFACE IMPORTED)
endif()

if(WIN32)
  # Boost_LIBRARIES may contain link-type keywords "optimized,debug"
  # that aren't understood by INTERFACE_LINK_DIRECTORIES directly,
  # meanwhile older cmake on other OSs don't support
  # target_link_libraries(INTERFACE).
  target_link_libraries(LibBoost INTERFACE ${Boost_LIBRARIES})
else()
  set_target_properties(LibBoost PROPERTIES
    INTERFACE_LINK_LIBRARIES "${Boost_LIBRARIES}")
endif()

set_target_properties(LibBoost PROPERTIES
  INTERFACE_LINK_DIRECTORIES "${Boost_LIBRARY_DIRS}"
  INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIR}"
  )

mark_as_advanced(
  Boost_INCLUDE_DIR
  Boost_LIBRARIES
  )

# EOF #
