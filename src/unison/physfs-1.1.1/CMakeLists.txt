# PhysicsFS; a portable, flexible file i/o abstraction.
# Copyright (C) 2007  Ryan C. Gordon.
#
# Please see the file LICENSE.txt in the source's root directory.

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

PROJECT(PhysicsFS)
SET(PHYSFS_VERSION 1.1.1)
SET(PHYSFS_SOVERSION 1)

# I hate that they define "WIN32" ... we're about to move to Win64...I hope!
IF(WIN32 AND NOT WINDOWS)
    SET(WINDOWS TRUE)
ENDIF(WIN32 AND NOT WINDOWS)

# Bleh, let's do it for "APPLE" too.
IF(APPLE AND NOT MACOSX)
    SET(MACOSX TRUE)
ENDIF(APPLE AND NOT MACOSX)

INCLUDE(CheckIncludeFile)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckCSourceCompiles)

INCLUDE_DIRECTORIES(.)
#INCLUDE_DIRECTORIES(platform)
#INCLUDE_DIRECTORIES(archivers)

IF(MACOSX)
    # Fallback to older OS X on PowerPC to support wider range of systems...
    IF(CMAKE_OSX_ARCHITECTURES MATCHES ppc)
        ADD_DEFINITIONS(-DMAC_OS_X_VERSION_MIN_REQUIRED=1020)
        SET(OTHER_LDFLAGS ${OTHER_LDFLAGS} " -mmacosx-version-min=10.2")
    ENDIF(CMAKE_OSX_ARCHITECTURES MATCHES ppc)

    # Need these everywhere...
    ADD_DEFINITIONS(-fno-common)
    SET(OTHER_LDFLAGS ${OTHER_LDFLAGS} " -framework Carbon -framework IOKit")
ENDIF(MACOSX)

# Add some gcc-specific command lines.
IF(CMAKE_COMPILER_IS_GNUCC)
    # Always build with debug symbols...you can strip it later.
    ADD_DEFINITIONS(-g -pipe -Werror -fsigned-char)

    # Stupid BeOS generates warnings in the system headers.
    IF(NOT BEOS)
        ADD_DEFINITIONS(-Wall)
    ENDIF(NOT BEOS)

    CHECK_C_SOURCE_COMPILES("
        #if ((defined(__GNUC__)) && (__GNUC__ >= 4))
        int main(int argc, char **argv) { int is_gcc4 = 1; return 0; }
        #else
        #error This is not gcc4.
        #endif
    " PHYSFS_IS_GCC4)

    IF(PHYSFS_IS_GCC4)
        ADD_DEFINITIONS(-fvisibility=hidden)
    ENDIF(PHYSFS_IS_GCC4)
ENDIF(CMAKE_COMPILER_IS_GNUCC)

IF(MSVC)
    # VS.NET 8.0 got really really anal about strcpy, etc, which even if we
    #  cleaned up our code, zlib, etc still use...so disable the warning.
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS=1)
ENDIF(MSVC)

# Basic chunks of source code ...

SET(ZLIB_SRCS
    zlib123/adler32.c
    zlib123/compress.c
    zlib123/crc32.c
    zlib123/deflate.c
    zlib123/gzio.c
    zlib123/infback.c
    zlib123/inffast.c
    zlib123/inflate.c
    zlib123/inftrees.c
    zlib123/trees.c
    zlib123/uncompr.c
    zlib123/zutil.c
)

SET(LZMA_SRCS
    lzma/7zBuffer.c
    lzma/7zCrc.c
    lzma/7zDecode.c
    lzma/7zExtract.c
    lzma/7zHeader.c
    lzma/7zIn.c
    lzma/7zItem.c
    lzma/7zMethodID.c
    lzma/LzmaDecode.c
    lzma/LzmaStateDecode.c
)

IF(BEOS)
    # We add this explicitly, since we don't want CMake to think this
    #  is a C++ project unless we're on BeOS.
    SET(PHYSFS_BEOS_SRCS platform/beos.cpp)
    SET(OPTIONAL_LIBRARY_LIBS ${OPTIONAL_LIBRARY_LIBS} be root)
ENDIF(BEOS)

# Almost everything is "compiled" here, but things that don't apply to the
#  build are #ifdef'd out. This is to make it easy to embed PhysicsFS into
#  another project or bring up a new build system: just compile all the source
#  code and #define the things you want.
SET(PHYSFS_SRCS
    physfs.c
    physfs_byteorder.c
    physfs_unicode.c
    platform/os2.c
    platform/pocketpc.c
    platform/posix.c
    platform/unix.c
    platform/macosx.c
    platform/windows.c
    archivers/dir.c
    archivers/grp.c
    archivers/hog.c
    archivers/lzma.c
    archivers/mvl.c
    archivers/qpak.c
    archivers/wad.c
    archivers/zip.c
    ${PHYSFS_BEOS_SRCS}
)


# platform layers ...

IF(UNIX)
    IF(BEOS)
        SET(PHYSFS_HAVE_CDROM_SUPPORT TRUE)
        SET(PHYSFS_HAVE_THREAD_SUPPORT TRUE)
        SET(HAVE_PTHREAD_H TRUE)
    ELSE(BEOS)
        # !!! FIXME
        #  AC_DEFINE([PHYSFS_HAVE_LLSEEK], 1, [define if we have llseek])
        CHECK_INCLUDE_FILE(sys/ucred.h HAVE_UCRED_H)
        IF(HAVE_UCRED_H)
            ADD_DEFINITIONS(-DPHYSFS_HAVE_SYS_UCRED_H=1)
            SET(PHYSFS_HAVE_CDROM_SUPPORT TRUE)
        ENDIF(HAVE_UCRED_H)

        CHECK_INCLUDE_FILE(mntent.h HAVE_MNTENT_H)
        IF(HAVE_MNTENT_H)
            ADD_DEFINITIONS(-DPHYSFS_HAVE_MNTENT_H=1)
            SET(PHYSFS_HAVE_CDROM_SUPPORT TRUE)
        ENDIF(HAVE_MNTENT_H)

        CHECK_INCLUDE_FILE(pthread.h HAVE_PTHREAD_H)
        IF(HAVE_PTHREAD_H)
            SET(PHYSFS_HAVE_THREAD_SUPPORT TRUE)
        ELSE(HAVE_PTHREAD_H)
            ADD_DEFINITIONS(-DPHYSFS_NO_PTHREADS_SUPPORT=1)
        ENDIF(HAVE_PTHREAD_H)
    ENDIF(BEOS)
ENDIF(UNIX)

IF(WINDOWS)
    SET(PHYSFS_HAVE_CDROM_SUPPORT TRUE)
    SET(PHYSFS_HAVE_THREAD_SUPPORT TRUE)
ENDIF(WINDOWS)

IF(NOT PHYSFS_HAVE_CDROM_SUPPORT)
    ADD_DEFINITIONS(-DPHYSFS_NO_CDROM_SUPPORT=1)
    MESSAGE(WARNING " ***")
    MESSAGE(WARNING " *** There is no CD-ROM support in this build!")
    MESSAGE(WARNING " *** PhysicsFS will just pretend there are no discs.")
    MESSAGE(WARNING " *** This may be fine, depending on how PhysicsFS is used,")
    MESSAGE(WARNING " ***   but is this what you REALLY wanted?")
    MESSAGE(WARNING " *** (Maybe fix CMakeLists.txt, or write a platform driver?)")
    MESSAGE(WARNING " ***")
ENDIF(NOT PHYSFS_HAVE_CDROM_SUPPORT)

IF(PHYSFS_HAVE_THREAD_SUPPORT)
    ADD_DEFINITIONS(-D_REENTRANT -D_THREAD_SAFE)
ELSE(PHYSFS_HAVE_THREAD_SUPPORT)
    MESSAGE(WARNING " ***")
    MESSAGE(WARNING " *** There is no thread support in this build!")
    MESSAGE(WARNING " *** PhysicsFS will NOT be reentrant!")
    MESSAGE(WARNING " *** This may be fine, depending on how PhysicsFS is used,")
    MESSAGE(WARNING " ***   but is this what you REALLY wanted?")
    MESSAGE(WARNING " *** (Maybe fix CMakeLists.txt, or write a platform driver?)")
    MESSAGE(WARNING " ***")
ENDIF(PHYSFS_HAVE_THREAD_SUPPORT)

CHECK_INCLUDE_FILE(assert.h HAVE_ASSERT_H)
IF(HAVE_ASSERT_H)
    ADD_DEFINITIONS(-DHAVE_ASSERT_H=1)
ENDIF(HAVE_ASSERT_H)



# Archivers ...

OPTION(PHYSFS_ARCHIVE_ZIP "Enable ZIP support" TRUE)
IF(PHYSFS_ARCHIVE_ZIP)
    ADD_DEFINITIONS(-DPHYSFS_SUPPORTS_ZIP=1)
    SET(PHYSFS_NEED_ZLIB TRUE)
ENDIF(PHYSFS_ARCHIVE_ZIP)

OPTION(PHYSFS_ARCHIVE_7Z "Enable 7zip support" TRUE)
IF(PHYSFS_ARCHIVE_7Z)
    ADD_DEFINITIONS(-DPHYSFS_SUPPORTS_7Z=1)
    # !!! FIXME: rename to 7z.c?
    SET(PHYSFS_SRCS ${PHYSFS_SRCS} ${LZMA_SRCS})
    INCLUDE_DIRECTORIES(lzma)
    ADD_DEFINITIONS(-D_LZMA_IN_CB=1)
    ADD_DEFINITIONS(-D_LZMA_PROB32=1)
    ADD_DEFINITIONS(-D_LZMA_SYSTEM_SIZE_T=1)
    ADD_DEFINITIONS(-D_SZ_ONE_DIRECTORY=1)
ENDIF(PHYSFS_ARCHIVE_7Z)

OPTION(PHYSFS_ARCHIVE_GRP "Enable Build Engine GRP support" TRUE)
IF(PHYSFS_ARCHIVE_GRP)
    ADD_DEFINITIONS(-DPHYSFS_SUPPORTS_GRP=1)
ENDIF(PHYSFS_ARCHIVE_GRP)

OPTION(PHYSFS_ARCHIVE_WAD "Enable Doom WAD support" TRUE)
IF(PHYSFS_ARCHIVE_WAD)
    ADD_DEFINITIONS(-DPHYSFS_SUPPORTS_WAD=1)
ENDIF(PHYSFS_ARCHIVE_WAD)

OPTION(PHYSFS_ARCHIVE_HOG "Enable Descent I/II HOG support" TRUE)
IF(PHYSFS_ARCHIVE_HOG)
    ADD_DEFINITIONS(-DPHYSFS_SUPPORTS_HOG=1)
ENDIF(PHYSFS_ARCHIVE_HOG)

OPTION(PHYSFS_ARCHIVE_MVL "Enable Descent I/II MVL support" TRUE)
IF(PHYSFS_ARCHIVE_MVL)
    ADD_DEFINITIONS(-DPHYSFS_SUPPORTS_MVL=1)
ENDIF(PHYSFS_ARCHIVE_MVL)

OPTION(PHYSFS_ARCHIVE_QPAK "Enable Quake I/II QPAK support" TRUE)
IF(PHYSFS_ARCHIVE_QPAK)
    ADD_DEFINITIONS(-DPHYSFS_SUPPORTS_QPAK=1)
ENDIF(PHYSFS_ARCHIVE_QPAK)


# See if some archiver required zlib, and see about using system version.

IF(PHYSFS_NEED_ZLIB)
    CHECK_INCLUDE_FILE(zlib.h HAVE_ZLIB_H)
    IF(HAVE_ZLIB_H)
        CHECK_LIBRARY_EXISTS("z" "inflate" "" HAVE_LIBZ)
        IF(HAVE_LIBZ)
            SET(HAVE_SYSTEM_ZLIB TRUE)
        ENDIF(HAVE_LIBZ)
    ENDIF(HAVE_ZLIB_H)

    IF(HAVE_SYSTEM_ZLIB)
        OPTION(PHYSFS_INTERNAL_ZLIB "Link own zlib instead of system library" FALSE)
    ELSE(HAVE_SYSTEM_ZLIB)
        SET(PHYSFS_INTERNAL_ZLIB TRUE)
    ENDIF(HAVE_SYSTEM_ZLIB)

    IF(PHYSFS_INTERNAL_ZLIB)
        INCLUDE_DIRECTORIES(zlib123)
        ADD_DEFINITIONS(-DZ_PREFIX=1)
        SET(PHYSFS_SRCS ${PHYSFS_SRCS} ${ZLIB_SRCS})
    ELSE(PHYSFS_INTERNAL_ZLIB)
        SET(OPTIONAL_LIBRARY_LIBS ${OPTIONAL_LIBRARY_LIBS} z)
    ENDIF(PHYSFS_INTERNAL_ZLIB)
ENDIF(PHYSFS_NEED_ZLIB)

OPTION(PHYSFS_BUILD_STATIC "Build static library" TRUE)
IF(PHYSFS_BUILD_STATIC)
    ADD_LIBRARY(physfs-static STATIC ${PHYSFS_SRCS})
    SET_TARGET_PROPERTIES(physfs-static PROPERTIES OUTPUT_NAME "physfs")
    SET(PHYSFS_LIB_TARGET physfs-static)
    SET(PHYSFS_INSTALL_TARGETS ${PHYSFS_INSTALL_TARGETS} ";physfs-static")
ENDIF(PHYSFS_BUILD_STATIC)

OPTION(PHYSFS_BUILD_SHARED "Build shared library" TRUE)
IF(PHYSFS_BUILD_SHARED)
    ADD_LIBRARY(physfs SHARED ${PHYSFS_SRCS})
    SET_TARGET_PROPERTIES(physfs PROPERTIES VERSION ${PHYSFS_VERSION})
    SET_TARGET_PROPERTIES(physfs PROPERTIES SOVERSION ${PHYSFS_SOVERSION})
    TARGET_LINK_LIBRARIES(physfs ${OPTIONAL_LIBRARY_LIBS} ${OTHER_LDFLAGS})
    SET(PHYSFS_LIB_TARGET physfs)
    SET(PHYSFS_INSTALL_TARGETS ${PHYSFS_INSTALL_TARGETS} ";physfs")
ENDIF(PHYSFS_BUILD_SHARED)

IF(NOT PHYSFS_BUILD_SHARED AND NOT PHYSFS_BUILD_STATIC)
    MESSAGE(FATAL "Both shared and static libraries are disabled!")
ENDIF(NOT PHYSFS_BUILD_SHARED AND NOT PHYSFS_BUILD_STATIC)

# CMake FAQ says I need this...
IF(PHYSFS_BUILD_SHARED AND PHYSFS_BUILD_STATIC)
    SET_TARGET_PROPERTIES(physfs PROPERTIES CLEAN_DIRECT_OUTPUT 1)
    SET_TARGET_PROPERTIES(physfs-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
ENDIF(PHYSFS_BUILD_SHARED AND PHYSFS_BUILD_STATIC)

OPTION(PHYSFS_BUILD_TEST "Build stdio test program." TRUE)
MARK_AS_ADVANCED(PHYSFS_BUILD_TEST)
IF(PHYSFS_BUILD_TEST)
    CHECK_INCLUDE_FILE(readline/readline.h HAVE_READLINE_H)
    CHECK_INCLUDE_FILE(readline/history.h HAVE_HISTORY_H)
    IF(HAVE_READLINE_H AND HAVE_HISTORY_H)
        SET(CMAKE_REQUIRED_LIBRARIES curses)
        CHECK_LIBRARY_EXISTS("readline" "readline" "" HAVE_LIBREADLINE)
        CHECK_LIBRARY_EXISTS("readline" "history" "" HAVE_LIBHISTORY)
        IF(HAVE_LIBREADLINE AND HAVE_LIBHISTORY)
            SET(HAVE_SYSTEM_READLINE TRUE)
            SET(TEST_PHYSFS_LIBS ${TEST_PHYSFS_LIBS} " " readline curses)
            ADD_DEFINITIONS(-DPHYSFS_HAVE_READLINE=1)
        ENDIF(HAVE_LIBREADLINE AND HAVE_LIBHISTORY)
    ENDIF(HAVE_READLINE_H AND HAVE_HISTORY_H)
    ADD_EXECUTABLE(test_physfs test/test_physfs.c)
    TARGET_LINK_LIBRARIES(test_physfs ${PHYSFS_LIB_TARGET} ${TEST_PHYSFS_LIBS} ${OTHER_LDFLAGS})
    SET(PHYSFS_INSTALL_TARGETS ${PHYSFS_INSTALL_TARGETS} ";test_physfs")
ENDIF(PHYSFS_BUILD_TEST)

OPTION(PHYSFS_BUILD_WX_TEST "Build wxWidgets test program." TRUE)
MARK_AS_ADVANCED(PHYSFS_BUILD_WX_TEST)
IF(PHYSFS_BUILD_WX_TEST)
    SET(wxWidgets_USE_LIBS base core adv)
    SET(wxWidgets_INCLUDE_DIRS_NO_SYSTEM 1)
    FIND_PACKAGE(wxWidgets)
    IF(wxWidgets_FOUND)
        INCLUDE(${wxWidgets_USE_FILE})
        ADD_EXECUTABLE(wxtest_physfs test/wxtest_physfs.cpp)
        SET_SOURCE_FILES_PROPERTIES(test/wxtest_physfs.cpp COMPILE_FLAGS ${wxWidgets_CXX_FLAGS})
        TARGET_LINK_LIBRARIES(wxtest_physfs ${PHYSFS_LIB_TARGET} ${wxWidgets_LIBRARIES} ${OTHER_LDFLAGS})
        SET(PHYSFS_INSTALL_TARGETS ${PHYSFS_INSTALL_TARGETS} ";wxtest_physfs")
    ELSE(wxWidgets_FOUND)
        MESSAGE(STATUS "wxWidgets not found. Disabling wx test app.")
        SET(PHYSFS_BUILD_WX_TEST FALSE)
    ENDIF(wxWidgets_FOUND)
ENDIF(PHYSFS_BUILD_WX_TEST)

INSTALL(TARGETS ${PHYSFS_INSTALL_TARGETS}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
INSTALL(FILES physfs.h DESTINATION include)

FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
    ADD_CUSTOM_TARGET(docs ${DOXYGEN_EXECUTABLE} COMMENT "Building documentation")
ELSE(DOXYGEN_FOUND)
    MESSAGE(STATUS "Doxygen not found. You won't be able to build documentation.")
ENDIF(DOXYGEN_FOUND)

IF(UNIX)
    ADD_CUSTOM_TARGET(dist ./extras/makedist.sh ${PHYSFS_VERSION} COMMENT "Building source tarball")
ENDIF(UNIX)

MACRO(MESSAGE_BOOL_OPTION _NAME _VALUE)
    IF(${_VALUE})
        MESSAGE(STATUS "  ${_NAME}: enabled")
    ELSE(${_VALUE})
        MESSAGE(STATUS "  ${_NAME}: disabled")
    ENDIF(${_VALUE})
ENDMACRO(MESSAGE_BOOL_OPTION)

MESSAGE(STATUS "PhysicsFS will build with the following options:")
MESSAGE_BOOL_OPTION("ZIP support" PHYSFS_ARCHIVE_ZIP)
MESSAGE_BOOL_OPTION("7zip support" PHYSFS_ARCHIVE_7Z)
MESSAGE_BOOL_OPTION("GRP support" PHYSFS_ARCHIVE_GRP)
MESSAGE_BOOL_OPTION("WAD support" PHYSFS_ARCHIVE_WAD)
MESSAGE_BOOL_OPTION("HOG support" PHYSFS_ARCHIVE_HOG)
MESSAGE_BOOL_OPTION("MVL support" PHYSFS_ARCHIVE_MVL)
MESSAGE_BOOL_OPTION("QPAK support" PHYSFS_ARCHIVE_QPAK)
MESSAGE_BOOL_OPTION("CD-ROM drive support" PHYSFS_HAVE_CDROM_SUPPORT)
MESSAGE_BOOL_OPTION("Thread safety" PHYSFS_HAVE_THREAD_SUPPORT)
MESSAGE_BOOL_OPTION("Build own zlib" PHYSFS_INTERNAL_ZLIB)
MESSAGE_BOOL_OPTION("Build static library" PHYSFS_BUILD_STATIC)
MESSAGE_BOOL_OPTION("Build shared library" PHYSFS_BUILD_SHARED)
MESSAGE_BOOL_OPTION("Build wxWidgets test program" PHYSFS_BUILD_WX_TEST)
MESSAGE_BOOL_OPTION("Build stdio test program" PHYSFS_BUILD_TEST)
IF(PHYSFS_BUILD_TEST)
    MESSAGE_BOOL_OPTION("  Use readline in test program" HAVE_SYSTEM_READLINE)
ENDIF(PHYSFS_BUILD_TEST)

# end of CMakeLists.txt ...

