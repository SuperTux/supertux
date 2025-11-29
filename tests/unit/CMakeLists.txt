# Will eventually handle game tests here too
include(CTest)

# Find GTest for tests that require it
find_package(GTest QUIET)

function(make_unit_test test_name)
  cmake_parse_arguments(PARSE_ARGV 1 mtargs
    "NO_PREPEND_SRC;USES_GTEST" "" "SOURCE;EXTERNAL;LIBRARIES;DEFINITIONS;INCLUDES")
  # EXTERNAL is just more readable for external source files we use
  if (NOT mtargs_NO_PREPEND_SRC)
    list(TRANSFORM mtargs_EXTERNAL PREPEND ${SUPERTUX_SOURCE_DIR}/src/)
  endif()
  add_executable(${test_name} ${mtargs_SOURCE} ${mtargs_EXTERNAL})
  target_compile_features(${test_name} PRIVATE cxx_std_17)
  target_include_directories(${test_name} PUBLIC ${SUPERTUX_SOURCE_DIR}/src ${CMAKE_BINARY_DIR} ${mtargs_INCLUDES})
  if (mtargs_DEFINITIONS)
    target_compile_definitions(${test_name} PUBLIC ${mtargs_DEFINITIONS})
  endif()
  if (mtargs_LIBRARIES)
    target_link_libraries(${test_name} PUBLIC ${mtargs_LIBRARIES})
  endif()
  if (mtargs_USES_GTEST AND TARGET GTest::gtest)
    target_link_libraries(${test_name} PUBLIC GTest::gtest GTest::gtest_main)
  endif()
  set(all_test_targets "${all_test_targets};${test_name}" CACHE INTERNAL "")
  add_test(NAME ${test_name}
    COMMAND ${test_name})
endfunction(make_unit_test)

# Tests without GTest dependency
make_unit_test(MD5Test SOURCE md5_test.cpp
  EXTERNAL addon/md5.cpp)

make_unit_test(AATriangleTest SOURCE aatriangle_test.cpp
  EXTERNAL math/aatriangle.cpp
  LIBRARIES SDL2 glm DEFINITIONS GLM_ENABLE_EXPERIMENTAL)

make_unit_test(CollisionTest SOURCE collision_test.cpp
  EXTERNAL math/rectf.cpp
  LIBRARIES SDL2 glm DEFINITIONS GLM_ENABLE_EXPERIMENTAL)

# NOTE: DynamicScopedTest is disabled - see comment in dynamic_scoped_test.cpp

# Tests requiring GTest
if(TARGET GTest::gtest)
  make_unit_test(SizeTest USES_GTEST SOURCE size_test.cpp
    EXTERNAL math/size.cpp)

  make_unit_test(SizefTest USES_GTEST SOURCE sizef_test.cpp
    EXTERNAL math/sizef.cpp
    LIBRARIES glm DEFINITIONS GLM_ENABLE_EXPERIMENTAL)

  make_unit_test(RectTest USES_GTEST SOURCE rect_test.cpp
    EXTERNAL math/rect.cpp
    LIBRARIES SDL2 glm DEFINITIONS GLM_ENABLE_EXPERIMENTAL)

  make_unit_test(RectfTest USES_GTEST SOURCE rectf_test.cpp
    EXTERNAL math/rectf.cpp
    LIBRARIES SDL2 glm DEFINITIONS GLM_ENABLE_EXPERIMENTAL)

  make_unit_test(RandomTest USES_GTEST SOURCE random_test.cpp
    EXTERNAL math/random.cpp)

  # NOTE: UIDTest is disabled - requires log.cpp which has many dependencies
  # make_unit_test(UIDTest USES_GTEST SOURCE uid_test.cpp
  #   EXTERNAL util/uid.cpp util/uid_generator.cpp)

  make_unit_test(StringUtilTest USES_GTEST SOURCE string_util_test.cpp
    EXTERNAL util/string_util.cpp)

  make_unit_test(VersionTest USES_GTEST SOURCE version_test.cpp)
endif()

message("ALL TESTS: ${all_test_targets}")

add_custom_target(tests DEPENDS ${all_test_targets})
add_custom_command(TARGET tests POST_BUILD COMMAND ctest -C $<CONFIGURATION> --output-on-failure)
