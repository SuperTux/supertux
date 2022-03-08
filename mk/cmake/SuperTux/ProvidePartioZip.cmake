find_package(ZLIB REQUIRED)

add_library(partio_zip_lib STATIC ${CMAKE_CURRENT_SOURCE_DIR}/external/partio_zip/zip_manager.cpp)
target_link_libraries(partio_zip_lib PUBLIC ${ZLIB_LIBRARIES})
target_include_directories(partio_zip_lib SYSTEM PRIVATE ${ZLIB_INCLUDE_DIRS})
target_include_directories(partio_zip_lib SYSTEM PUBLIC external/partio_zip)

# EOF #
