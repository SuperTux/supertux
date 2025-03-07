project(SDL_SavePNG)

add_library(SDL_SavePNG OBJECT ${PROJECT_SOURCE_DIR}/savepng.c)
set_target_properties(SDL_SavePNG PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(SDL_SavePNG PUBLIC PNG SDL2)
target_include_directories(SDL_SavePNG PUBLIC ${PROJECT_SOURCE_DIR})
