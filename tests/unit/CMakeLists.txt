# Will eventually handle game tests here too
include(CTest)

# TODO make a macro/function for this
add_executable(MD5Test md5_test.cpp ${SUPERTUX_SOURCE_DIR}/src/addon/md5.cpp)
target_include_directories(MD5Test PUBLIC ${SUPERTUX_SOURCE_DIR}/src)
add_test(NAME MD5Test
	COMMAND MD5Test)

#add_executable(AATriangleTest aatriangle_test.cpp)
#target_include_directories(AATriangleTest PUBLIC ${SUPERTUX_SOURCE_DIR}/src)
#add_test(NAME AATriangleTest
#	COMMAND AATriangleTest)
