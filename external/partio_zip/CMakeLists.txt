project(PartioZip)

add_library(PartioZip OBJECT ${PROJECT_SOURCE_DIR}/zip_manager.cpp ${PROJECT_SOURCE_DIR}/zip_manager.hpp)
target_link_libraries(PartioZip PUBLIC ZLIB)
target_include_directories(PartioZip PUBLIC ${PROJECT_SOURCE_DIR})
