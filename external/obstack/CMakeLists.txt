project(obstack)

add_library(obstack INTERFACE ${PROJECT_SOURCE_DIR}/obstack.c)
target_include_directories(obstack INTERFACE ${PROJECT_SOURCE_DIR})
