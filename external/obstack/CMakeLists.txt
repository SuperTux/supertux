project(obstack)

add_library(obstack OBJECT ${PROJECT_SOURCE_DIR}/obstack.c)
set_target_properties(obstack PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(obstack PUBLIC ${PROJECT_SOURCE_DIR})
