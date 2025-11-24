project(FindLocale)

add_library(FindLocale OBJECT ${PROJECT_SOURCE_DIR}/findlocale.c)
set_target_properties(FindLocale PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(FindLocale PUBLIC ${PROJECT_SOURCE_DIR})
