/*
 * PocketPC support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_platforms.h"

#ifdef PHYSFS_PLATFORM_POCKETPC

#include <stdio.h>
#include <windows.h>

#include "physfs_internal.h"

#define INVALID_FILE_ATTRIBUTES  0xFFFFFFFF
#define INVALID_SET_FILE_POINTER 0xFFFFFFFF
typedef struct
{
    HANDLE handle;
    int readonly;
} winCEfile;


const char *__PHYSFS_platformDirSeparator = "\\";
static char *userDir = NULL;

/*
 * Figure out what the last failing Win32 API call was, and
 *  generate a human-readable string for the error message.
 *
 * The return value is a static buffer that is overwritten with
 *  each call to this function.
 */
static const char *win32strerror(void)
{
    static TCHAR msgbuf[255];
    TCHAR *ptr = msgbuf;

    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
        msgbuf,
        sizeof (msgbuf) / sizeof (TCHAR),
        NULL 
        );

    /* chop off newlines. */
    for (ptr = msgbuf; *ptr; ptr++)
    {
        if ((*ptr == '\n') || (*ptr == '\r'))
        {
            *ptr = ' ';
            break;
        } /* if */
    } /* for */

    return((const char *) msgbuf);
} /* win32strerror */


/* !!! FIXME: need to check all of these for NULLs. */
#define UTF8_TO_UNICODE_STACK_MACRO(w_assignto, str) { \
    if (str == NULL) \
        w_assignto = NULL; \
    else { \
        const PHYSFS_uint64 len = (PHYSFS_uint64) ((strlen(str) * 4) + 1); \
        w_assignto = (char *) __PHYSFS_smallAlloc(len); \
        PHYSFS_uc2fromutf8(str, (PHYSFS_uint16 *) w_assignto, len); \
    } \
} \


static char *getExePath()
{
    DWORD buflen;
    int success = 0;
    TCHAR *ptr = NULL;
    TCHAR *retval = (TCHAR*) allocator.Malloc(sizeof (TCHAR) * (MAX_PATH + 1));
    char *charretval;
    BAIL_IF_MACRO(retval == NULL, ERR_OUT_OF_MEMORY, NULL);

    retval[0] = _T('\0');
    /* !!! FIXME: don't preallocate here? */
    /* !!! FIXME: use smallAlloc? */
    buflen = GetModuleFileName(NULL, retval, MAX_PATH + 1);
    if (buflen <= 0)
        __PHYSFS_setError(win32strerror());
    else
    {
        retval[buflen] = '\0';  /* does API always null-terminate this? */
        ptr = retval+buflen;
        while( ptr != retval )
        {
            if( *ptr != _T('\\') )
                *ptr-- = _T('\0');
            else
                break;
        } /* while */
        success = 1;
    } /* else */

    if (!success)
    {
        allocator.Free(retval);
        return(NULL);  /* physfs error message will be set, above. */
    } /* if */

    buflen = (buflen * 4) + 1;
    charretval = (char *) allocator.Malloc(buflen);
    if (charretval != NULL)
        PHYSFS_utf8fromucs2((const PHYSFS_uint16 *) retval, charretval, buflen);
    allocator.Free(retval);
    return(charretval);   /* w00t. */
} /* getExePath */


int __PHYSFS_platformInit(void)
{
    userDir = getExePath();
    BAIL_IF_MACRO(userDir == NULL, NULL, 0); /* failed? */
    return(1);  /* always succeed. */
} /* __PHYSFS_platformInit */


int __PHYSFS_platformDeinit(void)
{
    allocator.Free(userDir);
    return(1);  /* always succeed. */
} /* __PHYSFS_platformDeinit */


void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
    /* no-op on this platform. */
} /* __PHYSFS_platformDetectAvailableCDs */


char *__PHYSFS_platformCalcBaseDir(const char *argv0)
{
    return(getExePath());
} /* __PHYSFS_platformCalcBaseDir */


char *__PHYSFS_platformGetUserName(void)
{
    BAIL_MACRO(ERR_NOT_IMPLEMENTED, NULL);
} /* __PHYSFS_platformGetUserName */


char *__PHYSFS_platformGetUserDir(void)
{
    return userDir;
    BAIL_MACRO(ERR_NOT_IMPLEMENTED, NULL);
} /* __PHYSFS_platformGetUserDir */


PHYSFS_uint64 __PHYSFS_platformGetThreadID(void)
{
    return(1);  /* single threaded. */
} /* __PHYSFS_platformGetThreadID */


int __PHYSFS_platformExists(const char *fname)
{
    int retval = 0;
    wchar_t *w_fname = NULL;

    UTF8_TO_UNICODE_STACK_MACRO(w_fname, fname);
    if (w_fname != NULL)
        retval = (GetFileAttributes(w_fname) != INVALID_FILE_ATTRIBUTES);
    __PHYSFS_smallFree(w_fname);

    return(retval);
} /* __PHYSFS_platformExists */


int __PHYSFS_platformIsSymLink(const char *fname)
{
    BAIL_MACRO(ERR_NOT_IMPLEMENTED, 0);
} /* __PHYSFS_platformIsSymlink */


int __PHYSFS_platformIsDirectory(const char *fname)
{
    int retval = 0;
    wchar_t *w_fname = NULL;

    UTF8_TO_UNICODE_STACK_MACRO(w_fname, fname);
    if (w_fname != NULL)
        retval = ((GetFileAttributes(w_fname) & FILE_ATTRIBUTE_DIRECTORY) != 0);
    __PHYSFS_smallFree(w_fname);

    return(retval);
} /* __PHYSFS_platformIsDirectory */


char *__PHYSFS_platformCvtToDependent(const char *prepend,
                                      const char *dirName,
                                      const char *append)
{
    int len = ((prepend) ? strlen(prepend) : 0) +
    ((append) ? strlen(append) : 0) +
    strlen(dirName) + 1;
    char *retval = (char *) allocator.Malloc(len);
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


static int doEnumCallback(const wchar_t *w_fname)
{
    const PHYSFS_uint64 len = (PHYSFS_uint64) ((wcslen(w_fname) * 4) + 1);
    char *str = (char *) __PHYSFS_smallAlloc(len);
    PHYSFS_utf8fromucs2((const PHYSFS_uint16 *) w_fname, str, len);
    callback(callbackdata, origdir, str);
    __PHYSFS_smallFree(str);
    return 1;
} /* doEnumCallback */


void __PHYSFS_platformEnumerateFiles(const char *dirname,
                                     int omitSymLinks,
                                     PHYSFS_EnumFilesCallback callback,
                                     const char *origdir,
                                     void *callbackdata)
{
    HANDLE dir;
    WIN32_FIND_DATA ent;
    char *SearchPath;
    wchar_t *w_SearchPath;
    size_t len = strlen(dirname);

    /* Allocate a new string for path, maybe '\\', "*", and NULL terminator */
    SearchPath = (char *) __PHYSFS_smallAlloc(len + 3);
    BAIL_IF_MACRO(SearchPath == NULL, ERR_OUT_OF_MEMORY, NULL);    

    /* Copy current dirname */
    strcpy(SearchPath, dirname);

    /* if there's no '\\' at the end of the path, stick one in there. */
    if (SearchPath[len - 1] != '\\')
    {
        SearchPath[len++] = '\\';
        SearchPath[len] = '\0';
    } /* if */

    /* Append the "*" to the end of the string */
    strcat(SearchPath, "*");

    UTF8_TO_UNICODE_STACK_MACRO(w_SearchPath, SearchPath);
    __PHYSFS_smallFree(SearchPath);
    dir = FindFirstFile(w_SearchPath, &ent);
    __PHYSFS_smallFree(w_SearchPath);

    if (dir == INVALID_HANDLE_VALUE)
        return;

    do
    {
        const char *str = NULL;

        if (wcscmp(ent.cFileName, L".") == 0)
            continue;

        if (wcscmp(ent.cFileName, L"..") == 0)
            continue;

        if (!doEnumCallback(ent.cFileName))
            break;
    } while (FindNextFile(dir, &ent) != 0);

    FindClose(dir);
} /* __PHYSFS_platformEnumerateFiles */


char *__PHYSFS_platformCurrentDir(void)
{
    return("\\");
} /* __PHYSFS_platformCurrentDir */


char *__PHYSFS_platformRealPath(const char *path)
{
    char *retval = (char *) allocator.Malloc(strlen(path) + 1);
    strcpy(retval,path);
    return(retval);
} /* __PHYSFS_platformRealPath */


int __PHYSFS_platformMkDir(const char *path)
{
    int retval = 0;
    wchar_t *w_path = NULL;
    UTF8_TO_UNICODE_STACK_MACRO(w_path, path);
    if (w_path != NULL)
    {
        retval = CreateDirectory(w_path, NULL);
        __PHYSFS_smallFree(w_fname);
    } /* if */
    return(retval);
} /* __PHYSFS_platformMkDir */


static void *doOpen(const char *fname, DWORD mode, DWORD creation, int rdonly)
{
    HANDLE fileHandle;
    winCEfile *retval;
    wchar_t *w_fname = NULL;

    UTF8_TO_UNICODE_STACK_MACRO(w_fname, fname);
    fileHandle = CreateFile(w_fname, mode, FILE_SHARE_READ, NULL,
                            creation, FILE_ATTRIBUTE_NORMAL, NULL);
    __PHYSFS_smallFree(w_fname);

    BAIL_IF_MACRO(fileHandle == INVALID_HANDLE_VALUE, win32strerror(), NULL);

    retval = (winCEfile *) allocator.Malloc(sizeof (winCEfile));
    if (retval == NULL)
    {
        CloseHandle(fileHandle);
        BAIL_MACRO(ERR_OUT_OF_MEMORY, NULL);
    } /* if */

    retval->readonly = rdonly;
    retval->handle = fileHandle;
    return(retval);
} /* doOpen */


void *__PHYSFS_platformOpenRead(const char *filename)
{
    return(doOpen(filename, GENERIC_READ, OPEN_EXISTING, 1));
} /* __PHYSFS_platformOpenRead */


void *__PHYSFS_platformOpenWrite(const char *filename)
{
    return(doOpen(filename, GENERIC_WRITE, CREATE_ALWAYS, 0));
} /* __PHYSFS_platformOpenWrite */


void *__PHYSFS_platformOpenAppend(const char *filename)
{
    void *retval = doOpen(filename, GENERIC_WRITE, OPEN_ALWAYS, 0);
    if (retval != NULL)
    {
        HANDLE h = ((winCEfile *) retval)->handle;
        if (SetFilePointer(h, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
        {
            const char *err = win32strerror();
            CloseHandle(h);
            allocator.Free(retval);
            BAIL_MACRO(err, NULL);
        } /* if */
    } /* if */

    return(retval);

} /* __PHYSFS_platformOpenAppend */


PHYSFS_sint64 __PHYSFS_platformRead(void *opaque, void *buffer,
                                    PHYSFS_uint32 size, PHYSFS_uint32 count)
{
    HANDLE Handle = ((winCEfile *) opaque)->handle;
    DWORD CountOfBytesRead;
    PHYSFS_sint64 retval;

    /* Read data from the file */
    /*!!! - uint32 might be a greater # than DWORD */
    if (!ReadFile(Handle, buffer, count * size, &CountOfBytesRead, NULL))
    {
        retval = -1;
    } /* if */
    else
    {
        /* Return the number of "objects" read. */
        /* !!! - What if not the right amount of bytes was read to make an object? */
        retval = CountOfBytesRead / size;
    } /* else */

    return(retval);

} /* __PHYSFS_platformRead */


PHYSFS_sint64 __PHYSFS_platformWrite(void *opaque, const void *buffer,
                                     PHYSFS_uint32 size, PHYSFS_uint32 count)
{
    HANDLE Handle = ((winCEfile *) opaque)->handle;
    DWORD CountOfBytesWritten;
    PHYSFS_sint64 retval;

    /* Read data from the file */
    /*!!! - uint32 might be a greater # than DWORD */
    if (!WriteFile(Handle, buffer, count * size, &CountOfBytesWritten, NULL))
    {
        retval = -1;
    } /* if */
    else
    {
        /* Return the number of "objects" read. */
        /*!!! - What if not the right number of bytes was written? */
        retval = CountOfBytesWritten / size;
    } /* else */

    return(retval);

} /* __PHYSFS_platformWrite */


int __PHYSFS_platformSeek(void *opaque, PHYSFS_uint64 pos)
{
    HANDLE Handle = ((winCEfile *) opaque)->handle;
    DWORD HighOrderPos;
    DWORD rc;

    /* Get the high order 32-bits of the position */
    //HighOrderPos = HIGHORDER_UINT64(pos);
    HighOrderPos = (unsigned long)(pos>>32);

    /*!!! SetFilePointer needs a signed 64-bit value. */
    /* Move pointer "pos" count from start of file */
    rc = SetFilePointer(Handle, (unsigned long)(pos&0x00000000ffffffff),
                        &HighOrderPos, FILE_BEGIN);

    if ((rc == INVALID_SET_FILE_POINTER) && (GetLastError() != NO_ERROR))
    {
        BAIL_MACRO(win32strerror(), 0);
    }

    return(1);  /* No error occured */
} /* __PHYSFS_platformSeek */


PHYSFS_sint64 __PHYSFS_platformTell(void *opaque)
{
    HANDLE Handle = ((winCEfile *) opaque)->handle;
    DWORD HighPos = 0;
    DWORD LowPos;
    PHYSFS_sint64 retval;

    /* Get current position */
    LowPos = SetFilePointer(Handle, 0, &HighPos, FILE_CURRENT);
    if ((LowPos == INVALID_SET_FILE_POINTER) && (GetLastError() != NO_ERROR))
    {
        BAIL_MACRO(win32strerror(), 0);
    } /* if */
    else
    {
        /* Combine the high/low order to create the 64-bit position value */
        retval = (((PHYSFS_uint64) HighPos) << 32) | LowPos;
        //assert(retval >= 0);
    } /* else */

    return(retval);
} /* __PHYSFS_platformTell */


PHYSFS_sint64 __PHYSFS_platformFileLength(void *opaque)
{
    HANDLE Handle = ((winCEfile *) opaque)->handle;
    DWORD SizeHigh;
    DWORD SizeLow;
    PHYSFS_sint64 retval;

    SizeLow = GetFileSize(Handle, &SizeHigh);
    if ((SizeLow == INVALID_SET_FILE_POINTER) && (GetLastError() != NO_ERROR))
    {
        BAIL_MACRO(win32strerror(), -1);
    } /* if */
    else
    {
        /* Combine the high/low order to create the 64-bit position value */
        retval = (((PHYSFS_uint64) SizeHigh) << 32) | SizeLow;
        //assert(retval >= 0);
    } /* else */

    return(retval);
} /* __PHYSFS_platformFileLength */


int __PHYSFS_platformEOF(void *opaque)
{
    PHYSFS_sint64 FilePosition;
    int retval = 0;

    /* Get the current position in the file */
    if ((FilePosition = __PHYSFS_platformTell(opaque)) != 0)
    {
        /* Non-zero if EOF is equal to the file length */
        retval = FilePosition == __PHYSFS_platformFileLength(opaque);
    } /* if */

    return(retval);
} /* __PHYSFS_platformEOF */


int __PHYSFS_platformFlush(void *opaque)
{
    winCEfile *fh = ((winCEfile *) opaque);
    if (!fh->readonly)
        BAIL_IF_MACRO(!FlushFileBuffers(fh->handle), win32strerror(), 0);

    return(1);
} /* __PHYSFS_platformFlush */


int __PHYSFS_platformClose(void *opaque)
{
    HANDLE Handle = ((winCEfile *) opaque)->handle;
    BAIL_IF_MACRO(!CloseHandle(Handle), win32strerror(), 0);
    allocator.Free(opaque);
    return(1);
} /* __PHYSFS_platformClose */


int __PHYSFS_platformDelete(const char *path)
{
    wchar_t *w_path = NULL;
    UTF8_TO_UNICODE_STACK_MACRO(w_path, path);

    /* If filename is a folder */
    if (GetFileAttributes(w_path) == FILE_ATTRIBUTE_DIRECTORY)
    {
        int retval = !RemoveDirectory(w_path);
        __PHYSFS_smallFree(w_path);
        BAIL_IF_MACRO(retval, win32strerror(), 0);
    } /* if */
    else
    {
        int retval = !DeleteFile(w_path);
        __PHYSFS_smallFree(w_path);
        BAIL_IF_MACRO(retval, win32strerror(), 0);
    } /* else */

    return(1);  /* if you got here, it worked. */
} /* __PHYSFS_platformDelete */


/*
 * !!! FIXME: why aren't we using Critical Sections instead of Mutexes?
 * !!! FIXME:  mutexes on Windows are for cross-process sync. CritSects are
 * !!! FIXME:  mutexes for threads in a single process and are faster.
 */
void *__PHYSFS_platformCreateMutex(void)
{
    return((void *) CreateMutex(NULL, FALSE, NULL));
} /* __PHYSFS_platformCreateMutex */


void __PHYSFS_platformDestroyMutex(void *mutex)
{
    CloseHandle((HANDLE) mutex);
} /* __PHYSFS_platformDestroyMutex */


int __PHYSFS_platformGrabMutex(void *mutex)
{
    return(WaitForSingleObject((HANDLE) mutex, INFINITE) != WAIT_FAILED);
} /* __PHYSFS_platformGrabMutex */


void __PHYSFS_platformReleaseMutex(void *mutex)
{
    ReleaseMutex((HANDLE) mutex);
} /* __PHYSFS_platformReleaseMutex */


PHYSFS_sint64 __PHYSFS_platformGetLastModTime(const char *fname)
{
    BAIL_MACRO(ERR_NOT_IMPLEMENTED, -1);
} /* __PHYSFS_platformGetLastModTime */


/* !!! FIXME: Don't use C runtime for allocators? */
int __PHYSFS_platformSetDefaultAllocator(PHYSFS_Allocator *a)
{
    return(0);  /* just use malloc() and friends. */
} /* __PHYSFS_platformSetDefaultAllocator */

#endif  /* PHYSFS_PLATFORM_POCKETPC */

/* end of pocketpc.c ... */

