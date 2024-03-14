project(SDLSavePNG)

add_library(SDL_SavePNG OBJECT ${PROJECT_SOURCE_DIR}/savepng.c)
target_link_libraries(SDL_SavePNG PUBLIC ${SDL2_LIBRARY} ${PNG_LIBRARY})
target_include_directories(SDL_SavePNG PUBLIC ${PROJECT_SOURCE_DIR} ${SDL2_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS})
