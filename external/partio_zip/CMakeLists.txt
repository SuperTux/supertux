project(PartioZip)

add_library(partio_zip INTERFACE ${PROJECT_SOURCE_DIR}/zip_manager.cpp)
target_include_directories(partio_zip INTERFACE ${PROJECT_SOURCE_DIR})
