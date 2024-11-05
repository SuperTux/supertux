## These variables enable MSVC to find libraries located in "dependencies{32|64}"

if(NOT VCPKG_BUILD)
  ## Store path of dependecy folder
  set(DEPENDENCY_FOLDER "${PROJECT_SOURCE_DIR}/dependencies")
endif()

## To test if the host (not the build) is x64:
## "$ENV{PROCESSOR_ARCHITEW6432}" STREQUAL "AMD64"
if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  set(WIN64 ON)
  if(NOT VCPKG_BUILD)
    set(DEPENDENCY_FOLDER "${DEPENDENCY_FOLDER}64")
  endif()
else()
  set(WIN64 OFF)
  if(NOT VCPKG_BUILD)
    set(DEPENDENCY_FOLDER "${DEPENDENCY_FOLDER}32")
  endif()
endif()

if(WIN64)
  set(SUPERTUX_SYSTEM_NAME win64)
else()
  set(SUPERTUX_SYSTEM_NAME win32)
endif()

if(NOT VCPKG_BUILD)
  set(ENV{PATH} "$ENV{PATH};${DEPENDENCY_FOLDER}/include")
  set(ENV{LIB} "${DEPENDENCY_FOLDER}/lib")
  set(ENV{OPENALDIR} "${DEPENDENCY_FOLDER}")
endif()

## Enable multi-processor compilation (faster)
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

## And shut up about unsafe stuff
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
## Add an icon
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/mk/msvc/icon_rc.template" "${PROJECT_BINARY_DIR}/tmp/icon.rc")

## Find all used libraries
if(NOT VCPKG_BUILD)
  file(GLOB DLLS "${DEPENDENCY_FOLDER}/dll/*")
endif()

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
