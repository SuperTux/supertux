# PhysicsFS; a portable, flexible file i/o abstraction.
#
# Please see the file LICENSE.txt in the source's root directory.

# The CMake project file is meant to get this compiling on all sorts of
#  platforms quickly, and serve as the way Unix platforms and Linux distros
#  package up official builds, but you don't _need_ to use this; we have
#  built PhysicsFS to (hopefully) be able to drop into your project and
#  compile, using preprocessor checks for platform-specific bits instead of
#  testing in here.

cmake_minimum_required(VERSION 2.8.4)

project(PhysicsFS)
set(PHYSFS_VERSION 3.1.0)

# Increment this if/when we break backwards compatibility.
set(PHYSFS_SOVERSION 1)

# I hate that they define "WIN32" ... we're about to move to Win64...I hope!
if(WIN32 AND NOT WINDOWS)
    set(WINDOWS TRUE)
endif()

include_directories(./src)

if(APPLE)
    set(OTHER_LDFLAGS ${OTHER_LDFLAGS} "-framework IOKit -framework Foundation")
    set(PHYSFS_M_SRCS src/physfs_platform_apple.m)
endif()

if(CMAKE_COMPILER_IS_GNUCC)
    # Don't use -rpath.
    set(CMAKE_SKIP_RPATH ON CACHE BOOL "Skip RPATH" FORCE)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "SunPro")
    add_definitions(-erroff=E_EMPTY_TRANSLATION_UNIT)
    add_definitions(-xldscope=hidden)
endif()

if(HAIKU)
    # We add this explicitly, since we don't want CMake to think this
    #  is a C++ project unless we're on Haiku.
    set(PHYSFS_CPP_SRCS src/physfs_platform_haiku.cpp)
    find_library(BE_LIBRARY be)
    find_library(ROOT_LIBRARY root)
    set(OPTIONAL_LIBRARY_LIBS ${OPTIONAL_LIBRARY_LIBS} ${BE_LIBRARY} ${ROOT_LIBRARY})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "WindowsPhone" OR CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
    set(WINRT TRUE)
endif()

if(WINRT)
    set(PHYSFS_CPP_SRCS src/physfs_platform_winrt.cpp)
endif()

if(UNIX AND NOT WINDOWS AND NOT APPLE)  # (MingW and such might be UNIX _and_ WINDOWS!)
    find_library(PTHREAD_LIBRARY pthread)
    if(PTHREAD_LIBRARY)
        set(OPTIONAL_LIBRARY_LIBS ${OPTIONAL_LIBRARY_LIBS} ${PTHREAD_LIBRARY})
    endif()
endif()

# Almost everything is "compiled" here, but things that don't apply to the
#  build are #ifdef'd out. This is to make it easy to embed PhysicsFS into
#  another project or bring up a new build system: just compile all the source
#  code and #define the things you want.
set(PHYSFS_SRCS
    src/physfs.c
    src/physfs_byteorder.c
    src/physfs_unicode.c
    src/physfs_platform_posix.c
    src/physfs_platform_unix.c
    src/physfs_platform_windows.c
    src/physfs_platform_os2.c
    src/physfs_platform_qnx.c
    src/physfs_archiver_dir.c
    src/physfs_archiver_unpacked.c
    src/physfs_archiver_grp.c
    src/physfs_archiver_hog.c
    src/physfs_archiver_7z.c
    src/physfs_archiver_mvl.c
    src/physfs_archiver_qpak.c
    src/physfs_archiver_wad.c
    src/physfs_archiver_zip.c
    src/physfs_archiver_slb.c
    src/physfs_archiver_iso9660.c
    src/physfs_archiver_vdf.c
    ${PHYSFS_CPP_SRCS}
    ${PHYSFS_M_SRCS}
)


# Archivers ...
# These are (mostly) on by default now, so these options are only useful for
#  disabling them.

option(PHYSFS_ARCHIVE_ZIP "Enable ZIP support" TRUE)
if(NOT PHYSFS_ARCHIVE_ZIP)
    add_definitions(-DPHYSFS_SUPPORTS_ZIP=0)
endif()

option(PHYSFS_ARCHIVE_7Z "Enable 7zip support" TRUE)
if(NOT PHYSFS_ARCHIVE_7Z)
    add_definitions(-DPHYSFS_SUPPORTS_7Z=0)
endif()

option(PHYSFS_ARCHIVE_GRP "Enable Build Engine GRP support" TRUE)
if(NOT PHYSFS_ARCHIVE_GRP)
    add_definitions(-DPHYSFS_SUPPORTS_GRP=0)
endif()

option(PHYSFS_ARCHIVE_WAD "Enable Doom WAD support" TRUE)
if(NOT PHYSFS_ARCHIVE_WAD)
    add_definitions(-DPHYSFS_SUPPORTS_WAD=0)
endif()

option(PHYSFS_ARCHIVE_HOG "Enable Descent I/II HOG support" TRUE)
if(NOT PHYSFS_ARCHIVE_HOG)
    add_definitions(-DPHYSFS_SUPPORTS_HOG=0)
endif()

option(PHYSFS_ARCHIVE_MVL "Enable Descent I/II MVL support" TRUE)
if(NOT PHYSFS_ARCHIVE_MVL)
    add_definitions(-DPHYSFS_SUPPORTS_MVL=0)
endif()

option(PHYSFS_ARCHIVE_QPAK "Enable Quake I/II QPAK support" TRUE)
if(NOT PHYSFS_ARCHIVE_QPAK)
    add_definitions(-DPHYSFS_SUPPORTS_QPAK=0)
endif()

option(PHYSFS_ARCHIVE_SLB "Enable I-War / Independence War SLB support" TRUE)
if(NOT PHYSFS_ARCHIVE_SLB)
    add_definitions(-DPHYSFS_SUPPORTS_SLB=0)
endif()

option(PHYSFS_ARCHIVE_ISO9660 "Enable ISO9660 support" TRUE)
if(NOT PHYSFS_ARCHIVE_ISO9660)
    add_definitions(-DPHYSFS_SUPPORTS_ISO9660=0)
endif()

option(PHYSFS_ARCHIVE_VDF "Enable Gothic I/II VDF archive support" TRUE)
if(NOT PHYSFS_ARCHIVE_VDF)
    add_definitions(-DPHYSFS_SUPPORTS_VDF=0)
endif()


option(PHYSFS_BUILD_STATIC "Build static library" TRUE)
if(PHYSFS_BUILD_STATIC)
    add_library(physfs-static STATIC ${PHYSFS_SRCS})
    # Don't rename this on Windows, since DLLs will also produce an import
    #  library named "physfs.lib" which would conflict; Unix tend to like the
    #  same library name with a different extension for static libs, but
    #  Windows can just have a separate name.
    if(NOT MSVC)
        set_target_properties(physfs-static PROPERTIES OUTPUT_NAME "physfs")
    endif()
    if(WINRT)
        # Ignore LNK4264 warnings; we don't author any WinRT components, just consume them, so we're okay in a static library.
		set_target_properties(physfs-static PROPERTIES VS_WINRT_COMPONENT True)
        set_target_properties(physfs-static PROPERTIES STATIC_LIBRARY_FLAGS "/ignore:4264")
    endif()

    set(PHYSFS_LIB_TARGET physfs-static)
    set(PHYSFS_INSTALL_TARGETS ${PHYSFS_INSTALL_TARGETS} ";physfs-static")
endif()

option(PHYSFS_BUILD_SHARED "Build shared library" TRUE)
if(PHYSFS_BUILD_SHARED)
    add_library(physfs SHARED ${PHYSFS_SRCS})
    set_target_properties(physfs PROPERTIES MACOSX_RPATH 1)
    set_target_properties(physfs PROPERTIES VERSION ${PHYSFS_VERSION})
    set_target_properties(physfs PROPERTIES SOVERSION ${PHYSFS_SOVERSION})
    if(WINRT)
		set_target_properties(physfs PROPERTIES VS_WINRT_COMPONENT True)
    endif()
    target_link_libraries(physfs ${OPTIONAL_LIBRARY_LIBS} ${OTHER_LDFLAGS})
    set(PHYSFS_LIB_TARGET physfs)
    set(PHYSFS_INSTALL_TARGETS ${PHYSFS_INSTALL_TARGETS} ";physfs")
endif()

if(NOT PHYSFS_BUILD_SHARED AND NOT PHYSFS_BUILD_STATIC)
    message(FATAL "Both shared and static libraries are disabled!")
endif()

# CMake FAQ says I need this...
if(PHYSFS_BUILD_SHARED AND PHYSFS_BUILD_STATIC AND NOT WINDOWS)
    set_target_properties(physfs PROPERTIES CLEAN_DIRECT_OUTPUT 1)
    set_target_properties(physfs-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
endif()

option(PHYSFS_BUILD_TEST "Build stdio test program." TRUE)
mark_as_advanced(PHYSFS_BUILD_TEST)
if(PHYSFS_BUILD_TEST)
    find_path(READLINE_H readline/readline.h)
    find_path(HISTORY_H readline/history.h)
    if(READLINE_H AND HISTORY_H)
        find_library(CURSES_LIBRARY NAMES curses ncurses)
        set(CMAKE_REQUIRED_LIBRARIES ${CURSES_LIBRARY})
        find_library(READLINE_LIBRARY readline)
        if(READLINE_LIBRARY)
            set(HAVE_SYSTEM_READLINE TRUE)
            set(TEST_PHYSFS_LIBS ${TEST_PHYSFS_LIBS} ${READLINE_LIBRARY} ${CURSES_LIBRARY})
            include_directories(SYSTEM ${READLINE_H} ${HISTORY_H})
            add_definitions(-DPHYSFS_HAVE_READLINE=1)
        endif()
    endif()
    add_executable(test_physfs test/test_physfs.c)
    target_link_libraries(test_physfs ${PHYSFS_LIB_TARGET} ${TEST_PHYSFS_LIBS} ${OTHER_LDFLAGS})
    set(PHYSFS_INSTALL_TARGETS ${PHYSFS_INSTALL_TARGETS} ";test_physfs")
endif()

install(TARGETS ${PHYSFS_INSTALL_TARGETS}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib${LIB_SUFFIX}
        ARCHIVE DESTINATION lib${LIB_SUFFIX})
install(FILES src/physfs.h DESTINATION include)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(PHYSFS_OUTPUT_DOXYFILE "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile"
        "${PHYSFS_OUTPUT_DOXYFILE}"
        COPYONLY
    )
    file(APPEND "${PHYSFS_OUTPUT_DOXYFILE}" "\n\n# Below auto-generated by cmake...\n\n")
    file(APPEND "${PHYSFS_OUTPUT_DOXYFILE}" "PROJECT_NUMBER = \"${PHYSFS_VERSION}\"\n")
    file(APPEND "${PHYSFS_OUTPUT_DOXYFILE}" "OUTPUT_DIRECTORY = \"${CMAKE_CURRENT_BINARY_DIR}/docs\"\n")
    file(APPEND "${PHYSFS_OUTPUT_DOXYFILE}" "\n# End auto-generated section.\n\n")

    add_custom_target(
        docs
        ${DOXYGEN_EXECUTABLE} "${PHYSFS_OUTPUT_DOXYFILE}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Building documentation in 'docs' directory..."
    )
else()
    message(STATUS "Doxygen not found. You won't be able to build documentation.")
endif()

if(UNIX AND NOT ANDROID)
    set(PHYSFS_TARBALL "${CMAKE_CURRENT_SOURCE_DIR}/../physfs-${PHYSFS_VERSION}.tar.bz2")
    add_custom_target(
        dist
        hg archive -t tbz2 "${PHYSFS_TARBALL}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Building source tarball '${PHYSFS_TARBALL}'..."
    )
    add_custom_target(
        uninstall
        "${CMAKE_CURRENT_SOURCE_DIR}/extras/uninstall.sh"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Uninstall the project..."
    )
endif()

if(NOT MSVC)
    configure_file(
        "extras/physfs.pc.in"
        "extras/physfs.pc"
        @ONLY
    )
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/extras/physfs.pc"
        DESTINATION "lib${LIB_SUFFIX}/pkgconfig"
    )
endif()

macro(message_bool_option _NAME _VALUE)
    if(${_VALUE})
        message(STATUS "  ${_NAME}: enabled")
    else()
        message(STATUS "  ${_NAME}: disabled")
    endif()
endmacro()

message(STATUS "PhysicsFS will build with the following options:")
message_bool_option("ZIP support" PHYSFS_ARCHIVE_ZIP)
message_bool_option("7zip support" PHYSFS_ARCHIVE_7Z)
message_bool_option("GRP support" PHYSFS_ARCHIVE_GRP)
message_bool_option("WAD support" PHYSFS_ARCHIVE_WAD)
message_bool_option("HOG support" PHYSFS_ARCHIVE_HOG)
message_bool_option("MVL support" PHYSFS_ARCHIVE_MVL)
message_bool_option("QPAK support" PHYSFS_ARCHIVE_QPAK)
message_bool_option("SLB support" PHYSFS_ARCHIVE_SLB)
message_bool_option("VDF support" PHYSFS_ARCHIVE_VDF)
message_bool_option("ISO9660 support" PHYSFS_ARCHIVE_ISO9660)
message_bool_option("Build static library" PHYSFS_BUILD_STATIC)
message_bool_option("Build shared library" PHYSFS_BUILD_SHARED)
message_bool_option("Build stdio test program" PHYSFS_BUILD_TEST)
if(PHYSFS_BUILD_TEST)
    message_bool_option("  Use readline in test program" HAVE_SYSTEM_READLINE)
endif()

# end of CMakeLists.txt ...

