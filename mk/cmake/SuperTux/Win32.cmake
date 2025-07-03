if(WIN64)
  set(SUPERTUX_SYSTEM_NAME win64)
else()
  set(SUPERTUX_SYSTEM_NAME win32)
endif()

## Enable multi-processor compilation (faster)
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

## And shut up about unsafe stuff
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

## Add an icon
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/mk/msvc/icon_rc.template" "${PROJECT_BINARY_DIR}/tmp/icon.rc")

add_definitions(-D_USE_MATH_DEFINES -DNOMINMAX)
add_definitions(-DWIN32)

## When using MinGW, tell dlltool to create a dbghelp static library using dbghelp.def
if(MINGW)
  find_program(DLLTOOL_PATH
    NAMES dlltool dlltool.exe
    DOC "The path to the dlltool utility"
    REQUIRED
  )
  add_custom_target(DbgHelp
    COMMAND ${DLLTOOL_PATH} -k -d ${PROJECT_SOURCE_DIR}/mk/mingw/dbghelp.def -l ${PROJECT_BINARY_DIR}/dbghelp.a
    BYPRODUCTS ${PROJECT_BINARY_DIR}/dbghelp.a
  )
  add_dependencies(supertux2 DbgHelp)
  target_link_libraries(supertux2 PRIVATE ${PROJECT_BINARY_DIR}/dbghelp.a)
endif()

## On Windows, add an icon
if(WIN32 AND MINGW)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o
    COMMAND ${CMAKE_RC_COMPILER} -I${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons -i${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.rc -o ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o)
  set(SUPERTUX_SOURCES_C ${SUPERTUX_SOURCES_C} ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o)
endif()

# EOF #
