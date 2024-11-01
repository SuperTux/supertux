# Will eventually handle game tests here too
include(CTest)

function(make_unit_test test_name)
	cmake_parse_arguments(PARSE_ARGV 1 mtargs
		"NO_PREPEND_SRC" "" "SOURCE;EXTERNAL;LIBRARIES;INCLUDES")
	# EXTERNAL is just more readable for external source files we use
	if (NOT mtargs_NO_PREPEND_SRC)
		list(TRANSFORM mtargs_EXTERNAL PREPEND ${SUPERTUX_SOURCE_DIR}/src/)
	endif()
	add_executable(${test_name} ${mtargs_SOURCE} ${mtargs_EXTERNAL})
	target_include_directories(${test_name} PUBLIC ${SUPERTUX_SOURCE_DIR}/src ${mtargs_INCLUDES})
	if (mtargs_LIBRARIES)
		target_link_libraries(${test_name} PUBLIC ${mtargs_LIBRARIES})
	endif()
	add_test(NAME ${test_name}
		COMMAND ${test_name})
endfunction(make_unit_test)

make_unit_test(MD5Test SOURCE md5_test.cpp
	EXTERNAL addon/md5.cpp)

make_unit_test(AATriangleTest SOURCE aatriangle_test.cpp
	EXTERNAL math/aatriangle.cpp
	LIBRARIES SDL2)

make_unit_test(CollisionTest SOURCE collision_test.cpp
	EXTERNAL math/rectf.cpp
	LIBRARIES SDL2)
