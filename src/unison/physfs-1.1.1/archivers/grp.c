/*
 * GRP support routines for PhysicsFS.
 *
 * This driver handles BUILD engine archives ("groupfiles"). This format
 *  (but not this driver) was put together by Ken Silverman.
 *
 * The format is simple enough. In Ken's words:
 *
 *    What's the .GRP file format?
 *
 *     The ".grp" file format is just a collection of a lot of files stored
 *     into 1 big one. I tried to make the format as simple as possible: The
 *     first 12 bytes contains my name, "KenSilverman". The next 4 bytes is
 *     the number of files that were compacted into the group file. Then for
 *     each file, there is a 16 byte structure, where the first 12 bytes are
 *     the filename, and the last 4 bytes are the file's size. The rest of
 *     the group file is just the raw data packed one after the other in the
 *     same order as the list of files.
 *
 * (That info is from http://www.advsys.net/ken/build.htm ...)
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#if (defined PHYSFS_SUPPORTS_GRP)

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
} GRPentry;

typedef struct
{
    char *filename;
    PHYSFS_sint64 last_mod_time;
    PHYSFS_uint32 entryCount;
    GRPentry *entries;
} GRPinfo;

typedef struct
{
    void *handle;
    GRPentry *entry;
    PHYSFS_uint32 curPos;
} GRPfileinfo;


static void GRP_dirClose(dvoid *opaque)
{
    GRPinfo *info = ((GRPinfo *) opaque);
    allocator.Free(info->filename);
    allocator.Free(info->entries);
    allocator.Free(info);
} /* GRP_dirClose */


static PHYSFS_sint64 GRP_read(fvoid *opaque, void *buffer,
                              PHYSFS_uint32 objSize, PHYSFS_uint32 objCount)
{
    GRPfileinfo *finfo = (GRPfileinfo *) opaque;
    GRPentry *entry = finfo->entry;
    PHYSFS_uint32 bytesLeft = entry->size - finfo->curPos;
    PHYSFS_uint32 objsLeft = (bytesLeft / objSize);
    PHYSFS_sint64 rc;

    if (objsLeft < objCount)
        objCount = objsLeft;

    rc = __PHYSFS_platformRead(finfo->handle, buffer, objSize, objCount);
    if (rc > 0)
        finfo->curPos += (PHYSFS_uint32) (rc * objSize);

    return(rc);
} /* GRP_read */


static PHYSFS_sint64 GRP_write(fvoid *opaque, const void *buffer,
                               PHYSFS_uint32 objSize, PHYSFS_uint32 objCount)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, -1);
} /* GRP_write */


static int GRP_eof(fvoid *opaque)
{
    GRPfileinfo *finfo = (GRPfileinfo *) opaque;
    GRPentry *entry = finfo->entry;
    return(finfo->curPos >= entry->size);
} /* GRP_eof */


static PHYSFS_sint64 GRP_tell(fvoid *opaque)
{
    return(((GRPfileinfo *) opaque)->curPos);
} /* GRP_tell */


static int GRP_seek(fvoid *opaque, PHYSFS_uint64 offset)
{
    GRPfileinfo *finfo = (GRPfileinfo *) opaque;
    GRPentry *entry = finfo->entry;
    int rc;

    BAIL_IF_MACRO(offset < 0, ERR_INVALID_ARGUMENT, 0);
    BAIL_IF_MACRO(offset >= entry->size, ERR_PAST_EOF, 0);
    rc = __PHYSFS_platformSeek(finfo->handle, entry->startPos + offset);
    if (rc)
        finfo->curPos = (PHYSFS_uint32) offset;

    return(rc);
} /* GRP_seek */


static PHYSFS_sint64 GRP_fileLength(fvoid *opaque)
{
    GRPfileinfo *finfo = (GRPfileinfo *) opaque;
    return((PHYSFS_sint64) finfo->entry->size);
} /* GRP_fileLength */


static int GRP_fileClose(fvoid *opaque)
{
    GRPfileinfo *finfo = (GRPfileinfo *) opaque;
    BAIL_IF_MACRO(!__PHYSFS_platformClose(finfo->handle), NULL, 0);
    allocator.Free(finfo);
    return(1);
} /* GRP_fileClose */


static int grp_open(const char *filename, int forWriting,
                    void **fh, PHYSFS_uint32 *count)
{
    PHYSFS_uint8 buf[12];

    *fh = NULL;
    BAIL_IF_MACRO(forWriting, ERR_ARC_IS_READ_ONLY, 0);

    *fh = __PHYSFS_platformOpenRead(filename);
    BAIL_IF_MACRO(*fh == NULL, NULL, 0);
    
    if (__PHYSFS_platformRead(*fh, buf, 12, 1) != 1)
        goto openGrp_failed;

    if (memcmp(buf, "KenSilverman", 12) != 0)
    {
        __PHYSFS_setError(ERR_UNSUPPORTED_ARCHIVE);
        goto openGrp_failed;
    } /* if */

    if (__PHYSFS_platformRead(*fh, count, sizeof (PHYSFS_uint32), 1) != 1)
        goto openGrp_failed;

    *count = PHYSFS_swapULE32(*count);

    return(1);

openGrp_failed:
    if (*fh != NULL)
        __PHYSFS_platformClose(*fh);

    *count = -1;
    *fh = NULL;
    return(0);
} /* grp_open */


static int GRP_isArchive(const char *filename, int forWriting)
{
    void *fh;
    PHYSFS_uint32 fileCount;
    int retval = grp_open(filename, forWriting, &fh, &fileCount);

    if (fh != NULL)
        __PHYSFS_platformClose(fh);

    return(retval);
} /* GRP_isArchive */


static int grp_entry_cmp(void *_a, PHYSFS_uint32 one, PHYSFS_uint32 two)
{
    GRPentry *a = (GRPentry *) _a;
    return(strcmp(a[one].name, a[two].name));
} /* grp_entry_cmp */


static void grp_entry_swap(void *_a, PHYSFS_uint32 one, PHYSFS_uint32 two)
{
    GRPentry tmp;
    GRPentry *first = &(((GRPentry *) _a)[one]);
    GRPentry *second = &(((GRPentry *) _a)[two]);
    memcpy(&tmp, first, sizeof (GRPentry));
    memcpy(first, second, sizeof (GRPentry));
    memcpy(second, &tmp, sizeof (GRPentry));
} /* grp_entry_swap */


static int grp_load_entries(const char *name, int forWriting, GRPinfo *info)
{
    void *fh = NULL;
    PHYSFS_uint32 fileCount;
    PHYSFS_uint32 location = 16;  /* sizeof sig. */
    GRPentry *entry;
    char *ptr;

    BAIL_IF_MACRO(!grp_open(name, forWriting, &fh, &fileCount), NULL, 0);
    info->entryCount = fileCount;
    info->entries = (GRPentry *) allocator.Malloc(sizeof(GRPentry)*fileCount);
    if (info->entries == NULL)
    {
        __PHYSFS_platformClose(fh);
        BAIL_MACRO(ERR_OUT_OF_MEMORY, 0);
    } /* if */

    location += (16 * fileCount);

    for (entry = info->entries; fileCount > 0; fileCount--, entry++)
    {
        if (__PHYSFS_platformRead(fh, &entry->name, 12, 1) != 1)
        {
            __PHYSFS_platformClose(fh);
            return(0);
        } /* if */

        entry->name[12] = '\0';  /* name isn't null-terminated in file. */
        if ((ptr = strchr(entry->name, ' ')) != NULL)
            *ptr = '\0';  /* trim extra spaces. */

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
                  grp_entry_cmp, grp_entry_swap);
    return(1);
} /* grp_load_entries */


static void *GRP_openArchive(const char *name, int forWriting)
{
    PHYSFS_sint64 modtime = __PHYSFS_platformGetLastModTime(name);
    GRPinfo *info = (GRPinfo *) allocator.Malloc(sizeof (GRPinfo));

    BAIL_IF_MACRO(info == NULL, ERR_OUT_OF_MEMORY, 0);

    memset(info, '\0', sizeof (GRPinfo));
    info->filename = (char *) allocator.Malloc(strlen(name) + 1);
    GOTO_IF_MACRO(!info->filename, ERR_OUT_OF_MEMORY, GRP_openArchive_failed);

    if (!grp_load_entries(name, forWriting, info))
        goto GRP_openArchive_failed;

    strcpy(info->filename, name);
    info->last_mod_time = modtime;

    return(info);

GRP_openArchive_failed:
    if (info != NULL)
    {
        if (info->filename != NULL)
            allocator.Free(info->filename);
        if (info->entries != NULL)
            allocator.Free(info->entries);
        allocator.Free(info);
    } /* if */

    return(NULL);
} /* GRP_openArchive */


static void GRP_enumerateFiles(dvoid *opaque, const char *dname,
                               int omitSymLinks, PHYSFS_EnumFilesCallback cb,
                               const char *origdir, void *callbackdata)
{
    /* no directories in GRP files. */
    if (*dname == '\0')
    {
        GRPinfo *info = (GRPinfo *) opaque;
        GRPentry *entry = info->entries;
        PHYSFS_uint32 max = info->entryCount;
        PHYSFS_uint32 i;

        for (i = 0; i < max; i++, entry++)
            cb(callbackdata, origdir, entry->name);
    } /* if */
} /* GRP_enumerateFiles */


static GRPentry *grp_find_entry(GRPinfo *info, const char *name)
{
    char *ptr = strchr(name, '.');
    GRPentry *a = info->entries;
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
        rc = strcmp(name, a[middle].name);
        if (rc == 0)  /* found it! */
            return(&a[middle]);
        else if (rc > 0)
            lo = middle + 1;
        else
            hi = middle - 1;
    } /* while */

    BAIL_MACRO(ERR_NO_SUCH_FILE, NULL);
} /* grp_find_entry */


static int GRP_exists(dvoid *opaque, const char *name)
{
    return(grp_find_entry((GRPinfo *) opaque, name) != NULL);
} /* GRP_exists */


static int GRP_isDirectory(dvoid *opaque, const char *name, int *fileExists)
{
    *fileExists = GRP_exists(opaque, name);
    return(0);  /* never directories in a groupfile. */
} /* GRP_isDirectory */


static int GRP_isSymLink(dvoid *opaque, const char *name, int *fileExists)
{
    *fileExists = GRP_exists(opaque, name);
    return(0);  /* never symlinks in a groupfile. */
} /* GRP_isSymLink */


static PHYSFS_sint64 GRP_getLastModTime(dvoid *opaque,
                                        const char *name,
                                        int *fileExists)
{
    GRPinfo *info = (GRPinfo *) opaque;
    PHYSFS_sint64 retval = -1;

    *fileExists = (grp_find_entry(info, name) != NULL);
    if (*fileExists)  /* use time of GRP itself in the physical filesystem. */
        retval = info->last_mod_time;

    return(retval);
} /* GRP_getLastModTime */


static fvoid *GRP_openRead(dvoid *opaque, const char *fnm, int *fileExists)
{
    GRPinfo *info = (GRPinfo *) opaque;
    GRPfileinfo *finfo;
    GRPentry *entry;

    entry = grp_find_entry(info, fnm);
    *fileExists = (entry != NULL);
    BAIL_IF_MACRO(entry == NULL, NULL, NULL);

    finfo = (GRPfileinfo *) allocator.Malloc(sizeof (GRPfileinfo));
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
} /* GRP_openRead */


static fvoid *GRP_openWrite(dvoid *opaque, const char *name)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, NULL);
} /* GRP_openWrite */


static fvoid *GRP_openAppend(dvoid *opaque, const char *name)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, NULL);
} /* GRP_openAppend */


static int GRP_remove(dvoid *opaque, const char *name)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, 0);
} /* GRP_remove */


static int GRP_mkdir(dvoid *opaque, const char *name)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, 0);
} /* GRP_mkdir */


const PHYSFS_ArchiveInfo __PHYSFS_ArchiveInfo_GRP =
{
    "GRP",
    GRP_ARCHIVE_DESCRIPTION,
    "Ryan C. Gordon <icculus@icculus.org>",
    "http://icculus.org/physfs/",
};


const PHYSFS_Archiver __PHYSFS_Archiver_GRP =
{
    &__PHYSFS_ArchiveInfo_GRP,
    GRP_isArchive,          /* isArchive() method      */
    GRP_openArchive,        /* openArchive() method    */
    GRP_enumerateFiles,     /* enumerateFiles() method */
    GRP_exists,             /* exists() method         */
    GRP_isDirectory,        /* isDirectory() method    */
    GRP_isSymLink,          /* isSymLink() method      */
    GRP_getLastModTime,     /* getLastModTime() method */
    GRP_openRead,           /* openRead() method       */
    GRP_openWrite,          /* openWrite() method      */
    GRP_openAppend,         /* openAppend() method     */
    GRP_remove,             /* remove() method         */
    GRP_mkdir,              /* mkdir() method          */
    GRP_dirClose,           /* dirClose() method       */
    GRP_read,               /* read() method           */
    GRP_write,              /* write() method          */
    GRP_eof,                /* eof() method            */
    GRP_tell,               /* tell() method           */
    GRP_seek,               /* seek() method           */
    GRP_fileLength,         /* fileLength() method     */
    GRP_fileClose           /* fileClose() method      */
};

#endif  /* defined PHYSFS_SUPPORTS_GRP */

/* end of grp.c ... */

