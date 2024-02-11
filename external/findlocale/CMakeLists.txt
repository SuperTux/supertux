project(FindLocale)

add_library(findlocale INTERFACE ${PROJECT_SOURCE_DIR}/findlocale.c)
target_include_directories(findlocale INTERFACE ${PROJECT_SOURCE_DIR})
