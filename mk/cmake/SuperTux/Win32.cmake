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

## When using MinGW, tell dlltool to generate a dbghelp static import library using dbghelp.def
find_program(DLLTOOL_PATH
  NAMES dlltool dlltool.exe
  DOC "The path to the dlltool utility"
)
if(MINGW AND DLLTOOL_PATH)
  add_custom_target(dbghelp
    COMMAND ${CMAKE_COMMAND}
            -DDLLTOOL_PATH="${DLLTOOL_PATH}"
            -DDEFINITIONS="${PROJECT_SOURCE_DIR}/mk/mingw/dbghelp.def"
            -DOUTPUT="${PROJECT_BINARY_DIR}/dbghelp.dll.a"
            -P ${PROJECT_SOURCE_DIR}/mk/cmake/GenerateMinGWDbgHelp.cmake
    BYPRODUCTS ${PROJECT_BINARY_DIR}/dbghelp.dll.a
    COMMENT "Prompting generation of dbghelp.dll.a static import library"
  )

  target_link_libraries(supertux2 PRIVATE ${PROJECT_BINARY_DIR}/dbghelp.dll.a)
  add_dependencies(supertux2 dbghelp)
endif()

## On Windows, add an icon
if(WIN32 AND MINGW)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o
    COMMAND ${CMAKE_RC_COMPILER} -I${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons -i${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.rc -o ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o)
  set(SUPERTUX_SOURCES_C ${SUPERTUX_SOURCES_C} ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o)
endif()

# EOF #
