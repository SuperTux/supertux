find_package(PNG REQUIRED)

file(GLOB SAVEPNG_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} external/SDL_SavePNG/savepng.c)
add_library(savepng STATIC ${SAVEPNG_SOURCES_CXX})
target_include_directories(savepng SYSTEM PUBLIC ${SDL2_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS})
target_link_libraries(savepng PUBLIC ${PNG_LIBRARIES})

# EOF #
