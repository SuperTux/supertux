@echo off
rem this is a simple batch file to build PhysicsFS on OS/2. You need to have
rem  the Innotek libc and GCC (or "kLIBC") installed for this to work:
rem
rem     http://svn.netlabs.org/libc
rem
rem This script (and, indeed, our OS/2 support) could use some tweaking.
rem  Patches go to icculus@icculus.org ...

set PHYSFSLANG=PHYSFS_LANG_ENGLISH
set DEBUGFLAGS=-D_NDEBUG -O2 -s
rem set CFLAGS=%DEBUGFLAGS% -Wall -Werror -Zomf -Zmt -Zmtd -I. -Izlib123 -c -D__ST_MT_ERRNO__ -DOS2 -DZ_PREFIX -DPHYSFS_SUPPORTS_ZIP -DPHYSFS_SUPPORTS_7Z -DPHYSFS_SUPPORTS_GRP -DPHYSFS_SUPPORTS_WAD -DPHYSFS_SUPPORTS_QPAK -DPHYSFS_SUPPORTS_HOG -DPHYSFS_SUPPORTS_MVL -DPHYSFS_LANG=%PHYSFSLANG% -DHAVE_ASSERT_H
set CFLAGS=%DEBUGFLAGS% -Wall -Werror -Zomf -I. -Iz -c -D__ST_MT_ERRNO__ -DOS2 -DZ_PREFIX -DPHYSFS_SUPPORTS_ZIP -DPHYSFS_SUPPORTS_7Z -DPHYSFS_SUPPORTS_GRP -DPHYSFS_SUPPORTS_WAD -DPHYSFS_SUPPORTS_QPAK -DPHYSFS_SUPPORTS_HOG -DPHYSFS_SUPPORTS_MVL -DHAVE_ASSERT_H

rem goto :dolinking

@echo cleaning up any previous build...
@mkdir bin 2>NUL
@erase /N bin\*.* 2>NUL

@echo Building export definitions...
@echo ;don't edit this directly! It is rewritten by makeos2.cmd! > bin\test_physfs.def
@echo NAME TESTPHYSFS WINDOWCOMPAT >> bin\test_physfs.def
@echo DESCRIPTION 'PhysicsFS: http://icculus.org/physfs/' >> bin\test_physfs.def
@echo STACKSIZE 0x10000 >> bin\test_physfs.def
@echo BASE=0x10000 >> bin\test_physfs.def
@echo PROTMODE >> bin\test_physfs.def

@echo ;don't edit this directly! It is rewritten by makeos2.cmd! > bin\physfs.def
@echo LIBRARY 'physfs' INITINSTANCE TERMINSTANCE >> bin\physfs.def
@echo STACKSIZE 0x10000 >> bin\physfs.def
@echo CODE LOADONCALL >> bin\physfs.def
@echo DATA LOADONCALL NONSHARED MULTIPLE >> bin\physfs.def
@echo DESCRIPTION 'PhysicsFS: http://icculus.org/physfs/' >> bin\physfs.def
@echo EXPORTS >> bin\physfs.def
@echo "_PHYSFS_getLinkedVersion" >> bin\physfs.def
@echo "_PHYSFS_init" >> bin\physfs.def
@echo "_PHYSFS_deinit" >> bin\physfs.def
@echo "_PHYSFS_isInit" >> bin\physfs.def
@echo "_PHYSFS_supportedArchiveTypes" >> bin\physfs.def
@echo "_PHYSFS_freeList" >> bin\physfs.def
@echo "_PHYSFS_getLastError" >> bin\physfs.def
@echo "_PHYSFS_getDirSeparator" >> bin\physfs.def
@echo "_PHYSFS_permitSymbolicLinks" >> bin\physfs.def
@echo "_PHYSFS_symbolicLinksPermitted" >> bin\physfs.def
@echo "_PHYSFS_getCdRomDirs" >> bin\physfs.def
@echo "_PHYSFS_getBaseDir" >> bin\physfs.def
@echo "_PHYSFS_getUserDir" >> bin\physfs.def
@echo "_PHYSFS_getWriteDir" >> bin\physfs.def
@echo "_PHYSFS_setWriteDir" >> bin\physfs.def
@echo "_PHYSFS_addToSearchPath" >> bin\physfs.def
@echo "_PHYSFS_removeFromSearchPath" >> bin\physfs.def
@echo "_PHYSFS_getSearchPath" >> bin\physfs.def
@echo "_PHYSFS_setSaneConfig" >> bin\physfs.def
@echo "_PHYSFS_mkdir" >> bin\physfs.def
@echo "_PHYSFS_delete" >> bin\physfs.def
@echo "_PHYSFS_getRealDir" >> bin\physfs.def
@echo "_PHYSFS_enumerateFiles" >> bin\physfs.def
@echo "_PHYSFS_exists" >> bin\physfs.def
@echo "_PHYSFS_isDirectory" >> bin\physfs.def
@echo "_PHYSFS_isSymbolicLink" >> bin\physfs.def
@echo "_PHYSFS_openWrite" >> bin\physfs.def
@echo "_PHYSFS_openAppend" >> bin\physfs.def
@echo "_PHYSFS_openRead" >> bin\physfs.def
@echo "_PHYSFS_close" >> bin\physfs.def
@echo "_PHYSFS_read" >> bin\physfs.def
@echo "_PHYSFS_write" >> bin\physfs.def
@echo "_PHYSFS_eof" >> bin\physfs.def
@echo "_PHYSFS_tell" >> bin\physfs.def
@echo "_PHYSFS_seek" >> bin\physfs.def
@echo "_PHYSFS_fileLength" >> bin\physfs.def
@echo "_PHYSFS_swapSLE16" >> bin\physfs.def
@echo "_PHYSFS_swapULE16" >> bin\physfs.def
@echo "_PHYSFS_swapSLE32" >> bin\physfs.def
@echo "_PHYSFS_swapULE32" >> bin\physfs.def
@echo "_PHYSFS_swapSLE64" >> bin\physfs.def
@echo "_PHYSFS_swapULE64" >> bin\physfs.def
@echo "_PHYSFS_swapSBE16" >> bin\physfs.def
@echo "_PHYSFS_swapUBE16" >> bin\physfs.def
@echo "_PHYSFS_swapSBE32" >> bin\physfs.def
@echo "_PHYSFS_swapUBE32" >> bin\physfs.def
@echo "_PHYSFS_swapSBE64" >> bin\physfs.def
@echo "_PHYSFS_swapUBE64" >> bin\physfs.def
@echo "_PHYSFS_getLastModTime" >> bin\physfs.def
@echo "_PHYSFS_readSLE16" >> bin\physfs.def
@echo "_PHYSFS_readULE16" >> bin\physfs.def
@echo "_PHYSFS_readSLE32" >> bin\physfs.def
@echo "_PHYSFS_readULE32" >> bin\physfs.def
@echo "_PHYSFS_readSLE64" >> bin\physfs.def
@echo "_PHYSFS_readULE64" >> bin\physfs.def
@echo "_PHYSFS_readSBE16" >> bin\physfs.def
@echo "_PHYSFS_readUBE16" >> bin\physfs.def
@echo "_PHYSFS_readSBE32" >> bin\physfs.def
@echo "_PHYSFS_readUBE32" >> bin\physfs.def
@echo "_PHYSFS_readSBE64" >> bin\physfs.def
@echo "_PHYSFS_readUBE64" >> bin\physfs.def
@echo "_PHYSFS_writeSLE16" >> bin\physfs.def
@echo "_PHYSFS_writeULE16" >> bin\physfs.def
@echo "_PHYSFS_writeSLE32" >> bin\physfs.def
@echo "_PHYSFS_writeULE32" >> bin\physfs.def
@echo "_PHYSFS_writeSLE64" >> bin\physfs.def
@echo "_PHYSFS_writeULE64" >> bin\physfs.def
@echo "_PHYSFS_writeSBE16" >> bin\physfs.def
@echo "_PHYSFS_writeUBE16" >> bin\physfs.def
@echo "_PHYSFS_writeSBE32" >> bin\physfs.def
@echo "_PHYSFS_writeUBE32" >> bin\physfs.def
@echo "_PHYSFS_writeSBE64" >> bin\physfs.def
@echo "_PHYSFS_writeUBE64" >> bin\physfs.def
@echo "_PHYSFS_setBuffer" >> bin\physfs.def
@echo "_PHYSFS_flush" >> bin\physfs.def
@echo "_PHYSFS_mount" >> bin\physfs.def
@echo "_PHYSFS_getMountPoint" >> bin\physfs.def
@echo "_PHYSFS_setAllocator" >> bin\physfs.def
@echo "_PHYSFS_getCdRomDirsCallback" >> bin\physfs.def
@echo "_PHYSFS_getSearchPathCallback" >> bin\physfs.def
@echo "_PHYSFS_enumerateFilesCallback" >> bin\physfs.def
@echo "_PHYSFS_utf8ToUcs2" >> bin\physfs.def
@echo "_PHYSFS_utf8FromUcs2" >> bin\physfs.def
@echo "_PHYSFS_utf8ToUcs4" >> bin\physfs.def
@echo "_PHYSFS_utf8FromUcs4" >> bin\physfs.def
@echo "_PHYSFS_utf8FromLatin1" >> bin\physfs.def

@echo Building export library...
emximp -o bin/physfs.lib bin/physfs.def
emximp -o bin/physfs.a bin/physfs.def

@echo Compiling PhysicsFS library...
@echo on
gcc %CFLAGS% -o bin/physfs.obj physfs.c
gcc %CFLAGS% -o bin/physfs_byteorder.obj physfs_byteorder.c
gcc %CFLAGS% -o bin/physfs_unicode.obj physfs_unicode.c
gcc %CFLAGS% -o bin/os2.obj platform/os2.c
gcc %CFLAGS% -o bin/dir.obj archivers/dir.c
gcc %CFLAGS% -o bin/grp.obj archivers/grp.c
gcc %CFLAGS% -o bin/wad.obj archivers/wad.c
gcc %CFLAGS% -o bin/lzma.obj archivers/lzma.c
gcc %CFLAGS% -o bin/zip.obj archivers/zip.c
gcc %CFLAGS% -o bin/qpak.obj archivers/qpak.c
gcc %CFLAGS% -o bin/hog.obj archivers/hog.c
gcc %CFLAGS% -o bin/mvl.obj archivers/mvl.c
gcc %CFLAGS% -o bin/adler32.obj zlib123/adler32.c
gcc %CFLAGS% -o bin/compress.obj zlib123/compress.c
gcc %CFLAGS% -o bin/crc32.obj zlib123/crc32.c
gcc %CFLAGS% -o bin/deflate.obj zlib123/deflate.c
gcc %CFLAGS% -o bin/gzio.obj zlib123/gzio.c
gcc %CFLAGS% -o bin/infback.obj zlib123/infback.c
gcc %CFLAGS% -o bin/inffast.obj zlib123/inffast.c
gcc %CFLAGS% -o bin/inflate.obj zlib123/inflate.c
gcc %CFLAGS% -o bin/inftrees.obj zlib123/inftrees.c
gcc %CFLAGS% -o bin/trees.obj zlib123/trees.c
gcc %CFLAGS% -o bin/uncompr.obj zlib123/uncompr.c
gcc %CFLAGS% -o bin/zutil.obj zlib123/zutil.c
gcc %CFLAGS% -o bin/7zBuffer.obj lzma/7zBuffer.c
gcc %CFLAGS% -o bin/7zCrc.obj lzma/7zCrc.c
gcc %CFLAGS% -o bin/7zDecode.obj lzma/7zDecode.c
gcc %CFLAGS% -o bin/7zExtract.obj lzma/7zExtract.c
gcc %CFLAGS% -o bin/7zHeader.obj lzma/7zHeader.c
gcc %CFLAGS% -o bin/7zIn.obj lzma/7zIn.c
gcc %CFLAGS% -o bin/7zItem.obj lzma/7zItem.c
gcc %CFLAGS% -o bin/7zMethodID.obj lzma/7zMethodID.c
gcc %CFLAGS% -o bin/LzmaDecode.obj lzma/LzmaDecode.c
gcc %CFLAGS% -o bin/LzmaStateDecode.obj lzma/LzmaStateDecode.c
@echo off

:dolinking
@echo Linking PhysicsFS library...
gcc %DEBUGFLAGS% -Zdll -Zcrtdll -Zomf -o bin/physfs.dll bin/*.obj bin/physfs.def

rem goto :builddone

@echo Compiling test program...
gcc %CFLAGS% -o bin/test_physfs.obj test/test_physfs.c
@echo Linking test program...
gcc %DEBUGFLAGS% -Zomf -Zcrtdll -o bin/test_physfs.exe bin/test_physfs.obj bin/physfs.lib bin/test_physfs.def

:builddone

@echo "All done!"

rem end of makeos2.cmd ...

