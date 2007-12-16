/*
 * LZMA support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file is written by Dennis Schridde, with some peeking at "7zMain.c"
 *   by Igor Pavlov.
 */

#if (defined PHYSFS_SUPPORTS_7Z)

#include <stdlib.h>
#include <string.h>

#include "physfs.h"

#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

#ifndef _LZMA_IN_CB
#define _LZMA_IN_CB
/* Use callback for input data */
#endif

/* #define _LZMA_OUT_READ */
/* Use read function for output data */

#ifndef _LZMA_PROB32
#define _LZMA_PROB32
/* It can increase speed on some 32-bit CPUs,
   but memory usage will be doubled in that case */
#endif

#ifndef _LZMA_SYSTEM_SIZE_T
#define _LZMA_SYSTEM_SIZE_T
/* Use system's size_t. You can use it to enable 64-bit sizes supporting */
#endif

#include "7zIn.h"
#include "7zCrc.h"
#include "7zExtract.h"


/* 7z internal from 7zIn.c */
int TestSignatureCandidate(Byte *testBytes);


typedef struct _CFileInStream
{
    ISzInStream InStream;
    void *File;
} CFileInStream;

/*
 * In LZMA the archive is splited in blocks, those are called folders
 * Set by LZMA_read()
*/
typedef struct _LZMAfolder
{
    PHYSFS_uint8 *cache; /* Cached folder */
    PHYSFS_uint32 size; /* Size of folder */
    PHYSFS_uint32 index; /* Index of folder in archive */
    PHYSFS_uint32 references; /* Number of files using this block */
} LZMAfolder;

/*
 * Set by LZMA_openArchive(), except folder which gets it's values
 *  in LZMA_read()
 */
typedef struct _LZMAarchive
{
    struct _LZMAentry *firstEntry; /* Used for cleanup on shutdown */
    struct _LZMAentry *lastEntry;
    LZMAfolder *folder; /* Array of folders */
    CArchiveDatabaseEx db; /* For 7z: Database */
    CFileInStream stream; /* For 7z: Input file incl. read and seek callbacks */
} LZMAarchive;

/* Set by LZMA_openRead(), except offset which is set by LZMA_read() */
typedef struct _LZMAentry
{
    struct _LZMAentry *next; /* Used for cleanup on shutdown */
    struct _LZMAentry *previous;
    LZMAarchive *archive; /* Link to corresponding archive */
    CFileItem *file; /* For 7z: File info, eg. name, size */
    PHYSFS_uint32 fileIndex; /* Index of file in archive */
    PHYSFS_uint32 folderIndex; /* Index of folder in archive */
    size_t offset; /* Offset in folder */
    PHYSFS_uint64 position; /* Current "virtual" position in file */
} LZMAentry;


/* Memory management implementations to be passed to 7z */

static void *SzAllocPhysicsFS(size_t size)
{
    return ((size == 0) ? NULL : allocator.Malloc(size));
} /* SzAllocPhysicsFS */


static void SzFreePhysicsFS(void *address)
{
    if (address != NULL)
        allocator.Free(address);
} /* SzFreePhysicsFS */


/* Filesystem implementations to be passed to 7z */

#ifdef _LZMA_IN_CB

#define kBufferSize (1 << 12)
static Byte g_Buffer[kBufferSize];  /* !!! FIXME: not thread safe! */

SZ_RESULT SzFileReadImp(void *object, void **buffer, size_t maxReqSize,
                        size_t *processedSize)
{
    CFileInStream *s = (CFileInStream *)object;
    PHYSFS_sint64 processedSizeLoc;
    if (maxReqSize > kBufferSize)
        maxReqSize = kBufferSize;
    processedSizeLoc = __PHYSFS_platformRead(s->File, g_Buffer, 1, maxReqSize);
    *buffer = g_Buffer;
    if (processedSize != NULL)
        *processedSize = (size_t) processedSizeLoc;
    return SZ_OK;
} /* SzFileReadImp */

#else

SZ_RESULT SzFileReadImp(void *object, void *buffer, size_t size,
                        size_t *processedSize)
{
    CFileInStream *s = (CFileInStream *)object;
    size_t processedSizeLoc = __PHYSFS_platformRead(s->File, buffer, 1, size);
    if (processedSize != 0)
        *processedSize = processedSizeLoc;
    return SZ_OK;
} /* SzFileReadImp */

#endif

SZ_RESULT SzFileSeekImp(void *object, CFileSize pos)
{
    CFileInStream *s = (CFileInStream *) object;
    if (__PHYSFS_platformSeek(s->File, (PHYSFS_uint64) pos))
        return SZ_OK;
    return SZE_FAIL;
} /* SzFileSeekImp */


/*
 * Find entry 'name' in 'archive' and report the 'index' back
 */
static int lzma_find_entry(LZMAarchive *archive, const char *name,
                           PHYSFS_uint32 *index)
{
    for (*index = 0; *index < archive->db.Database.NumFiles; (*index)++)
    {
        if (strcmp(archive->db.Database.Files[*index].Name, name) == 0)
            return 1;
    } /* for */

    BAIL_MACRO(ERR_NO_SUCH_FILE, 0);
} /* lzma_find_entry */


/*
 * Report the first file index of a directory
 */
static PHYSFS_sint32 lzma_find_start_of_dir(LZMAarchive *archive,
                                            const char *path,
                                            int stop_on_first_find)
{
    PHYSFS_sint32 lo = 0;
    PHYSFS_sint32 hi = (PHYSFS_sint32) (archive->db.Database.NumFiles - 1);
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
        name = archive->db.Database.Files[middle].Name;
        rc = strncmp(path, name, dlen);
        if (rc == 0)
        {
            char ch = name[dlen];
            if ('/' < ch) /* make sure this isn't just a substr match. */
                rc = -1;
            else if ('/' > ch)
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
} /* lzma_find_start_of_dir */


/*
 * Wrap all 7z calls in this, so the physfs error state is set appropriately.
 */
static int lzma_err(SZ_RESULT rc)
{
    switch (rc)
    {
        case SZ_OK: /* Same as LZMA_RESULT_OK */
            break;
        case SZE_DATA_ERROR: /* Same as LZMA_RESULT_DATA_ERROR */
            __PHYSFS_setError(ERR_DATA_ERROR);
            break;
        case SZE_OUTOFMEMORY:
            __PHYSFS_setError(ERR_OUT_OF_MEMORY);
            break;
        case SZE_CRC_ERROR:
            __PHYSFS_setError(ERR_CORRUPTED);
            break;
        case SZE_NOTIMPL:
            __PHYSFS_setError(ERR_NOT_IMPLEMENTED);
            break;
        case SZE_FAIL:
            __PHYSFS_setError(ERR_UNKNOWN_ERROR);  /* !!! FIXME: right? */
            break;
        case SZE_ARCHIVE_ERROR:
            __PHYSFS_setError(ERR_CORRUPTED);  /* !!! FIXME: right? */
            break;
        default:
            __PHYSFS_setError(ERR_UNKNOWN_ERROR);
    } /* switch */

    return(rc);
} /* lzma_err */


static PHYSFS_sint64 LZMA_read(fvoid *opaque, void *outBuffer,
                               PHYSFS_uint32 objSize, PHYSFS_uint32 objCount)
{
    LZMAentry *entry = (LZMAentry *) opaque;

    PHYSFS_sint64 wantedSize = objSize*objCount;
    PHYSFS_sint64 remainingSize = entry->file->Size - entry->position;

    size_t fileSize;
    ISzAlloc allocImp;
    ISzAlloc allocTempImp;

    BAIL_IF_MACRO(wantedSize == 0, NULL, 0); /* quick rejection. */
    BAIL_IF_MACRO(remainingSize == 0, ERR_PAST_EOF, 0);

    if (remainingSize < wantedSize)
    {
        wantedSize = remainingSize - (remainingSize % objSize);
        objCount = (PHYSFS_uint32) (remainingSize / objSize);
        BAIL_IF_MACRO(objCount == 0, ERR_PAST_EOF, 0); /* quick rejection. */
        __PHYSFS_setError(ERR_PAST_EOF); /* this is always true here. */
    } /* if */

    /* Prepare callbacks for 7z */
    allocImp.Alloc = SzAllocPhysicsFS;
    allocImp.Free = SzFreePhysicsFS;

    allocTempImp.Alloc = SzAllocPhysicsFS;
    allocTempImp.Free = SzFreePhysicsFS;

    /* Only decompress the folder if it is not allready cached */
    if (entry->archive->folder[entry->folderIndex].cache == NULL)
    {
        size_t tmpsize = entry->archive->folder[entry->folderIndex].size;
        int rc = lzma_err(SzExtract(
            &entry->archive->stream.InStream, /* compressed data */
            &entry->archive->db,
            entry->fileIndex,
            /* Index of cached folder, will be changed by SzExtract */
            &entry->archive->folder[entry->folderIndex].index,
            /* Cache for decompressed folder, allocated/freed by SzExtract */
            &entry->archive->folder[entry->folderIndex].cache,
            /* Size of cache, will be changed by SzExtract */
            &tmpsize,
            /* Offset of this file inside the cache, set by SzExtract */
            &entry->offset,
            &fileSize, /* Size of this file */
            &allocImp,
            &allocTempImp));

        entry->archive->folder[entry->folderIndex].size = tmpsize;
        if (rc != SZ_OK)
            return -1;
    } /* if */

    /* Copy wanted bytes over from cache to outBuffer */
/* !!! FIXME: strncpy for non-string data? */
	strncpy(outBuffer,
            (void*) (entry->archive->folder[entry->folderIndex].cache +
                     entry->offset + entry->position),
            (size_t) wantedSize);
    entry->position += wantedSize;
    return objCount;
} /* LZMA_read */


static PHYSFS_sint64 LZMA_write(fvoid *opaque, const void *buf,
                               PHYSFS_uint32 objSize, PHYSFS_uint32 objCount)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, -1);
} /* LZMA_write */


static int LZMA_eof(fvoid *opaque)
{
    LZMAentry *entry = (LZMAentry *) opaque;
    return (entry->position >= entry->file->Size);
} /* LZMA_eof */


static PHYSFS_sint64 LZMA_tell(fvoid *opaque)
{
    LZMAentry *entry = (LZMAentry *) opaque;
    return (entry->position);
} /* LZMA_tell */


static int LZMA_seek(fvoid *opaque, PHYSFS_uint64 offset)
{
    LZMAentry *entry = (LZMAentry *) opaque;

    BAIL_IF_MACRO(offset < 0, ERR_SEEK_OUT_OF_RANGE, 0);
    BAIL_IF_MACRO(offset > entry->file->Size, ERR_PAST_EOF, 0);

    entry->position = offset;
    return 1;
} /* LZMA_seek */


static PHYSFS_sint64 LZMA_fileLength(fvoid *opaque)
{
    LZMAentry *entry = (LZMAentry *) opaque;
    return (entry->file->Size);
} /* LZMA_fileLength */


static int LZMA_fileClose(fvoid *opaque)
{
    LZMAentry *entry = (LZMAentry *) opaque;

    /* Fix archive */
    if (entry == entry->archive->firstEntry)
        entry->archive->firstEntry = entry->next;
    if (entry == entry->archive->lastEntry)
        entry->archive->lastEntry = entry->previous;

    /* Fix neighbours */
    if (entry->previous != NULL)
        entry->previous->next = entry->next;
    if (entry->next != NULL)
        entry->next->previous = entry->previous;

    entry->archive->folder[entry->folderIndex].references--;
    if (entry->archive->folder[entry->folderIndex].references == 0)
    {
        allocator.Free(entry->archive->folder[entry->folderIndex].cache);
        entry->archive->folder[entry->folderIndex].cache = NULL;
    }

    allocator.Free(entry);
    entry = NULL;

    return(1);
} /* LZMA_fileClose */


static int LZMA_isArchive(const char *filename, int forWriting)
{
    PHYSFS_uint8 sig[k7zSignatureSize];
    PHYSFS_uint8 res;
    void *in;

    BAIL_IF_MACRO(forWriting, ERR_ARC_IS_READ_ONLY, 0);

    in = __PHYSFS_platformOpenRead(filename);
    BAIL_IF_MACRO(in == NULL, NULL, 0);

    if (__PHYSFS_platformRead(in, sig, k7zSignatureSize, 1) != 1)
        BAIL_MACRO(NULL, 0);

    /* Test whether sig is the 7z signature */
    res = TestSignatureCandidate(sig);

    __PHYSFS_platformClose(in);

    return res;
} /* LZMA_isArchive */


static void *LZMA_openArchive(const char *name, int forWriting)
{
    PHYSFS_uint64 len;
    LZMAarchive *archive = NULL;
    ISzAlloc allocImp;
    ISzAlloc allocTempImp;

    BAIL_IF_MACRO(forWriting, ERR_ARC_IS_READ_ONLY, NULL);
    BAIL_IF_MACRO(!LZMA_isArchive(name,forWriting), ERR_UNSUPPORTED_ARCHIVE, 0);

    archive = (LZMAarchive *) allocator.Malloc(sizeof (LZMAarchive));
    BAIL_IF_MACRO(archive == NULL, ERR_OUT_OF_MEMORY, NULL);

    archive->firstEntry = NULL;
    archive->lastEntry = NULL;

    if ((archive->stream.File = __PHYSFS_platformOpenRead(name)) == NULL)
    {
        allocator.Free(archive);
        return NULL;
    } /* if */

    /* Prepare structs for 7z */
    archive->stream.InStream.Read = SzFileReadImp;
    archive->stream.InStream.Seek = SzFileSeekImp;

    allocImp.Alloc = SzAllocPhysicsFS;
    allocImp.Free = SzFreePhysicsFS;

    allocTempImp.Alloc = SzAllocPhysicsFS;
    allocTempImp.Free = SzFreePhysicsFS;

    InitCrcTable();
    SzArDbExInit(&archive->db);
    if (lzma_err(SzArchiveOpen(&archive->stream.InStream, &archive->db,
                               &allocImp, &allocTempImp)) != SZ_OK)
    {
        __PHYSFS_platformClose(archive->stream.File);
        allocator.Free(archive);
        return NULL;
    } /* if */

    len = archive->db.Database.NumFolders * sizeof (LZMAfolder);
    archive->folder = (LZMAfolder *) allocator.Malloc(len);
    BAIL_IF_MACRO(archive->folder == NULL, ERR_OUT_OF_MEMORY, NULL);

    /*
     * Init with 0 so we know when a folder is already cached
     * Values will be set by LZMA_read()
     */
    memset(archive->folder, 0, (size_t) len);

    return(archive);
} /* LZMA_openArchive */


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


static void LZMA_enumerateFiles(dvoid *opaque, const char *dname,
                                int omitSymLinks, PHYSFS_EnumFilesCallback cb,
                                const char *origdir, void *callbackdata)
{
    LZMAarchive *archive = (LZMAarchive *) opaque;
    PHYSFS_sint32 dlen;
    PHYSFS_sint32 dlen_inc;
    PHYSFS_sint32 max;
    PHYSFS_sint32 i;

    i = lzma_find_start_of_dir(archive, dname, 0);
    if (i == -1)  /* no such directory. */
        return;

    dlen = strlen(dname);
    if ((dlen > 0) && (dname[dlen - 1] == '/')) /* ignore trailing slash. */
        dlen--;

    dlen_inc = ((dlen > 0) ? 1 : 0) + dlen;
    max = (PHYSFS_sint32) archive->db.Database.NumFiles;
    while (i < max)
    {
        char *add;
        char *ptr;
        PHYSFS_sint32 ln;
        char *e = archive->db.Database.Files[i].Name;
        if ((dlen) && ((strncmp(e, dname, dlen)) || (e[dlen] != '/')))
            break;  /* past end of this dir; we're done. */

        add = e + dlen_inc;
        ptr = strchr(add, '/');
        ln = (PHYSFS_sint32) ((ptr) ? ptr-add : strlen(add));
        doEnumCallback(cb, callbackdata, origdir, add, ln);
        ln += dlen_inc;  /* point past entry to children... */

        /* increment counter and skip children of subdirs... */
        while ((++i < max) && (ptr != NULL))
        {
            char *e_new = archive->db.Database.Files[i].Name;
            if ((strncmp(e, e_new, ln) != 0) || (e_new[ln] != '/'))
                break;
        } /* while */
    } /* while */
} /* LZMA_enumerateFiles */


static int LZMA_exists(dvoid *opaque, const char *name)
{
    LZMAarchive *archive = (LZMAarchive *) opaque;
    PHYSFS_uint32 index = 0;
    return(lzma_find_entry(archive, name, &index));
} /* LZMA_exists */


static PHYSFS_sint64 LZMA_getLastModTime(dvoid *opaque,
                                         const char *name,
                                         int *fileExists)
{
    /* !!! FIXME: Lacking support in the LZMA C SDK. */
    BAIL_MACRO(ERR_NOT_IMPLEMENTED, -1);
} /* LZMA_getLastModTime */


static int LZMA_isDirectory(dvoid *opaque, const char *name, int *fileExists)
{
    LZMAarchive *archive = (LZMAarchive *) opaque;
    PHYSFS_uint32 index = 0;

    *fileExists = lzma_find_entry(archive, name, &index);

    return(archive->db.Database.Files[index].IsDirectory);
} /* LZMA_isDirectory */


static int LZMA_isSymLink(dvoid *opaque, const char *name, int *fileExists)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, 0);
} /* LZMA_isSymLink */


static fvoid *LZMA_openRead(dvoid *opaque, const char *name, int *fileExists)
{
    LZMAarchive *archive = (LZMAarchive *) opaque;
    LZMAentry *entry = NULL;
    PHYSFS_uint32 fileIndex = 0;
    PHYSFS_uint32 folderIndex = 0;

    *fileExists = lzma_find_entry(archive, name, &fileIndex);
    BAIL_IF_MACRO(!*fileExists, ERR_NO_SUCH_FILE, NULL);

    folderIndex = archive->db.FileIndexToFolderIndexMap[fileIndex];
    BAIL_IF_MACRO(folderIndex == (PHYSFS_uint32)-1, ERR_UNKNOWN_ERROR, NULL);

    entry = (LZMAentry *) allocator.Malloc(sizeof (LZMAentry));
    BAIL_IF_MACRO(entry == NULL, ERR_OUT_OF_MEMORY, NULL);

    entry->fileIndex = fileIndex;
    entry->folderIndex = folderIndex;
    entry->archive = archive;
    entry->file = archive->db.Database.Files + entry->fileIndex;
    entry->offset = 0; /* Offset will be set by LZMA_read() */
    entry->position = 0;

    archive->folder[folderIndex].references++;

    entry->next = NULL;
    entry->previous = entry->archive->lastEntry;
    if (entry->previous != NULL)
        entry->previous->next = entry;
    entry->archive->lastEntry = entry;
    if (entry->archive->firstEntry == NULL)
        entry->archive->firstEntry = entry;

    return(entry);
} /* LZMA_openRead */


static fvoid *LZMA_openWrite(dvoid *opaque, const char *filename)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, NULL);
} /* LZMA_openWrite */


static fvoid *LZMA_openAppend(dvoid *opaque, const char *filename)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, NULL);
} /* LZMA_openAppend */


static void LZMA_dirClose(dvoid *opaque)
{
    LZMAarchive *archive = (LZMAarchive *) opaque;
    LZMAentry *entry = archive->firstEntry;
    LZMAentry *tmpEntry = entry;

    while (entry != NULL)
    {
        tmpEntry = entry->next;
        LZMA_fileClose(entry);
        entry = tmpEntry;
    } /* while */

    SzArDbExFree(&archive->db, SzFreePhysicsFS);
    __PHYSFS_platformClose(archive->stream.File);

    /* Free the cache which might have been allocated by LZMA_read() */
    allocator.Free(archive->folder);
    allocator.Free(archive);
} /* LZMA_dirClose */


static int LZMA_remove(dvoid *opaque, const char *name)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, 0);
} /* LZMA_remove */


static int LZMA_mkdir(dvoid *opaque, const char *name)
{
    BAIL_MACRO(ERR_NOT_SUPPORTED, 0);
} /* LZMA_mkdir */


const PHYSFS_ArchiveInfo __PHYSFS_ArchiveInfo_LZMA =
{
    "7Z",
    LZMA_ARCHIVE_DESCRIPTION,
    "Dennis Schridde <devurandom@gmx.net>",
    "http://icculus.org/physfs/",
};


const PHYSFS_Archiver __PHYSFS_Archiver_LZMA =
{
    &__PHYSFS_ArchiveInfo_LZMA,
    LZMA_isArchive,          /* isArchive() method      */
    LZMA_openArchive,        /* openArchive() method    */
    LZMA_enumerateFiles,     /* enumerateFiles() method */
    LZMA_exists,             /* exists() method         */
    LZMA_isDirectory,        /* isDirectory() method    */
    LZMA_isSymLink,          /* isSymLink() method      */
    LZMA_getLastModTime,     /* getLastModTime() method */
    LZMA_openRead,           /* openRead() method       */
    LZMA_openWrite,          /* openWrite() method      */
    LZMA_openAppend,         /* openAppend() method     */
    LZMA_remove,             /* remove() method         */
    LZMA_mkdir,              /* mkdir() method          */
    LZMA_dirClose,           /* dirClose() method       */
    LZMA_read,               /* read() method           */
    LZMA_write,              /* write() method          */
    LZMA_eof,                /* eof() method            */
    LZMA_tell,               /* tell() method           */
    LZMA_seek,               /* seek() method           */
    LZMA_fileLength,         /* fileLength() method     */
    LZMA_fileClose           /* fileClose() method      */
};

#endif  /* defined PHYSFS_SUPPORTS_7Z */

/* end of lzma.c ... */

