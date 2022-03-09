# Discord SDK
# Before removing messages, please check with the community if no one minds that the Discord SDK gets bundled with ST.
# Some privay-concerned users or free software package managers might need an option to compile ST without the Discord integration.
option(ENABLE_DISCORD "Compile the Discord integration" OFF)

if(NOT ENABLE_DISCORD)
  message(STATUS "## WARNING : Discord integration will NOT be compiled. To enable Discord, pass -DENABLE_DISCORD=On")
else()
  message(STATUS "Discord WILL be compiled. To disable Discord, pass -DENABLE_DISCORD=Off")

  set(BUILD_EXAMPLES OFF CACHE BOOL "Skip Discord's sample programs" FORCE)

  add_subdirectory(external/discord-sdk EXCLUDE_FROM_ALL)

  # Add missing include directories to discord-rpc
  set_target_properties(discord-rpc PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/external/discord-sdk/include"
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/external/discord-sdk/include")
endif()

# EOF #
