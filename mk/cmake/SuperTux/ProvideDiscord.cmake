# Discord SDK
# Before removing messages, please check with the community if no one minds that the Discord SDK gets bundled with ST.
# Some privay-concerned users or free software package managers might need an option to compile ST without the Discord integration.
option(ENABLE_DISCORD "Compile the Discord integration" OFF)

if(ENABLE_DISCORD)
  message(STATUS "Discord WILL be compiled. To disable Discord, pass -DENABLE_DISCORD=Off")
  include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/external/discord-sdk/include)
  set(BUILD_EXAMPLES OFF CACHE BOOL "Skip Discord's sample programs" FORCE)
  add_subdirectory(external/discord-sdk)
else()
  message(STATUS "## WARNING : Discord integration will NOT be compiled. To enable Discord, pass -DENABLE_DISCORD=On")
endif()

# EOF #
