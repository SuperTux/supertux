# Discord SDK
# Before removing messages, please check with the community if no one minds that the Discord SDK gets bundled with ST.
# Some privay-concerned users or free software package managers might need an option to compile ST without the Discord integration.
option(ENABLE_DISCORD "Compile the Discord integration" OFF)

if(NOT ENABLE_DISCORD)
  message(STATUS "## WARNING : Discord integration will NOT be compiled. To enable Discord, pass -DENABLE_DISCORD=On")
else()
  message(STATUS "Discord WILL be compiled. To disable Discord, pass -DENABLE_DISCORD=Off")

  set(BUILD_EXAMPLES OFF CACHE BOOL "Skip Discord's sample programs" FORCE)

  # isolate variable changes to function scope
  function(build_discord)
    # Workaround for error in rapidjson-1.1.0/
    if(CMAKE_COMPILER_IS_GNUCXX)
      string(APPEND CMAKE_CXX_FLAGS " -Wno-error=class-memaccess ")
    endif()
    add_subdirectory(external/discord-sdk EXCLUDE_FROM_ALL)
  endfunction()
  build_discord()

  # Add missing include directories to discord-rpc
  set_target_properties(discord-rpc PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/external/discord-sdk/include"
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/external/discord-sdk/include")
endif()

# EOF #
