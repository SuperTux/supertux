option(BUILD_UNIT_TESTS "Built unit tests" OFF)
if(BUILD_UNIT_TESTS)
	add_subdirectory(unit)
endif()
