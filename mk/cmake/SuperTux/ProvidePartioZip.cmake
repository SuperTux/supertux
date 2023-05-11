find_package(ZLIB REQUIRED)

add_library(LibPartioZip ${PROJECT_SOURCE_DIR}/external/partio_zip/zip_manager.cpp)
target_link_libraries(LibPartioZip PUBLIC ${ZLIB_LIBRARIES})
target_include_directories(LibPartioZip SYSTEM PRIVATE ${ZLIB_INCLUDE_DIRS})
target_include_directories(LibPartioZip SYSTEM PUBLIC external/partio_zip)

# EOF #
