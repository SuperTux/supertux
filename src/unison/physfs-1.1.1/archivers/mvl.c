/*
 * MVL support routines for PhysicsFS.
 *
 * This driver handles Descent II Movielib archives.
 *
 * The file format of MVL is quite easy...
 *
 *   //MVL File format - Written by Heiko Herrmann
 *   char sig[4] = {'D','M', 'V', 'L'}; // "DMVL"=Descent MoVie Library
 *
 *   int num_files; // the number of files in this MVL
 *
 *   struct {
 *    char file_name[13]; // Filename, padded to 13 bytes with 0s
 *    int file_size; // filesize in bytes
 *   }DIR_STRUCT[num_files];
 *
 *   struct {
 *    char data[file_size]; // The file data
 *   }FILE_STRUCT[num_files];
 *
 * (That info is from http://www.descent2.com/ddn/specs/mvl/)
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Bradley Bell.
 *  Based on grp.c by Ryan C. Gordon.
 */

#if (defined PHYSFS_SUPPORTS_MVL)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "physfs.h"

#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

typedef struct
{
    char name[13];
    PHYSFS_uint32 startPos;
    PHYSFS_uint32 size;
} MVLentry;

typedef struct
{
    char *filename;
    PHYSFS_sint64 last_mod_time;
    PHYSFS_uint32 entryCount;
    MVLentry *entries;
} MVLinfo;

typedef struct
{
    void *handle;
    MVLentry *entry;
    PHYSFS_uint32 curPos;
} MVLfileinfo;


static void MVL_dirClose(dvoid *opaque)
{
    MVLinfo *info = ((MVLinfo *) opaque);
    allocator.Free(info->filename);
    allocator.Free(info->entries);
    allocator.Free(info);
} /* MVL_dirClose */


static PHYSFS_sint64 MVL_read(fvoid *opaque, void *buffer,
                              PHYSFS_uint32 objSize, PHYSFS_uint32 objCount)
{
    MVLfileinfo *finfo = (MVLfileinfo *) opaque;
    MVLentry *entry = finfo->entry;
    PHYSFS_uint32 bytesLeft = entry->size - finfo->curPos;
    PHYSFS_uint32 objsLeft = (bytesLeft / objSize);
    PHYSFS_sint64 rc;

    if (objsLeft < objCount)
        objCount = objsLeft;

    rc = __PHYSFS_platformRead(finfo->handle, buffer, objSize, objCount);
    if (rc > 0)
        finfo->curPos += (PHYSFS_uint32) (rc * objSize);

    return(rc);
} /* MVL_read */


static PHYSFS_sint64 MVL_write(fvoid *opaque, const void *buffer,
                               PHYSFS_uint32 objSize, PHYSFS_uint32 objCount)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, -1);
} /* MVL_write */


static int MVL_eof(fvoid *opaque)
{
    MVLfileinfo *finfo = (MVLfileinfo *) opaque;
    MVLentry *entry = finfo->entry;
    return(finfo->curPos >= entry->size);
} /* MVL_eof */


static PHYSFS_sint64 MVL_tell(fvoid *opaque)
{
    return(((MVLfileinfo *) opaque)->curPos);
} /* MVL_tell */


static int MVL_seek(fvoid *opaque, PHYSFS_uint64 offset)
{
    MVLfileinfo *finfo = (MVLfileinfo *) opaque;
    MVLentry *entry = finfo->entry;
    int rc;

    BAIL_IF_MACRO(offset < 0, ERR_INVALID_ARGUMENT, 0);
    BAIL_IF_MACRO(offset >= entry->size, ERR_PAST_EOF, 0);
    rc = __PHYSFS_platformSeek(finfo->handle, entry->startPos + offset);
    if (rc)
        finfo->curPos = (PHYSFS_uint32) offset;

    return(rc);
} /* MVL_seek */


static PHYSFS_sint64 MVL_fileLength(fvoid *opaque)
{
    MVLfileinfo *finfo = (MVLfileinfo *) opaque;
    return((PHYSFS_sint64) finfo->entry->size);
} /* MVL_fileLength */


static int MVL_fileClose(fvoid *opaque)
{
    MVLfileinfo *finfo = (MVLfileinfo *) opaque;
    BAIL_IF_MACRO(!__PHYSFS_platformClose(finfo->handle), NULL, 0);
    allocator.Free(finfo);
    return(1);
} /* MVL_fileClose */


static int mvl_open(const char *filename, int forWriting,
                    void **fh, PHYSFS_uint32 *count)
{
    PHYSFS_uint8 buf[4];

    *fh = NULL;
    BAIL_IF_MACRO(forWriting, ERR_ARC_IS_READ_ONLY, 0);

    *fh = __PHYSFS_platformOpenRead(filename);
    BAIL_IF_MACRO(*fh == NULL, NULL, 0);
    
    if (__PHYSFS_platformRead(*fh, buf, 4, 1) != 1)
        goto openMvl_failed;

    if (memcmp(buf, "DMVL", 4) != 0)
    {
        __PHYSFS_setError(ERR_UNSUPPORTED_ARCHIVE);
        goto openMvl_failed;
    } /* if */

    if (__PHYSFS_platformRead(*fh, count, sizeof (PHYSFS_uint32), 1) != 1)
        goto openMvl_failed;

    *count = PHYSFS_swapULE32(*count);

    return(1);

openMvl_failed:
    if (*fh != NULL)
        __PHYSFS_platformClose(*fh);

    *count = -1;
    *fh = NULL;
    return(0);
} /* mvl_open */


static int MVL_isArchive(const char *filename, int forWriting)
{
    void *fh;
    PHYSFS_uint32 fileCount;
    int retval = mvl_open(filename, forWriting, &fh, &fileCount);

    if (fh != NULL)
        __PHYSFS_platformClose(fh);

    return(retval);
} /* MVL_isArchive */


static int mvl_entry_cmp(void *_a, PHYSFS_uint32 one, PHYSFS_uint32 two)
{
    MVLentry *a = (MVLentry *) _a;
    return(strcmp(a[one].name, a[two].name));
} /* mvl_entry_cmp */


static void mvl_entry_swap(void *_a, PHYSFS_uint32 one, PHYSFS_uint32 two)
{
    MVLentry tmp;
    MVLentry *first = &(((MVLentry *) _a)[one]);
    MVLentry *second = &(((MVLentry *) _a)[two]);
    memcpy(&tmp, first, sizeof (MVLentry));
    memcpy(first, second, sizeof (MVLentry));
    memcpy(second, &tmp, sizeof (MVLentry));
} /* mvl_entry_swap */


static int mvl_load_entries(const char *name, int forWriting, MVLinfo *info)
{
    void *fh = NULL;
    PHYSFS_uint32 fileCount;
    PHYSFS_uint32 location = 8;  /* sizeof sig. */
    MVLentry *entry;

    BAIL_IF_MACRO(!mvl_open(name, forWriting, &fh, &fileCount), NULL, 0);
    info->entryCount = fileCount;
    info->entries = (MVLentry *) allocator.Malloc(sizeof(MVLentry)*fileCount);
    if (info->entries == NULL)
    {
        __PHYSFS_platformClose(fh);
        BAIL_MACRO(ERR_OUT_OF_MEMORY, 0);
    } /* if */

    location += (17 * fileCount);

    for (entry = info->entries; fileCount > 0; fileCount--, entry++)
    {
        if (__PHYSFS_platformRead(fh, &entry->name, 13, 1) != 1)
        {
            __PHYSFS_platformClose(fh);
            return(0);
        } /* if */

        if (__PHYSFS_platformRead(fh, &entry->size, 4, 1) != 1)
        {
            __PHYSFS_platformClose(fh);
            return(0);
        } /* if */

        entry->size = PHYSFS_swapULE32(entry->size);
        entry->startPos = location;
        location += entry->size;
    } /* for */

    __PHYSFS_platformClose(fh);

    __PHYSFS_sort(info->entries, info->entryCount,
                  mvl_entry_cmp, mvl_entry_swap);
    return(1);
} /* mvl_load_entries */


static void *MVL_openArchive(const char *name, int forWriting)
{
    PHYSFS_sint64 modtime = __PHYSFS_platformGetLastModTime(name);
    MVLinfo *info = (MVLinfo *) allocator.Malloc(sizeof (MVLinfo));

    BAIL_IF_MACRO(info == NULL, ERR_OUT_OF_MEMORY, NULL);
    memset(info, '\0', sizeof (MVLinfo));

    info->filename = (char *) allocator.Malloc(strlen(name) + 1);
    GOTO_IF_MACRO(!info->filename, ERR_OUT_OF_MEMORY, MVL_openArchive_failed);
    if (!mvl_load_entries(name, forWriting, info))
        goto MVL_openArchive_failed;

    strcpy(info->filename, name);
    info->last_mod_time = modtime;
    return(info);

MVL_openArchive_failed:
    if (info != NULL)
    {
        if (info->filename != NULL)
            allocator.Free(info->filename);
        if (info->entries != NULL)
            allocator.Free(info->entries);
        allocator.Free(info);
    } /* if */

    return(NULL);
} /* MVL_openArchive */


static void MVL_enumerateFiles(dvoid *opaque, const char *dname,
                               int omitSymLinks, PHYSFS_EnumFilesCallback cb,
                               const char *origdir, void *callbackdata)
{
    /* no directories in MVL files. */
    if (*dname == '\0')
    {
        MVLinfo *info = ((MVLinfo *) opaque);
        MVLentry *entry = info->entries;
        PHYSFS_uint32 max = info->entryCount;
        PHYSFS_uint32 i;

        for (i = 0; i < max; i++, entry++)
            cb(callbackdata, origdir, entry->name);
    } /* if */
} /* MVL_enumerateFiles */


static MVLentry *mvl_find_entry(MVLinfo *info, const char *name)
{
    char *ptr = strchr(name, '.');
    MVLentry *a = info->entries;
    PHYSFS_sint32 lo = 0;
    PHYSFS_sint32 hi = (PHYSFS_sint32) (info->entryCount - 1);
    PHYSFS_sint32 middle;
    int rc;

    /*
     * Rule out filenames to avoid unneeded processing...no dirs,
     *   big filenames, or extensions > 3 chars.
     */
    BAIL_IF_MACRO((ptr) && (strlen(ptr) > 4), ERR_NO_SUCH_FILE, NULL);
    BAIL_IF_MACRO(strlen(name) > 12, ERR_NO_SUCH_FILE, NULL);
    BAIL_IF_MACRO(strchr(name, '/') != NULL, ERR_NO_SUCH_FILE, NULL);

    while (lo <= hi)
    {
        middle = lo + ((hi - lo) / 2);
        rc = __PHYSFS_stricmpASCII(name, a[middle].name);
        if (rc == 0)  /* found it! */
            return(&a[middle]);
        else if (rc > 0)
            lo = middle + 1;
        else
            hi = middle - 1;
    } /* while */

    BAIL_MACRO(ERR_NO_SUCH_FILE, NULL);
} /* mvl_find_entry */


static int MVL_exists(dvoid *opaque, const char *name)
{
    return(mvl_find_entry(((MVLinfo *) opaque), name) != NULL);
} /* MVL_exists */


static int MVL_isDirectory(dvoid *opaque, const char *name, int *fileExists)
{
    *fileExists = MVL_exists(opaque, name);
    return(0);  /* never directories in a groupfile. */
} /* MVL_isDirectory */


static int MVL_isSymLink(dvoid *opaque, const char *name, int *fileExists)
{
    *fileExists = MVL_exists(opaque, name);
    return(0);  /* never symlinks in a groupfile. */
} /* MVL_isSymLink */


static PHYSFS_sint64 MVL_getLastModTime(dvoid *opaque,
                                        const char *name,
                                        int *fileExists)
{
    MVLinfo *info = ((MVLinfo *) opaque);
    PHYSFS_sint64 retval = -1;

    *fileExists = (mvl_find_entry(info, name) != NULL);
    if (*fileExists)  /* use time of MVL itself in the physical filesystem. */
        retval = info->last_mod_time;

    return(retval);
} /* MVL_getLastModTime */


static fvoid *MVL_openRead(dvoid *opaque, const char *fnm, int *fileExists)
{
    MVLinfo *info = ((MVLinfo *) opaque);
    MVLfileinfo *finfo;
    MVLentry *entry;

    entry = mvl_find_entry(info, fnm);
    *fileExists = (entry != NULL);
    BAIL_IF_MACRO(entry == NULL, NULL, NULL);

    finfo = (MVLfileinfo *) allocator.Malloc(sizeof (MVLfileinfo));
    BAIL_IF_MACRO(finfo == NULL, ERR_OUT_OF_MEMORY, NULL);

    finfo->handle = __PHYSFS_platformOpenRead(info->filename);
    if ( (finfo->handle == NULL) ||
         (!__PHYSFS_platformSeek(finfo->handle, entry->startPos)) )
    {
        allocator.Free(finfo);
        return(NULL);
    } /* if */

    finfo->curPos = 0;
    finfo->entry = entry;
    return(finfo);
} /* MVL_openRead */


static fvoid *MVL_openWrite(dvoid *opaque, const char *name)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, NULL);
} /* MVL_openWrite */


static fvoid *MVL_openAppend(dvoid *opaque, const char *name)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, NULL);
} /* MVL_openAppend */


static int MVL_remove(dvoid *opaque, const char *name)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, 0);
} /* MVL_remove */


static int MVL_mkdir(dvoid *opaque, const char *name)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, 0);
} /* MVL_mkdir */


const PHYSFS_ArchiveInfo __PHYSFS_ArchiveInfo_MVL =
{
    "MVL",
    MVL_ARCHIVE_DESCRIPTION,
    "Bradley Bell <btb@icculus.org>",
    "http://icculus.org/physfs/",
};


const PHYSFS_Archiver __PHYSFS_Archiver_MVL =
{
    &__PHYSFS_ArchiveInfo_MVL,
    MVL_isArchive,          /* isArchive() method      */
    MVL_openArchive,        /* openArchive() method    */
    MVL_enumerateFiles,     /* enumerateFiles() method */
    MVL_exists,             /* exists() method         */
    MVL_isDirectory,        /* isDirectory() method    */
    MVL_isSymLink,          /* isSymLink() method      */
    MVL_getLastModTime,     /* getLastModTime() method */
    MVL_openRead,           /* openRead() method       */
    MVL_openWrite,          /* openWrite() method      */
    MVL_openAppend,         /* openAppend() method     */
    MVL_remove,             /* remove() method         */
    MVL_mkdir,              /* mkdir() method          */
    MVL_dirClose,           /* dirClose() method       */
    MVL_read,               /* read() method           */
    MVL_write,              /* write() method          */
    MVL_eof,                /* eof() method            */
    MVL_tell,               /* tell() method           */
    MVL_seek,               /* seek() method           */
    MVL_fileLength,         /* fileLength() method     */
    MVL_fileClose           /* fileClose() method      */
};

#endif  /* defined PHYSFS_SUPPORTS_MVL */

/* end of mvl.c ... */

