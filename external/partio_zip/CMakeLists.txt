project(PartioZip)

add_library(partio_zip OBJECT ${PROJECT_SOURCE_DIR}/zip_manager.cpp ${PROJECT_SOURCE_DIR}/zip_manager.hpp)
target_link_libraries(partio_zip PUBLIC ${ZLIB_LIBRARIES})
target_include_directories(partio_zip PUBLIC ${PROJECT_SOURCE_DIR} ${ZLIB_INCLUDE_DIRS})
