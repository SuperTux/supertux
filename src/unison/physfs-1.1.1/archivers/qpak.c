/*
 * QPAK support routines for PhysicsFS.
 *
 *  This archiver handles the archive format utilized by Quake 1 and 2.
 *  Quake3-based games use the PkZip/Info-Zip format (which our zip.c
 *  archiver handles).
 *
 *  ========================================================================
 *
 *  This format info (in more detail) comes from:
 *     http://debian.fmi.uni-sofia.bg/~sergei/cgsr/docs/pak.txt
 *
 *  Quake PAK Format
 *
 *  Header
 *   (4 bytes)  signature = 'PACK'
 *   (4 bytes)  directory offset
 *   (4 bytes)  directory length
 *
 *  Directory
 *   (56 bytes) file name
 *   (4 bytes)  file position
 *   (4 bytes)  file length
 *
 *  ========================================================================
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#if (defined PHYSFS_SUPPORTS_QPAK)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "physfs.h"

#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

#if 1  /* Make this case insensitive? */
#define QPAK_strcmp(x, y) __PHYSFS_stricmpASCII(x, y)
#define QPAK_strncmp(x, y, z) __PHYSFS_strnicmpASCII(x, y, z)
#else
#define QPAK_strcmp(x, y) strcmp(x, y)
#define QPAK_strncmp(x, y, z) strncmp(x, y, z)
#endif


typedef struct
{
    char name[56];
    PHYSFS_uint32 startPos;
    PHYSFS_uint32 size;
} QPAKentry;

typedef struct
{
    char *filename;
    PHYSFS_sint64 last_mod_time;
    PHYSFS_uint32 entryCount;
    QPAKentry *entries;
} QPAKinfo;

typedef struct
{
    void *handle;
    QPAKentry *entry;
    PHYSFS_uint32 curPos;
} QPAKfileinfo;

/* Magic numbers... */
#define QPAK_SIG 0x4b434150   /* "PACK" in ASCII. */


static void QPAK_dirClose(dvoid *opaque)
{
    QPAKinfo *info = ((QPAKinfo *) opaque);
    allocator.Free(info->filename);
    allocator.Free(info->entries);
    allocator.Free(info);
} /* QPAK_dirClose */


static PHYSFS_sint64 QPAK_read(fvoid *opaque, void *buffer,
                              PHYSFS_uint32 objSize, PHYSFS_uint32 objCount)
{
    QPAKfileinfo *finfo = (QPAKfileinfo *) opaque;
    QPAKentry *entry = finfo->entry;
    PHYSFS_uint32 bytesLeft = entry->size - finfo->curPos;
    PHYSFS_uint32 objsLeft = (bytesLeft / objSize);
    PHYSFS_sint64 rc;

    if (objsLeft < objCount)
        objCount = objsLeft;

    rc = __PHYSFS_platformRead(finfo->handle, buffer, objSize, objCount);
    if (rc > 0)
        finfo->curPos += (PHYSFS_uint32) (rc * objSize);

    return(rc);
} /* QPAK_read */


static PHYSFS_sint64 QPAK_write(fvoid *opaque, const void *buffer,
                               PHYSFS_uint32 objSize, PHYSFS_uint32 objCount)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, -1);
} /* QPAK_write */


static int QPAK_eof(fvoid *opaque)
{
    QPAKfileinfo *finfo = (QPAKfileinfo *) opaque;
    QPAKentry *entry = finfo->entry;
    return(finfo->curPos >= entry->size);
} /* QPAK_eof */


static PHYSFS_sint64 QPAK_tell(fvoid *opaque)
{
    return(((QPAKfileinfo *) opaque)->curPos);
} /* QPAK_tell */


static int QPAK_seek(fvoid *opaque, PHYSFS_uint64 offset)
{
    QPAKfileinfo *finfo = (QPAKfileinfo *) opaque;
    QPAKentry *entry = finfo->entry;
    int rc;

    BAIL_IF_MACRO(offset < 0, ERR_INVALID_ARGUMENT, 0);
    BAIL_IF_MACRO(offset >= entry->size, ERR_PAST_EOF, 0);
    rc = __PHYSFS_platformSeek(finfo->handle, entry->startPos + offset);
    if (rc)
        finfo->curPos = (PHYSFS_uint32) offset;

    return(rc);
} /* QPAK_seek */


static PHYSFS_sint64 QPAK_fileLength(fvoid *opaque)
{
    QPAKfileinfo *finfo = (QPAKfileinfo *) opaque;
    return((PHYSFS_sint64) finfo->entry->size);
} /* QPAK_fileLength */


static int QPAK_fileClose(fvoid *opaque)
{
    QPAKfileinfo *finfo = (QPAKfileinfo *) opaque;
    BAIL_IF_MACRO(!__PHYSFS_platformClose(finfo->handle), NULL, 0);
    allocator.Free(finfo);
    return(1);
} /* QPAK_fileClose */


static int qpak_open(const char *filename, int forWriting,
                    void **fh, PHYSFS_uint32 *count)
{
    PHYSFS_uint32 buf;

    *fh = NULL;
    BAIL_IF_MACRO(forWriting, ERR_ARC_IS_READ_ONLY, 0);

    *fh = __PHYSFS_platformOpenRead(filename);
    BAIL_IF_MACRO(*fh == NULL, NULL, 0);
    
    if (__PHYSFS_platformRead(*fh, &buf, sizeof (PHYSFS_uint32), 1) != 1)
        goto openQpak_failed;

    buf = PHYSFS_swapULE32(buf);
    GOTO_IF_MACRO(buf != QPAK_SIG, ERR_UNSUPPORTED_ARCHIVE, openQpak_failed);

    if (__PHYSFS_platformRead(*fh, &buf, sizeof (PHYSFS_uint32), 1) != 1)
        goto openQpak_failed;

    buf = PHYSFS_swapULE32(buf);  /* directory table offset. */

    if (__PHYSFS_platformRead(*fh, count, sizeof (PHYSFS_uint32), 1) != 1)
        goto openQpak_failed;

    *count = PHYSFS_swapULE32(*count);

    /* corrupted archive? */
    GOTO_IF_MACRO((*count % 64) != 0, ERR_CORRUPTED, openQpak_failed);

    if (!__PHYSFS_platformSeek(*fh, buf))
        goto openQpak_failed;

    *count /= 64;
    return(1);

openQpak_failed:
    if (*fh != NULL)
        __PHYSFS_platformClose(*fh);

    *count = -1;
    *fh = NULL;
    return(0);
} /* qpak_open */


static int QPAK_isArchive(const char *filename, int forWriting)
{
    void *fh;
    PHYSFS_uint32 fileCount;
    int retval = qpak_open(filename, forWriting, &fh, &fileCount);

    if (fh != NULL)
        __PHYSFS_platformClose(fh);

    return(retval);
} /* QPAK_isArchive */


static int qpak_entry_cmp(void *_a, PHYSFS_uint32 one, PHYSFS_uint32 two)
{
    QPAKentry *a = (QPAKentry *) _a;
    return(QPAK_strcmp(a[one].name, a[two].name));
} /* qpak_entry_cmp */


static void qpak_entry_swap(void *_a, PHYSFS_uint32 one, PHYSFS_uint32 two)
{
    QPAKentry tmp;
    QPAKentry *first = &(((QPAKentry *) _a)[one]);
    QPAKentry *second = &(((QPAKentry *) _a)[two]);
    memcpy(&tmp, first, sizeof (QPAKentry));
    memcpy(first, second, sizeof (QPAKentry));
    memcpy(second, &tmp, sizeof (QPAKentry));
} /* qpak_entry_swap */


static int qpak_load_entries(const char *name, int forWriting, QPAKinfo *info)
{
    void *fh = NULL;
    PHYSFS_uint32 fileCount;
    QPAKentry *entry;

    BAIL_IF_MACRO(!qpak_open(name, forWriting, &fh, &fileCount), NULL, 0);
    info->entryCount = fileCount;
    info->entries = (QPAKentry*) allocator.Malloc(sizeof(QPAKentry)*fileCount);
    if (info->entries == NULL)
    {
        __PHYSFS_platformClose(fh);
        BAIL_MACRO(ERR_OUT_OF_MEMORY, 0);
    } /* if */

    for (entry = info->entries; fileCount > 0; fileCount--, entry++)
    {
        PHYSFS_uint32 loc;

        if (__PHYSFS_platformRead(fh,&entry->name,sizeof(entry->name),1) != 1)
        {
            __PHYSFS_platformClose(fh);
            return(0);
        } /* if */

        if (__PHYSFS_platformRead(fh,&loc,sizeof(loc),1) != 1)
        {
            __PHYSFS_platformClose(fh);
            return(0);
        } /* if */

        if (__PHYSFS_platformRead(fh,&entry->size,sizeof(entry->size),1) != 1)
        {
            __PHYSFS_platformClose(fh);
            return(0);
        } /* if */

        entry->size = PHYSFS_swapULE32(entry->size);
        entry->startPos = PHYSFS_swapULE32(loc);
    } /* for */

    __PHYSFS_platformClose(fh);

    __PHYSFS_sort(info->entries, info->entryCount,
                  qpak_entry_cmp, qpak_entry_swap);
    return(1);
} /* qpak_load_entries */


static void *QPAK_openArchive(const char *name, int forWriting)
{
    QPAKinfo *info = (QPAKinfo *) allocator.Malloc(sizeof (QPAKinfo));
    PHYSFS_sint64 modtime = __PHYSFS_platformGetLastModTime(name);

    BAIL_IF_MACRO(info == NULL, ERR_OUT_OF_MEMORY, NULL);
    memset(info, '\0', sizeof (QPAKinfo));

    info->filename = (char *) allocator.Malloc(strlen(name) + 1);
    if (info->filename == NULL)
    {
        __PHYSFS_setError(ERR_OUT_OF_MEMORY);
        goto QPAK_openArchive_failed;
    } /* if */

    if (!qpak_load_entries(name, forWriting, info))
        goto QPAK_openArchive_failed;

    strcpy(info->filename, name);
    info->last_mod_time = modtime;
    return(info);

QPAK_openArchive_failed:
    if (info != NULL)
    {
        if (info->filename != NULL)
            allocator.Free(info->filename);
        if (info->entries != NULL)
            allocator.Free(info->entries);
        allocator.Free(info);
    } /* if */

    return(NULL);
} /* QPAK_openArchive */


static PHYSFS_sint32 qpak_find_start_of_dir(QPAKinfo *info, const char *path,
                                            int stop_on_first_find)
{
    PHYSFS_sint32 lo = 0;
    PHYSFS_sint32 hi = (PHYSFS_sint32) (info->entryCount - 1);
    PHYSFS_sint32 middle;
    PHYSFS_uint32 dlen = strlen(path);
    PHYSFS_sint32 retval = -1;
    const char *name;
    int rc;

    if (*path == '\0')  /* root dir? */
        return(0);

    if ((dlen > 0) && (path[dlen - 1] == '/')) /* ignore trailing slash. */
        dlen--;

    while (lo <= hi)
    {
        middle = lo + ((hi - lo) / 2);
        name = info->entries[middle].name;
        rc = QPAK_strncmp(path, name, dlen);
        if (rc == 0)
        {
            char ch = name[dlen];
            if (ch < '/') /* make sure this isn't just a substr match. */
                rc = -1;
            else if (ch > '/')
                rc = 1;
            else 
            {
                if (stop_on_first_find) /* Just checking dir's existance? */
                    return(middle);

                if (name[dlen + 1] == '\0') /* Skip initial dir entry. */
                    return(middle + 1);

                /* there might be more entries earlier in the list. */
                retval = middle;
                hi = middle - 1;
            } /* else */
        } /* if */

        if (rc > 0)
            lo = middle + 1;
        else
            hi = middle - 1;
    } /* while */

    return(retval);
} /* qpak_find_start_of_dir */


/*
 * Moved to seperate function so we can use alloca then immediately throw
 *  away the allocated stack space...
 */
static void doEnumCallback(PHYSFS_EnumFilesCallback cb, void *callbackdata,
                           const char *odir, const char *str, PHYSFS_sint32 ln)
{
    char *newstr = __PHYSFS_smallAlloc(ln + 1);
    if (newstr == NULL)
        return;

    memcpy(newstr, str, ln);
    newstr[ln] = '\0';
    cb(callbackdata, odir, newstr);
    __PHYSFS_smallFree(newstr);
} /* doEnumCallback */


static void QPAK_enumerateFiles(dvoid *opaque, const char *dname,
                                int omitSymLinks, PHYSFS_EnumFilesCallback cb,
                                const char *origdir, void *callbackdata)
{
    QPAKinfo *info = ((QPAKinfo *) opaque);
    PHYSFS_sint32 dlen, dlen_inc, max, i;

    i = qpak_find_start_of_dir(info, dname, 0);
    if (i == -1)  /* no such directory. */
        return;

    dlen = strlen(dname);
    if ((dlen > 0) && (dname[dlen - 1] == '/')) /* ignore trailing slash. */
        dlen--;

    dlen_inc = ((dlen > 0) ? 1 : 0) + dlen;
    max = (PHYSFS_sint32) info->entryCount;
    while (i < max)
    {
        char *add;
        char *ptr;
        PHYSFS_sint32 ln;
        char *e = info->entries[i].name;
        if ((dlen) && ((QPAK_strncmp(e, dname, dlen)) || (e[dlen] != '/')))
            break;  /* past end of this dir; we're done. */

        add = e + dlen_inc;
        ptr = strchr(add, '/');
        ln = (PHYSFS_sint32) ((ptr) ? ptr-add : strlen(add));
        doEnumCallback(cb, callbackdata, origdir, add, ln);
        ln += dlen_inc;  /* point past entry to children... */

        /* increment counter and skip children of subdirs... */
        while ((++i < max) && (ptr != NULL))
        {
            char *e_new = info->entries[i].name;
            if ((QPAK_strncmp(e, e_new, ln) != 0) || (e_new[ln] != '/'))
                break;
        } /* while */
    } /* while */
} /* QPAK_enumerateFiles */


/*
 * This will find the QPAKentry associated with a path in platform-independent
 *  notation. Directories don't have QPAKentries associated with them, but 
 *  (*isDir) will be set to non-zero if a dir was hit.
 */
static QPAKentry *qpak_find_entry(QPAKinfo *info, const char *path, int *isDir)
{
    QPAKentry *a = info->entries;
    PHYSFS_sint32 pathlen = strlen(path);
    PHYSFS_sint32 lo = 0;
    PHYSFS_sint32 hi = (PHYSFS_sint32) (info->entryCount - 1);
    PHYSFS_sint32 middle;
    const char *thispath = NULL;
    int rc;

    while (lo <= hi)
    {
        middle = lo + ((hi - lo) / 2);
        thispath = a[middle].name;
        rc = QPAK_strncmp(path, thispath, pathlen);

        if (rc > 0)
            lo = middle + 1;

        else if (rc < 0)
            hi = middle - 1;

        else /* substring match...might be dir or entry or nothing. */
        {
            if (isDir != NULL)
            {
                *isDir = (thispath[pathlen] == '/');
                if (*isDir)
                    return(NULL);
            } /* if */

            if (thispath[pathlen] == '\0') /* found entry? */
                return(&a[middle]);
            else
                hi = middle - 1;  /* adjust search params, try again. */
        } /* if */
    } /* while */

    if (isDir != NULL)
        *isDir = 0;

    BAIL_MACRO(ERR_NO_SUCH_FILE, NULL);
} /* qpak_find_entry */


static int QPAK_exists(dvoid *opaque, const char *name)
{
    int isDir;    
    QPAKinfo *info = (QPAKinfo *) opaque;
    QPAKentry *entry = qpak_find_entry(info, name, &isDir);
    return((entry != NULL) || (isDir));
} /* QPAK_exists */


static int QPAK_isDirectory(dvoid *opaque, const char *name, int *fileExists)
{
    QPAKinfo *info = (QPAKinfo *) opaque;
    int isDir;
    QPAKentry *entry = qpak_find_entry(info, name, &isDir);

    *fileExists = ((isDir) || (entry != NULL));
    if (isDir)
        return(1); /* definitely a dir. */

    BAIL_MACRO(ERR_NO_SUCH_FILE, 0);
} /* QPAK_isDirectory */


static int QPAK_isSymLink(dvoid *opaque, const char *name, int *fileExists)
{
    *fileExists = QPAK_exists(opaque, name);
    return(0);  /* never symlinks in a quake pak. */
} /* QPAK_isSymLink */


static PHYSFS_sint64 QPAK_getLastModTime(dvoid *opaque,
                                        const char *name,
                                        int *fileExists)
{
    int isDir;
    QPAKinfo *info = ((QPAKinfo *) opaque);
    PHYSFS_sint64 retval = -1;
    QPAKentry *entry = qpak_find_entry(info, name, &isDir);

    *fileExists = ((isDir) || (entry != NULL));
    if (*fileExists)  /* use time of QPAK itself in the physical filesystem. */
        retval = info->last_mod_time;

    return(retval);
} /* QPAK_getLastModTime */


static fvoid *QPAK_openRead(dvoid *opaque, const char *fnm, int *fileExists)
{
    QPAKinfo *info = ((QPAKinfo *) opaque);
    QPAKfileinfo *finfo;
    QPAKentry *entry;
    int isDir;

    entry = qpak_find_entry(info, fnm, &isDir);
    *fileExists = ((entry != NULL) || (isDir));
    BAIL_IF_MACRO(isDir, ERR_NOT_A_FILE, NULL);
    BAIL_IF_MACRO(entry == NULL, ERR_NO_SUCH_FILE, NULL);

    finfo = (QPAKfileinfo *) allocator.Malloc(sizeof (QPAKfileinfo));
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
} /* QPAK_openRead */


static fvoid *QPAK_openWrite(dvoid *opaque, const char *name)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, NULL);
} /* QPAK_openWrite */


static fvoid *QPAK_openAppend(dvoid *opaque, const char *name)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, NULL);
} /* QPAK_openAppend */


static int QPAK_remove(dvoid *opaque, const char *name)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, 0);
} /* QPAK_remove */


static int QPAK_mkdir(dvoid *opaque, const char *name)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, 0);
} /* QPAK_mkdir */


const PHYSFS_ArchiveInfo __PHYSFS_ArchiveInfo_QPAK =
{
    "PAK",
    QPAK_ARCHIVE_DESCRIPTION,
    "Ryan C. Gordon <icculus@icculus.org>",
    "http://icculus.org/physfs/",
};


const PHYSFS_Archiver __PHYSFS_Archiver_QPAK =
{
    &__PHYSFS_ArchiveInfo_QPAK,
    QPAK_isArchive,          /* isArchive() method      */
    QPAK_openArchive,        /* openArchive() method    */
    QPAK_enumerateFiles,     /* enumerateFiles() method */
    QPAK_exists,             /* exists() method         */
    QPAK_isDirectory,        /* isDirectory() method    */
    QPAK_isSymLink,          /* isSymLink() method      */
    QPAK_getLastModTime,     /* getLastModTime() method */
    QPAK_openRead,           /* openRead() method       */
    QPAK_openWrite,          /* openWrite() method      */
    QPAK_openAppend,         /* openAppend() method     */
    QPAK_remove,             /* remove() method         */
    QPAK_mkdir,              /* mkdir() method          */
    QPAK_dirClose,           /* dirClose() method       */
    QPAK_read,               /* read() method           */
    QPAK_write,              /* write() method          */
    QPAK_eof,                /* eof() method            */
    QPAK_tell,               /* tell() method           */
    QPAK_seek,               /* seek() method           */
    QPAK_fileLength,         /* fileLength() method     */
    QPAK_fileClose           /* fileClose() method      */
};

#endif  /* defined PHYSFS_SUPPORTS_QPAK */

/* end of qpak.c ... */

