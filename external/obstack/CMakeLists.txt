project(obstack)

add_library(obstack OBJECT ${PROJECT_SOURCE_DIR}/obstack.c)
target_include_directories(obstack PUBLIC ${PROJECT_SOURCE_DIR})
