project(FindLocale)

add_library(findlocale OBJECT ${PROJECT_SOURCE_DIR}/findlocale.c)
target_include_directories(findlocale PUBLIC ${PROJECT_SOURCE_DIR})
