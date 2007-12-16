/*
 * OS/2 support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_platforms.h"

#ifdef PHYSFS_PLATFORM_OS2

#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME
#define INCL_DOSFILEMGR
#define INCL_DOSMODULEMGR
#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSMISC
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "physfs_internal.h"

const char *__PHYSFS_platformDirSeparator = "\\";


static const char *get_os2_error_string(APIRET rc)
{
    switch (rc)
    {
        case NO_ERROR: return(NULL);  /* not an error. */
        case ERROR_INTERRUPT: return(NULL);  /* not an error. */
        case ERROR_TIMEOUT: return(NULL);  /* not an error. */
        case ERROR_NOT_ENOUGH_MEMORY: return(ERR_OUT_OF_MEMORY);
        case ERROR_FILE_NOT_FOUND: return(ERR_NO_SUCH_FILE);
        case ERROR_PATH_NOT_FOUND: return(ERR_NO_SUCH_PATH);
        case ERROR_ACCESS_DENIED: return(ERR_ACCESS_DENIED);
        case ERROR_NOT_DOS_DISK: return(ERR_NOT_A_DOS_DISK);
        case ERROR_SHARING_VIOLATION: return(ERR_SHARING_VIOLATION);
        case ERROR_CANNOT_MAKE: return(ERR_CANNOT_MAKE);
        case ERROR_DEVICE_IN_USE: return(ERR_DEV_IN_USE);
        case ERROR_OPEN_FAILED: return(ERR_OPEN_FAILED);
        case ERROR_DISK_FULL: return(ERR_DISK_FULL);
        case ERROR_PIPE_BUSY: return(ERR_PIPE_BUSY);
        case ERROR_SHARING_BUFFER_EXCEEDED: return(ERR_SHARING_BUF_EXCEEDED);
        case ERROR_FILENAME_EXCED_RANGE: return(ERR_BAD_FILENAME);
        case ERROR_META_EXPANSION_TOO_LONG: return(ERR_BAD_FILENAME);
        case ERROR_TOO_MANY_HANDLES: return(ERR_TOO_MANY_HANDLES);
        case ERROR_TOO_MANY_OPEN_FILES: return(ERR_TOO_MANY_HANDLES);
        case ERROR_NO_MORE_SEARCH_HANDLES: return(ERR_TOO_MANY_HANDLES);
        case ERROR_SEEK_ON_DEVICE: return(ERR_SEEK_ERROR);
        case ERROR_NEGATIVE_SEEK: return(ERR_SEEK_OUT_OF_RANGE);
        /*!!! FIXME: Where did this go?  case ERROR_DEL_CURRENT_DIRECTORY: return(ERR_DEL_CWD);*/
        case ERROR_WRITE_PROTECT: return(ERR_WRITE_PROTECT_ERROR);
        case ERROR_WRITE_FAULT: return(ERR_WRITE_FAULT);
        case ERROR_LOCK_VIOLATION: return(ERR_LOCK_VIOLATION);
        case ERROR_GEN_FAILURE: return(ERR_GEN_FAILURE);
        case ERROR_UNCERTAIN_MEDIA: return(ERR_UNCERTAIN_MEDIA);
        case ERROR_PROTECTION_VIOLATION: return(ERR_PROT_VIOLATION);
        case ERROR_BROKEN_PIPE: return(ERR_BROKEN_PIPE);

        case ERROR_INVALID_PARAMETER:
        case ERROR_INVALID_NAME:
        case ERROR_INVALID_DRIVE:
        case ERROR_INVALID_HANDLE:
        case ERROR_INVALID_FUNCTION:
        case ERROR_INVALID_LEVEL:
        case ERROR_INVALID_CATEGORY:
        case ERROR_DUPLICATE_NAME:
        case ERROR_BUFFER_OVERFLOW:
        case ERROR_BAD_LENGTH:
        case ERROR_BAD_DRIVER_LEVEL:
        case ERROR_DIRECT_ACCESS_HANDLE:
        case ERROR_NOT_OWNER:
            return(ERR_PHYSFS_BAD_OS_CALL);

        default: return(ERR_OS2_GENERIC);
    } /* switch */

    return(NULL);
} /* get_os2_error_string */


static APIRET os2err(APIRET retval)
{
    char buf[128];
    const char *err = get_os2_error_string(retval);
    if (err == ERR_OS2_GENERIC)
    {
        snprintf(buf, sizeof (buf), ERR_OS2_GENERIC, (int) retval);
        err = buf;
    } /* if */

    if (err != NULL)
        __PHYSFS_setError(err);

    return(retval);
} /* os2err */


/* (be gentle, this function isn't very robust.) */
static void cvt_path_to_correct_case(char *buf)
{
    char *fname = buf + 3;            /* point to first element. */
    char *ptr = strchr(fname, '\\');  /* find end of first element. */

    buf[0] = toupper(buf[0]);  /* capitalize drive letter. */

    /*
     * Go through each path element, and enumerate its parent dir until
     *  a case-insensitive match is found. If one is (and it SHOULD be)
     *  then overwrite the original element with the correct case.
     * If there's an error, or the path has vanished for some reason, it
     *  won't hurt to have the original case, so we just keep going.
     */
    while (fname != NULL)
    {
        char spec[CCHMAXPATH];
        FILEFINDBUF3 fb;
        HDIR hdir = HDIR_CREATE;
        ULONG count = 1;
        APIRET rc;

        *(fname - 1) = '\0';  /* isolate parent dir string. */

        strcpy(spec, buf);      /* copy isolated parent dir... */
        strcat(spec, "\\*.*");  /*  ...and add wildcard search spec. */

        if (ptr != NULL)  /* isolate element to find (fname is the start). */
            *ptr = '\0';

        rc = DosFindFirst(spec, &hdir, FILE_DIRECTORY,
                          &fb, sizeof (fb), &count, FIL_STANDARD);
        if (rc == NO_ERROR)
        {
            while (count == 1)  /* while still entries to enumerate... */
            {
                if (__PHYSFS_stricmpASCII(fb.achName, fname) == 0)
                {
                    strcpy(fname, fb.achName);
                    break;  /* there it is. Overwrite and stop searching. */
                } /* if */

                DosFindNext(hdir, &fb, sizeof (fb), &count);
            } /* while */
            DosFindClose(hdir);
        } /* if */

        *(fname - 1) = '\\';   /* unisolate parent dir. */
        fname = ptr;           /* point to next element. */
        if (ptr != NULL)
        {
            *ptr = '\\';       /* unisolate element. */
            ptr = strchr(++fname, '\\');  /* find next element. */
        } /* if */
    } /* while */
} /* cvt_file_to_correct_case */


static char *baseDir = NULL;

int __PHYSFS_platformInit(void)
{
    char buf[CCHMAXPATH];
    APIRET rc;
    PTIB ptib;
    PPIB ppib;
    PHYSFS_sint32 len;

    assert(baseDir == NULL);
    BAIL_IF_MACRO(os2err(DosGetInfoBlocks(&ptib, &ppib)) != NO_ERROR, NULL, 0);
    rc = DosQueryModuleName(ppib->pib_hmte, sizeof (buf), (PCHAR) buf);
    BAIL_IF_MACRO(os2err(rc) != NO_ERROR, NULL, 0);

    /* chop off filename, leave path. */
    for (len = strlen(buf) - 1; len >= 0; len--)
    {
        if (buf[len] == '\\')
        {
            buf[len] = '\0';
            break;
        } /* if */
    } /* for */

    assert(len > 0);  /* should have been a "x:\\" on the front on string. */

    /* The string is capitalized! Figure out the REAL case... */
    cvt_path_to_correct_case(buf);

    baseDir = (char *) allocator.Malloc(len + 1);
    BAIL_IF_MACRO(baseDir == NULL, ERR_OUT_OF_MEMORY, 0);
    strcpy(baseDir, buf);
    return(1);  /* success. */
} /* __PHYSFS_platformInit */


int __PHYSFS_platformDeinit(void)
{
    assert(baseDir != NULL);
    allocator.Free(baseDir);
    baseDir = NULL;
    return(1);  /* success. */
} /* __PHYSFS_platformDeinit */


static int disc_is_inserted(ULONG drive)
{
    int rc;
    char buf[20];
    DosError(FERR_DISABLEHARDERR | FERR_DISABLEEXCEPTION);
    rc = DosQueryFSInfo(drive + 1, FSIL_VOLSER, buf, sizeof (buf));
    DosError(FERR_ENABLEHARDERR | FERR_ENABLEEXCEPTION);
    return(rc == NO_ERROR);
} /* is_cdrom_inserted */


/* looks like "CD01" in ASCII (littleendian)...used for an ioctl. */
#define CD01 0x31304443

static int is_cdrom_drive(ULONG drive)
{
    PHYSFS_uint32 param, data;
    ULONG ul1, ul2;
    APIRET rc;
    HFILE hfile = NULLHANDLE;
    char drivename[3] = { 'A' + drive, ':', '\0' };

    rc = DosOpen(drivename, &hfile, &ul1, 0, 0,
                 OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_FAIL_IF_NEW,
                 OPEN_FLAGS_DASD | OPEN_FLAGS_FAIL_ON_ERROR |
                 OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE, NULL);
    BAIL_IF_MACRO(rc != NO_ERROR, NULL, 0);

    data = 0;
    param = PHYSFS_swapULE32(CD01);
    ul1 = ul2 = sizeof (PHYSFS_uint32);
    rc = DosDevIOCtl(hfile, IOCTL_CDROMDISK, CDROMDISK_GETDRIVER,
                     &param, sizeof (param), &ul1, &data, sizeof (data), &ul2);

    DosClose(hfile);
    return((rc == NO_ERROR) && (PHYSFS_swapULE32(data) == CD01));
} /* is_cdrom_drive */


void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
    ULONG dummy = 0;
    ULONG drivemap = 0;
    ULONG i, bit;
    APIRET rc = DosQueryCurrentDisk(&dummy, &drivemap);
    if (os2err(rc) != NO_ERROR)
        return;

    for (i = 0, bit = 1; i < 26; i++, bit <<= 1)
    {
        if (drivemap & bit)  /* this logical drive exists. */
        {
            if ((is_cdrom_drive(i)) && (disc_is_inserted(i)))
            {
                char drive[4] = "x:\\";
                drive[0] = ('A' + i);
                cb(data, drive);
            } /* if */
        } /* if */
    } /* for */
} /* __PHYSFS_platformDetectAvailableCDs */


char *__PHYSFS_platformCalcBaseDir(const char *argv0)
{
    char *retval = (char *) allocator.Malloc(strlen(baseDir) + 1);
    BAIL_IF_MACRO(retval == NULL, ERR_OUT_OF_MEMORY, NULL);
    strcpy(retval, baseDir); /* calculated at init time. */
    return(retval);
} /* __PHYSFS_platformCalcBaseDir */


char *__PHYSFS_platformGetUserName(void)
{
    return(NULL);  /* (*shrug*) */
} /* __PHYSFS_platformGetUserName */


char *__PHYSFS_platformGetUserDir(void)
{
    return(__PHYSFS_platformCalcBaseDir(NULL));
} /* __PHYSFS_platformGetUserDir */


int __PHYSFS_platformExists(const char *fname)
{
    FILESTATUS3 fs;
    APIRET rc = DosQueryPathInfo(fname, FIL_STANDARD, &fs, sizeof (fs));
    return(os2err(rc) == NO_ERROR);
} /* __PHYSFS_platformExists */


int __PHYSFS_platformIsSymLink(const char *fname)
{
    return(0);  /* no symlinks in OS/2. */
} /* __PHYSFS_platformIsSymlink */


int __PHYSFS_platformIsDirectory(const char *fname)
{
    FILESTATUS3 fs;
    APIRET rc = DosQueryPathInfo(fname, FIL_STANDARD, &fs, sizeof (fs));
    BAIL_IF_MACRO(os2err(rc) != NO_ERROR, NULL, 0)
    return((fs.attrFile & FILE_DIRECTORY) != 0);
} /* __PHYSFS_platformIsDirectory */


/* !!! FIXME: can we lose the malloc here? */
char *__PHYSFS_platformCvtToDependent(const char *prepend,
                                      const char *dirName,
                                      const char *append)
{
    int len = ((prepend) ? strlen(prepend) : 0) +
              ((append) ? strlen(append) : 0) +
              strlen(dirName) + 1;
    char *retval = allocator.Malloc(len);
    char *p;

    BAIL_IF_MACRO(retval == NULL, ERR_OUT_OF_MEMORY, NULL);

    if (prepend)
        strcpy(retval, prepend);
    else
        retval[0] = '\0';

    strcat(retval, dirName);

    if (append)
        strcat(retval, append);

    for (p = strchr(retval, '/'); p != NULL; p = strchr(p + 1, '/'))
        *p = '\\';

    return(retval);
} /* __PHYSFS_platformCvtToDependent */


void __PHYSFS_platformEnumerateFiles(const char *dirname,
                                     int omitSymLinks,
                                     PHYSFS_EnumFilesCallback callback,
                                     const char *origdir,
                                     void *callbackdata)
{
    char spec[CCHMAXPATH];
    FILEFINDBUF3 fb;
    HDIR hdir = HDIR_CREATE;
    ULONG count = 1;
    APIRET rc;

    if (strlen(dirname) > sizeof (spec) - 5)
    {
        __PHYSFS_setError(ERR_BAD_FILENAME);
        return;
    } /* if */

    strcpy(spec, dirname);
    strcat(spec, (spec[strlen(spec) - 1] != '\\') ? "\\*.*" : "*.*");

    rc = DosFindFirst(spec, &hdir,
                      FILE_DIRECTORY | FILE_ARCHIVED |
                      FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM,
                      &fb, sizeof (fb), &count, FIL_STANDARD);

    if (os2err(rc) != NO_ERROR)
        return;

    while (count == 1)
    {
        if ((strcmp(fb.achName, ".") != 0) && (strcmp(fb.achName, "..") != 0))
            callback(callbackdata, origdir, fb.achName);

        DosFindNext(hdir, &fb, sizeof (fb), &count);
    } /* while */

    DosFindClose(hdir);
} /* __PHYSFS_platformEnumerateFiles */


char *__PHYSFS_platformCurrentDir(void)
{
    char *retval;
    ULONG currentDisk;
    ULONG dummy;
    ULONG pathSize = 0;
    APIRET rc;
    BYTE byte;

    rc = DosQueryCurrentDisk(&currentDisk, &dummy);
    BAIL_IF_MACRO(os2err(rc) != NO_ERROR, NULL, NULL);

    /* The first call just tells us how much space we need for the string. */
    rc = DosQueryCurrentDir(currentDisk, &byte, &pathSize);
    pathSize++; /* Add space for null terminator. */
    retval = (char *) allocator.Malloc(pathSize + 3);  /* plus "x:\\" */
    BAIL_IF_MACRO(retval == NULL, ERR_OUT_OF_MEMORY, NULL);

    /* Actually get the string this time. */
    rc = DosQueryCurrentDir(currentDisk, (PBYTE) (retval + 3), &pathSize);
    if (os2err(rc) != NO_ERROR)
    {
        allocator.Free(retval);
        return(NULL);
    } /* if */

    retval[0] = ('A' + (currentDisk - 1));
    retval[1] = ':';
    retval[2] = '\\';
    return(retval);
} /* __PHYSFS_platformCurrentDir */


char *__PHYSFS_platformRealPath(const char *path)
{
    char buf[CCHMAXPATH];
    char *retval;
    APIRET rc = DosQueryPathInfo(path, FIL_QUERYFULLNAME, buf, sizeof (buf));
    BAIL_IF_MACRO(os2err(rc) != NO_ERROR, NULL, NULL);
    retval = (char *) allocator.Malloc(strlen(buf) + 1);
    BAIL_IF_MACRO(retval == NULL, ERR_OUT_OF_MEMORY, NULL);
    strcpy(retval, buf);
    return(retval);
} /* __PHYSFS_platformRealPath */


int __PHYSFS_platformMkDir(const char *path)
{
    return(os2err(DosCreateDir(path, NULL)) == NO_ERROR);
} /* __PHYSFS_platformMkDir */


void *__PHYSFS_platformOpenRead(const char *filename)
{
    ULONG actionTaken = 0;
    HFILE hfile = NULLHANDLE;

    /*
     * File must be opened SHARE_DENYWRITE and ACCESS_READONLY, otherwise
     *  DosQueryFileInfo() will fail if we try to get a file length, etc.
     */
    os2err(DosOpen(filename, &hfile, &actionTaken, 0, FILE_NORMAL,
                   OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_FAIL_IF_NEW,
                   OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NO_LOCALITY |
                   OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYWRITE |
                   OPEN_ACCESS_READONLY, NULL));

    return((void *) hfile);
} /* __PHYSFS_platformOpenRead */


void *__PHYSFS_platformOpenWrite(const char *filename)
{
    ULONG actionTaken = 0;
    HFILE hfile = NULLHANDLE;

    /*
     * File must be opened SHARE_DENYWRITE and ACCESS_READWRITE, otherwise
     *  DosQueryFileInfo() will fail if we try to get a file length, etc.
     */
    os2err(DosOpen(filename, &hfile, &actionTaken, 0, FILE_NORMAL,
                   OPEN_ACTION_REPLACE_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW,
                   OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NO_LOCALITY |
                   OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYWRITE |
                   OPEN_ACCESS_READWRITE, NULL));

    return((void *) hfile);
} /* __PHYSFS_platformOpenWrite */


void *__PHYSFS_platformOpenAppend(const char *filename)
{
    ULONG dummy = 0;
    HFILE hfile = NULLHANDLE;
    APIRET rc;

    /*
     * File must be opened SHARE_DENYWRITE and ACCESS_READWRITE, otherwise
     *  DosQueryFileInfo() will fail if we try to get a file length, etc.
     */
    rc = os2err(DosOpen(filename, &hfile, &dummy, 0, FILE_NORMAL,
                   OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW,
                   OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NO_LOCALITY |
                   OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYWRITE |
                   OPEN_ACCESS_READWRITE, NULL));

    if (rc == NO_ERROR)
    {
        if (os2err(DosSetFilePtr(hfile, 0, FILE_END, &dummy)) != NO_ERROR)
        {
            DosClose(hfile);
            hfile = NULLHANDLE;
        } /* if */
    } /* if */

    return((void *) hfile);
} /* __PHYSFS_platformOpenAppend */


PHYSFS_sint64 __PHYSFS_platformRead(void *opaque, void *buffer,
                                    PHYSFS_uint32 size, PHYSFS_uint32 count)
{
    HFILE hfile = (HFILE) opaque;
    PHYSFS_sint64 retval;
    ULONG br;

    for (retval = 0; retval < count; retval++)
    {
        os2err(DosRead(hfile, buffer, size, &br));
        if (br < size)
        {
            DosSetFilePtr(hfile, -br, FILE_CURRENT, &br); /* try to cleanup. */
            return(retval);
        } /* if */

        buffer = (void *) ( ((char *) buffer) + size );
    } /* for */

    return(retval);
} /* __PHYSFS_platformRead */


PHYSFS_sint64 __PHYSFS_platformWrite(void *opaque, const void *buffer,
                                     PHYSFS_uint32 size, PHYSFS_uint32 count)
{
    HFILE hfile = (HFILE) opaque;
    PHYSFS_sint64 retval;
    ULONG bw;

    for (retval = 0; retval < count; retval++)
    {
        os2err(DosWrite(hfile, buffer, size, &bw));
        if (bw < size)
        {
            DosSetFilePtr(hfile, -bw, FILE_CURRENT, &bw); /* try to cleanup. */
            return(retval);
        } /* if */

        buffer = (void *) ( ((char *) buffer) + size );
    } /* for */

    return(retval);
} /* __PHYSFS_platformWrite */


int __PHYSFS_platformSeek(void *opaque, PHYSFS_uint64 pos)
{
    ULONG dummy;
    HFILE hfile = (HFILE) opaque;
    LONG dist = (LONG) pos;

    /* hooray for 32-bit filesystem limits!  :) */
    BAIL_IF_MACRO((PHYSFS_uint64) dist != pos, ERR_SEEK_OUT_OF_RANGE, 0);

    return(os2err(DosSetFilePtr(hfile, dist, FILE_BEGIN, &dummy)) == NO_ERROR);
} /* __PHYSFS_platformSeek */


PHYSFS_sint64 __PHYSFS_platformTell(void *opaque)
{
    ULONG pos;
    HFILE hfile = (HFILE) opaque;
    APIRET rc = os2err(DosSetFilePtr(hfile, 0, FILE_CURRENT, &pos));
    BAIL_IF_MACRO(rc != NO_ERROR, NULL, -1);
    return((PHYSFS_sint64) pos);
} /* __PHYSFS_platformTell */


PHYSFS_sint64 __PHYSFS_platformFileLength(void *opaque)
{
    FILESTATUS3 fs;
    HFILE hfile = (HFILE) opaque;
    APIRET rc = DosQueryFileInfo(hfile, FIL_STANDARD, &fs, sizeof (fs));
    BAIL_IF_MACRO(os2err(rc) != NO_ERROR, NULL, -1);
    return((PHYSFS_sint64) fs.cbFile);
} /* __PHYSFS_platformFileLength */


int __PHYSFS_platformEOF(void *opaque)
{
    PHYSFS_sint64 len, pos;

    len = __PHYSFS_platformFileLength(opaque);
    BAIL_IF_MACRO(len == -1, NULL, 1);  /* (*shrug*) */
    pos = __PHYSFS_platformTell(opaque);
    BAIL_IF_MACRO(pos == -1, NULL, 1);  /* (*shrug*) */

    return(pos >= len);
} /* __PHYSFS_platformEOF */


int __PHYSFS_platformFlush(void *opaque)
{
    return(os2err(DosResetBuffer((HFILE) opaque) == NO_ERROR));
} /* __PHYSFS_platformFlush */


int __PHYSFS_platformClose(void *opaque)
{
    return(os2err(DosClose((HFILE) opaque) == NO_ERROR));
} /* __PHYSFS_platformClose */


int __PHYSFS_platformDelete(const char *path)
{
    if (__PHYSFS_platformIsDirectory(path))
        return(os2err(DosDeleteDir(path)) == NO_ERROR);

    return(os2err(DosDelete(path) == NO_ERROR));
} /* __PHYSFS_platformDelete */


PHYSFS_sint64 __PHYSFS_platformGetLastModTime(const char *fname)
{
    PHYSFS_sint64 retval;
    struct tm tm;
    FILESTATUS3 fs;
    APIRET rc = DosQueryPathInfo(fname, FIL_STANDARD, &fs, sizeof (fs));
    BAIL_IF_MACRO(os2err(rc) != NO_ERROR, NULL, -1);

    /* Convert to a format that mktime() can grok... */
    tm.tm_sec = ((PHYSFS_uint32) fs.ftimeLastWrite.twosecs) * 2;
    tm.tm_min = fs.ftimeLastWrite.minutes;
    tm.tm_hour = fs.ftimeLastWrite.hours;
    tm.tm_mday = fs.fdateLastWrite.day;
    tm.tm_mon = fs.fdateLastWrite.month;
    tm.tm_year = ((PHYSFS_uint32) fs.fdateLastWrite.year) + 80;
    tm.tm_wday = -1 /*st_localtz.wDayOfWeek*/;
    tm.tm_yday = -1;
    tm.tm_isdst = -1;

    /* Convert to a format PhysicsFS can grok... */
    retval = (PHYSFS_sint64) mktime(&tm);
    BAIL_IF_MACRO(retval == -1, strerror(errno), -1);
    return(retval);
} /* __PHYSFS_platformGetLastModTime */


PHYSFS_uint64 __PHYSFS_platformGetThreadID(void)
{
    PTIB ptib;
    PPIB ppib;

    /*
     * Allegedly, this API never fails, but we'll punt and return a
     *  default value (zero might as well do) if it does.
     */
    BAIL_IF_MACRO(os2err(DosGetInfoBlocks(&ptib, &ppib)) != NO_ERROR, 0, 0);
    return((PHYSFS_uint64) ptib->tib_ordinal);
} /* __PHYSFS_platformGetThreadID */


void *__PHYSFS_platformCreateMutex(void)
{
    HMTX hmtx = NULLHANDLE;
    os2err(DosCreateMutexSem(NULL, &hmtx, 0, 0));
    return((void *) hmtx);
} /* __PHYSFS_platformCreateMutex */


void __PHYSFS_platformDestroyMutex(void *mutex)
{
    DosCloseMutexSem((HMTX) mutex);
} /* __PHYSFS_platformDestroyMutex */


int __PHYSFS_platformGrabMutex(void *mutex)
{
    /* Do _NOT_ call os2err() (which sets the physfs error msg) in here! */
    return(DosRequestMutexSem((HMTX) mutex, SEM_INDEFINITE_WAIT) == NO_ERROR);
} /* __PHYSFS_platformGrabMutex */


void __PHYSFS_platformReleaseMutex(void *mutex)
{
    DosReleaseMutexSem((HMTX) mutex);
} /* __PHYSFS_platformReleaseMutex */


/* !!! FIXME: Don't use C runtime for allocators? */
int __PHYSFS_platformSetDefaultAllocator(PHYSFS_Allocator *a)
{
    return(0);  /* just use malloc() and friends. */
} /* __PHYSFS_platformSetDefaultAllocator */

#endif  /* PHYSFS_PLATFORM_OS2 */

/* end of os2.c ... */

