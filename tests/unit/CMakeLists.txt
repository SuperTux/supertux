# Will eventually handle game tests here too
	
include(CTest)

# TODO make a macro/function for this
add_executable(MD5Test addon/md5.hpp)
add_test(NAME MD5Test
	COMMAND MD5Test)
