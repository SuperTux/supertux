# Will eventually handle game tests here too
include(CTest)

find_package(GTest REQUIRED)
find_package(SDL2 REQUIRED)

function(make_unit_test test_name)
  cmake_parse_arguments(PARSE_ARGV 1 mtargs
    "NO_PREPEND_SRC;GTEST;SDL2" "" "SOURCE;EXTERNAL;LIBRARIES;DEFINITIONS;INCLUDES")
  # EXTERNAL is just more readable for external source files we use
  if (NOT mtargs_NO_PREPEND_SRC)
    list(TRANSFORM mtargs_EXTERNAL PREPEND ${SUPERTUX_SOURCE_DIR}/src/)
  endif()
  add_executable(${test_name} ${mtargs_SOURCE} ${mtargs_EXTERNAL})
  target_compile_features(${test_name} PRIVATE cxx_std_17)
  target_compile_definitions(${test_name} PUBLIC GLM_ENABLE_EXPERIMENTAL)
  target_include_directories(${test_name} PUBLIC ${SUPERTUX_SOURCE_DIR}/src ${mtargs_INCLUDES})
  if (mtargs_DEFINITIONS)
    target_compile_definitions(${test_name} PUBLIC ${mtargs_DEFINITIONS})
  endif()
  set(_libs "${mtargs_LIBRARIES}")
  if (mtargs_SDL2)
    list(APPEND _libs SDL2::SDL2)
  endif()
  if (mtargs_GTEST)
    target_link_libraries(${test_name} PUBLIC GTest::GTest GTest::Main ${_libs})
  elseif (_libs)
    target_link_libraries(${test_name} PUBLIC ${_libs})
  endif()
  set(all_test_targets "${all_test_targets};${test_name}" CACHE INTERNAL "")
  add_test(NAME ${test_name}
    COMMAND $<TARGET_FILE:${test_name}>)
endfunction(make_unit_test)

make_unit_test(MD5Test SOURCE md5_test.cpp
  EXTERNAL addon/md5.cpp)

make_unit_test(AATriangleTest SOURCE aatriangle_test.cpp
  EXTERNAL math/aatriangle.cpp
  SDL2 DEFINITIONS GLM_ENABLE_EXPERIMENTAL)

make_unit_test(CollisionTest SOURCE collision_test.cpp
  EXTERNAL math/rectf.cpp
  SDL2 DEFINITIONS GLM_ENABLE_EXPERIMENTAL)

make_unit_test(DynamicScopedTest SOURCE dynamic_scoped_test.cpp
  GTEST)

# The following tests pull in large engine subsystems (tinygettext/gettext/SDL GUI
# via util/file_system.cpp and editor/object_option.cpp). They are kept in the tree
# but not yet wired into CTest until their transitive dependencies (tinygettext,
# gettext, SDL renderer) are linked. Re-enable once those libs are available here.
# make_unit_test(FileSystemTest SOURCE file_system_test.cpp
#   GTEST LIBRARIES SDL2 EXTERNAL util/file_system.cpp)
#
# make_unit_test(IFileStreamTest SOURCE ifile_stream_test.cpp
#   GTEST EXTERNAL physfs/ifile_stream.cpp)
#
# make_unit_test(ObjectOptionTest SOURCE object_option_test.cpp
#   GTEST EXTERNAL editor/object_option.cpp supertux/game_object.cpp video/color.cpp)
#
# make_unit_test(ReaderTest SOURCE reader_test.cpp
#   GTEST EXTERNAL util/reader_document.cpp util/reader_mapping.cpp util/reader_collection.cpp util/reader_object.cpp physfs/ifile_stream.cpp util/file_system.cpp util/log.cpp)

make_unit_test(RandomTest SOURCE random_test.cpp
  GTEST EXTERNAL math/random.cpp)

make_unit_test(RectTest SOURCE rect_test.cpp
  GTEST SDL2 EXTERNAL math/rect.cpp)

make_unit_test(RectfTest SOURCE rectf_test.cpp
  GTEST SDL2 EXTERNAL math/rectf.cpp)

make_unit_test(SizeTest SOURCE size_test.cpp
  GTEST SDL2 EXTERNAL math/size.cpp)

make_unit_test(SizefTest SOURCE sizef_test.cpp
  GTEST SDL2 EXTERNAL math/sizef.cpp)

make_unit_test(StringUtilTest SOURCE string_util_test.cpp
  GTEST EXTERNAL util/string_util.cpp)

# UidTest pulls in util/log.cpp, which transitively requires the full engine
# (simplesquirrel via supertux/console.hpp). It is kept in the tree but not
# wired into CTest until the test can link against the engine libraries.
# make_unit_test(UidTest SOURCE uid_test.cpp
#   GTEST SDL2 EXTERNAL util/uid_generator.cpp util/log.cpp)

make_unit_test(VersionTest SOURCE version_test.cpp
  GTEST INCLUDES ${CMAKE_BINARY_DIR})

message("ALL TESTS: ${all_test_targets}")

add_custom_target(tests DEPENDS ${all_test_targets})
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  DEPENDS ${all_test_targets}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
