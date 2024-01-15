project(SDLSavePNG)

add_library(SDL_SavePNG INTERFACE ${PROJECT_SOURCE_DIR}/savepng.c)
target_include_directories(SDL_SavePNG INTERFACE ${PROJECT_SOURCE_DIR})
