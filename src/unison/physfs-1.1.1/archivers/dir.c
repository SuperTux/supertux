/*
 * Standard directory I/O support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "physfs.h"

#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

static PHYSFS_sint64 DIR_read(fvoid *opaque, void *buffer,
                              PHYSFS_uint32 objSize, PHYSFS_uint32 objCount)
{
    PHYSFS_sint64 retval;
    retval = __PHYSFS_platformRead(opaque, buffer, objSize, objCount);
    return(retval);
} /* DIR_read */


static PHYSFS_sint64 DIR_write(fvoid *opaque, const void *buffer,
                               PHYSFS_uint32 objSize, PHYSFS_uint32 objCount)
{
    PHYSFS_sint64 retval;
    retval = __PHYSFS_platformWrite(opaque, buffer, objSize, objCount);
    return(retval);
} /* DIR_write */


static int DIR_eof(fvoid *opaque)
{
    return(__PHYSFS_platformEOF(opaque));
} /* DIR_eof */


static PHYSFS_sint64 DIR_tell(fvoid *opaque)
{
    return(__PHYSFS_platformTell(opaque));
} /* DIR_tell */


static int DIR_seek(fvoid *opaque, PHYSFS_uint64 offset)
{
    return(__PHYSFS_platformSeek(opaque, offset));
} /* DIR_seek */


static PHYSFS_sint64 DIR_fileLength(fvoid *opaque)
{
    return(__PHYSFS_platformFileLength(opaque));
} /* DIR_fileLength */


static int DIR_fileClose(fvoid *opaque)
{
    /*
     * we manually flush the buffer, since that's the place a close will
     *  most likely fail, but that will leave the file handle in an undefined
     *  state if it fails. Flush failures we can recover from.
     */
    BAIL_IF_MACRO(!__PHYSFS_platformFlush(opaque), NULL, 0);
    BAIL_IF_MACRO(!__PHYSFS_platformClose(opaque), NULL, 0);
    return(1);
} /* DIR_fileClose */


static int DIR_isArchive(const char *filename, int forWriting)
{
    /* directories ARE archives in this driver... */
    return(__PHYSFS_platformIsDirectory(filename));
} /* DIR_isArchive */


static void *DIR_openArchive(const char *name, int forWriting)
{
    const char *dirsep = PHYSFS_getDirSeparator();
    char *retval = NULL;
    size_t namelen = strlen(name);
    size_t seplen = strlen(dirsep);

    /* !!! FIXME: when is this not called right before openArchive? */
    BAIL_IF_MACRO(!DIR_isArchive(name, forWriting),
                    ERR_UNSUPPORTED_ARCHIVE, 0);

    retval = allocator.Malloc(namelen + seplen + 1);
    BAIL_IF_MACRO(retval == NULL, ERR_OUT_OF_MEMORY, NULL);

        /* make sure there's a dir separator at the end of the string */
    strcpy(retval, name);
    if (strcmp((name + namelen) - seplen, dirsep) != 0)
        strcat(retval, dirsep);

    return(retval);
} /* DIR_openArchive */


static void DIR_enumerateFiles(dvoid *opaque, const char *dname,
                               int omitSymLinks, PHYSFS_EnumFilesCallback cb,
                               const char *origdir, void *callbackdata)
{
    char *d = __PHYSFS_platformCvtToDependent((char *)opaque, dname, NULL);
    if (d != NULL)
    {
        __PHYSFS_platformEnumerateFiles(d, omitSymLinks, cb,
                                        origdir, callbackdata);
        allocator.Free(d);
    } /* if */
} /* DIR_enumerateFiles */


static int DIR_exists(dvoid *opaque, const char *name)
{
    char *f = __PHYSFS_platformCvtToDependent((char *) opaque, name, NULL);
    int retval;

    BAIL_IF_MACRO(f == NULL, NULL, 0);
    retval = __PHYSFS_platformExists(f);
    allocator.Free(f);
    return(retval);
} /* DIR_exists */


static int DIR_isDirectory(dvoid *opaque, const char *name, int *fileExists)
{
    char *d = __PHYSFS_platformCvtToDependent((char *) opaque, name, NULL);
    int retval = 0;

    BAIL_IF_MACRO(d == NULL, NULL, 0);
    *fileExists = __PHYSFS_platformExists(d);
    if (*fileExists)
        retval = __PHYSFS_platformIsDirectory(d);
    allocator.Free(d);
    return(retval);
} /* DIR_isDirectory */


static int DIR_isSymLink(dvoid *opaque, const char *name, int *fileExists)
{
    char *f = __PHYSFS_platformCvtToDependent((char *) opaque, name, NULL);
    int retval = 0;

    BAIL_IF_MACRO(f == NULL, NULL, 0);
    *fileExists = __PHYSFS_platformExists(f);
    if (*fileExists)
        retval = __PHYSFS_platformIsSymLink(f);
    allocator.Free(f);
    return(retval);
} /* DIR_isSymLink */


static PHYSFS_sint64 DIR_getLastModTime(dvoid *opaque,
                                        const char *name,
                                        int *fileExists)
{
    char *d = __PHYSFS_platformCvtToDependent((char *) opaque, name, NULL);
    PHYSFS_sint64 retval = -1;

    BAIL_IF_MACRO(d == NULL, NULL, 0);
    *fileExists = __PHYSFS_platformExists(d);
    if (*fileExists)
        retval = __PHYSFS_platformGetLastModTime(d);
    allocator.Free(d);
    return(retval);
} /* DIR_getLastModTime */


static fvoid *doOpen(dvoid *opaque, const char *name,
                     void *(*openFunc)(const char *filename),
                     int *fileExists)
{
    char *f = __PHYSFS_platformCvtToDependent((char *) opaque, name, NULL);
    void *rc = NULL;

    BAIL_IF_MACRO(f == NULL, NULL, NULL);

    if (fileExists != NULL)
    {
        *fileExists = __PHYSFS_platformExists(f);
        if (!(*fileExists))
        {
            allocator.Free(f);
            return(NULL);
        } /* if */
    } /* if */

    rc = openFunc(f);
    allocator.Free(f);

    return((fvoid *) rc);
} /* doOpen */


static fvoid *DIR_openRead(dvoid *opaque, const char *fnm, int *exist)
{
    return(doOpen(opaque, fnm, __PHYSFS_platformOpenRead, exist));
} /* DIR_openRead */


static fvoid *DIR_openWrite(dvoid *opaque, const char *filename)
{
    return(doOpen(opaque, filename, __PHYSFS_platformOpenWrite, NULL));
} /* DIR_openWrite */


static fvoid *DIR_openAppend(dvoid *opaque, const char *filename)
{
    return(doOpen(opaque, filename, __PHYSFS_platformOpenAppend, NULL));
} /* DIR_openAppend */


static int DIR_remove(dvoid *opaque, const char *name)
{
    char *f = __PHYSFS_platformCvtToDependent((char *) opaque, name, NULL);
    int retval;

    BAIL_IF_MACRO(f == NULL, NULL, 0);
    retval = __PHYSFS_platformDelete(f);
    allocator.Free(f);
    return(retval);
} /* DIR_remove */


static int DIR_mkdir(dvoid *opaque, const char *name)
{
    char *f = __PHYSFS_platformCvtToDependent((char *) opaque, name, NULL);
    int retval;

    BAIL_IF_MACRO(f == NULL, NULL, 0);
    retval = __PHYSFS_platformMkDir(f);
    allocator.Free(f);
    return(retval);
} /* DIR_mkdir */


static void DIR_dirClose(dvoid *opaque)
{
    allocator.Free(opaque);
} /* DIR_dirClose */



const PHYSFS_ArchiveInfo __PHYSFS_ArchiveInfo_DIR =
{
    "",
    DIR_ARCHIVE_DESCRIPTION,
    "Ryan C. Gordon <icculus@icculus.org>",
    "http://icculus.org/physfs/",
};



const PHYSFS_Archiver __PHYSFS_Archiver_DIR =
{
    &__PHYSFS_ArchiveInfo_DIR,
    DIR_isArchive,          /* isArchive() method      */
    DIR_openArchive,        /* openArchive() method    */
    DIR_enumerateFiles,     /* enumerateFiles() method */
    DIR_exists,             /* exists() method         */
    DIR_isDirectory,        /* isDirectory() method    */
    DIR_isSymLink,          /* isSymLink() method      */
    DIR_getLastModTime,     /* getLastModTime() method */
    DIR_openRead,           /* openRead() method       */
    DIR_openWrite,          /* openWrite() method      */
    DIR_openAppend,         /* openAppend() method     */
    DIR_remove,             /* remove() method         */
    DIR_mkdir,              /* mkdir() method          */
    DIR_dirClose,           /* dirClose() method       */
    DIR_read,               /* read() method           */
    DIR_write,              /* write() method          */
    DIR_eof,                /* eof() method            */
    DIR_tell,               /* tell() method           */
    DIR_seek,               /* seek() method           */
    DIR_fileLength,         /* fileLength() method     */
    DIR_fileClose           /* fileClose() method      */
};

/* end of dir.c ... */

