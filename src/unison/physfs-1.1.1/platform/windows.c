/*
 * Windows support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon, and made sane by Gregory S. Read.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_platforms.h"

#ifdef PHYSFS_PLATFORM_WINDOWS

/* Forcibly disable UNICODE, since we manage this ourselves. */
#ifdef UNICODE
#undef UNICODE
#endif

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#include "physfs_internal.h"

#define LOWORDER_UINT64(pos) (PHYSFS_uint32) \
    (pos & 0x00000000FFFFFFFF)
#define HIGHORDER_UINT64(pos) (PHYSFS_uint32) \
    (((pos & 0xFFFFFFFF00000000) >> 32) & 0x00000000FFFFFFFF)

/*
 * Users without the platform SDK don't have this defined.  The original docs
 *  for SetFilePointer() just said to compare with 0xFFFFFFFF, so this should
 *  work as desired.
 */
#define PHYSFS_INVALID_SET_FILE_POINTER  0xFFFFFFFF

/* just in case... */
#define PHYSFS_INVALID_FILE_ATTRIBUTES   0xFFFFFFFF

/* Not defined before the Vista SDK. */
#define PHYSFS_IO_REPARSE_TAG_SYMLINK    0xA000000C


#define UTF8_TO_UNICODE_STACK_MACRO(w_assignto, str) { \
    if (str == NULL) \
        w_assignto = NULL; \
    else { \
        const PHYSFS_uint64 len = (PHYSFS_uint64) ((strlen(str) * 4) + 1); \
        w_assignto = (WCHAR *) __PHYSFS_smallAlloc(len); \
        if (w_assignto != NULL) \
            PHYSFS_utf8ToUcs2(str, (PHYSFS_uint16 *) w_assignto, len); \
    } \
} \

static PHYSFS_uint64 wStrLen(const WCHAR *wstr)
{
    PHYSFS_uint64 len = 0;
    while (*(wstr++))
        len++;
    return(len);
} /* wStrLen */

static char *unicodeToUtf8Heap(const WCHAR *w_str)
{
    char *retval = NULL;
    if (w_str != NULL)
    {
        void *ptr = NULL;
        const PHYSFS_uint64 len = (wStrLen(w_str) * 4) + 1;
        retval = allocator.Malloc(len);
        BAIL_IF_MACRO(retval == NULL, ERR_OUT_OF_MEMORY, NULL);
        PHYSFS_utf8FromUcs2((const PHYSFS_uint16 *) w_str, retval, len);
        ptr = allocator.Realloc(retval, strlen(retval) + 1); /* shrink. */
        if (ptr != NULL)
            retval = (char *) ptr;
    } /* if */
    return(retval);
} /* unicodeToUtf8Heap */


static char *codepageToUtf8Heap(const char *cpstr)
{
    char *retval = NULL;
    if (cpstr != NULL)
    {
        const int len = (int) (strlen(cpstr) + 1);
        WCHAR *wbuf = (WCHAR *) __PHYSFS_smallAlloc(len * sizeof (WCHAR));
        BAIL_IF_MACRO(wbuf == NULL, ERR_OUT_OF_MEMORY, NULL);
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, cpstr, len, wbuf, len);
        retval = (char *) allocator.Malloc(len * 4);
        if (retval == NULL)
            __PHYSFS_setError(ERR_OUT_OF_MEMORY);
        else
            PHYSFS_utf8FromUcs2(wbuf, retval, len * 4);
        __PHYSFS_smallFree(wbuf);
    } /* if */
    return(retval);
} /* codepageToUtf8Heap */


typedef struct
{
    HANDLE handle;
    int readonly;
} WinApiFile;


static char *userDir = NULL;
static int osHasUnicode = 0;


/* pointers for APIs that may not exist on some Windows versions... */
static HANDLE libKernel32 = NULL;
static HANDLE libUserEnv = NULL;
static HANDLE libAdvApi32 = NULL;
static DWORD (WINAPI *pGetModuleFileNameW)(HMODULE, LPWCH, DWORD);
static BOOL (WINAPI *pGetUserProfileDirectoryW)(HANDLE, LPWSTR, LPDWORD);
static BOOL (WINAPI *pGetUserNameW)(LPWSTR, LPDWORD);
static DWORD (WINAPI *pGetFileAttributesW)(LPCWSTR);
static HANDLE (WINAPI *pFindFirstFileW)(LPCWSTR, LPWIN32_FIND_DATAW);
static BOOL (WINAPI *pFindNextFileW)(HANDLE, LPWIN32_FIND_DATAW);
static DWORD (WINAPI *pGetCurrentDirectoryW)(DWORD, LPWSTR);
static BOOL (WINAPI *pDeleteFileW)(LPCWSTR);
static BOOL (WINAPI *pRemoveDirectoryW)(LPCWSTR);
static BOOL (WINAPI *pCreateDirectoryW)(LPCWSTR, LPSECURITY_ATTRIBUTES);
static BOOL (WINAPI *pGetFileAttributesExA)
    (LPCSTR, GET_FILEEX_INFO_LEVELS, LPVOID);
static BOOL (WINAPI *pGetFileAttributesExW)
    (LPCWSTR, GET_FILEEX_INFO_LEVELS, LPVOID);
static DWORD (WINAPI *pFormatMessageW)
    (DWORD, LPCVOID, DWORD, DWORD, LPWSTR, DWORD, va_list *);
static HANDLE (WINAPI *pCreateFileW)
    (LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);


/*
 * Fallbacks for missing Unicode functions on Win95/98/ME. These are filled
 *  into the function pointers if looking up the real Unicode entry points
 *  in the system DLLs fails, so they're never used on WinNT/XP/Vista/etc.
 * They make an earnest effort to convert to/from UTF-8 and UCS-2 to 
 *  the user's current codepage.
 */

static BOOL WINAPI fallbackGetUserNameW(LPWSTR buf, LPDWORD len)
{
    const DWORD cplen = *len;
    char *cpstr = __PHYSFS_smallAlloc(cplen);
    BOOL retval = GetUserNameA(cpstr, len);
    if (buf != NULL)
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, cpstr, cplen, buf, *len);
    __PHYSFS_smallFree(cpstr);
    return(retval);
} /* fallbackGetUserNameW */

static DWORD WINAPI fallbackFormatMessageW(DWORD dwFlags, LPCVOID lpSource,
                                           DWORD dwMessageId, DWORD dwLangId,
                                           LPWSTR lpBuf, DWORD nSize,
                                           va_list *Arguments)
{
    char *cpbuf = (char *) __PHYSFS_smallAlloc(nSize);
    DWORD retval = FormatMessageA(dwFlags, lpSource, dwMessageId, dwLangId,
                                  cpbuf, nSize, Arguments);
    if (retval > 0)
        MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,cpbuf,retval,lpBuf,nSize);
    __PHYSFS_smallFree(cpbuf);
    return(retval);
} /* fallbackFormatMessageW */

static DWORD WINAPI fallbackGetModuleFileNameW(HMODULE hMod, LPWCH lpBuf,
                                               DWORD nSize)
{
    char *cpbuf = (char *) __PHYSFS_smallAlloc(nSize);
    DWORD retval = GetModuleFileNameA(hMod, cpbuf, nSize);
    if (retval > 0)
        MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,cpbuf,retval,lpBuf,nSize);
    __PHYSFS_smallFree(cpbuf);
    return(retval);
} /* fallbackGetModuleFileNameW */

static DWORD WINAPI fallbackGetFileAttributesW(LPCWSTR fname)
{
    DWORD retval = 0;
    const int buflen = (int) (wStrLen(fname) + 1);
    char *cpstr = (char *) __PHYSFS_smallAlloc(buflen);
    WideCharToMultiByte(CP_ACP, 0, fname, buflen, cpstr, buflen, NULL, NULL);
    retval = GetFileAttributesA(cpstr);
    __PHYSFS_smallFree(cpstr);
    return(retval);
} /* fallbackGetFileAttributesW */

static DWORD WINAPI fallbackGetCurrentDirectoryW(DWORD buflen, LPWSTR buf)
{
    DWORD retval = 0;
    char *cpbuf = NULL;
    if (buf != NULL)
        cpbuf = (char *) __PHYSFS_smallAlloc(buflen);
    retval = GetCurrentDirectoryA(buflen, cpbuf);
    if (cpbuf != NULL)
    {
        MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,cpbuf,retval,buf,buflen);
        __PHYSFS_smallFree(cpbuf);
    } /* if */
    return(retval);
} /* fallbackGetCurrentDirectoryW */

static BOOL WINAPI fallbackRemoveDirectoryW(LPCWSTR dname)
{
    BOOL retval = 0;
    const int buflen = (int) (wStrLen(dname) + 1);
    char *cpstr = (char *) __PHYSFS_smallAlloc(buflen);
    WideCharToMultiByte(CP_ACP, 0, dname, buflen, cpstr, buflen, NULL, NULL);
    retval = RemoveDirectoryA(cpstr);
    __PHYSFS_smallFree(cpstr);
    return(retval);
} /* fallbackRemoveDirectoryW */

static BOOL WINAPI fallbackCreateDirectoryW(LPCWSTR dname, 
                                            LPSECURITY_ATTRIBUTES attr)
{
    BOOL retval = 0;
    const int buflen = (int) (wStrLen(dname) + 1);
    char *cpstr = (char *) __PHYSFS_smallAlloc(buflen);
    WideCharToMultiByte(CP_ACP, 0, dname, buflen, cpstr, buflen, NULL, NULL);
    retval = CreateDirectoryA(cpstr, attr);
    __PHYSFS_smallFree(cpstr);
    return(retval);
} /* fallbackCreateDirectoryW */

static BOOL WINAPI fallbackDeleteFileW(LPCWSTR fname)
{
    BOOL retval = 0;
    const int buflen = (int) (wStrLen(fname) + 1);
    char *cpstr = (char *) __PHYSFS_smallAlloc(buflen);
    WideCharToMultiByte(CP_ACP, 0, fname, buflen, cpstr, buflen, NULL, NULL);
    retval = DeleteFileA(cpstr);
    __PHYSFS_smallFree(cpstr);
    return(retval);
} /* fallbackDeleteFileW */

static HANDLE WINAPI fallbackCreateFileW(LPCWSTR fname, 
                DWORD dwDesiredAccess, DWORD dwShareMode,
                LPSECURITY_ATTRIBUTES lpSecurityAttrs,
                DWORD dwCreationDisposition,
                DWORD dwFlagsAndAttrs, HANDLE hTemplFile)
{
    HANDLE retval;
    const int buflen = (int) (wStrLen(fname) + 1);
    char *cpstr = (char *) __PHYSFS_smallAlloc(buflen);
    WideCharToMultiByte(CP_ACP, 0, fname, buflen, cpstr, buflen, NULL, NULL);
    retval = CreateFileA(cpstr, dwDesiredAccess, dwShareMode, lpSecurityAttrs,
                         dwCreationDisposition, dwFlagsAndAttrs, hTemplFile);
    __PHYSFS_smallFree(cpstr);
    return(retval);
} /* fallbackCreateFileW */


/* A blatant abuse of pointer casting... */
static int symLookup(HMODULE dll, void **addr, const char *sym)
{
    return( (*addr = GetProcAddress(dll, sym)) != NULL );
} /* symLookup */


static int findApiSymbols(void)
{
    HMODULE dll = NULL;

    #define LOOKUP_NOFALLBACK(x, reallyLook) { \
        if (reallyLook) \
            symLookup(dll, (void **) &p##x, #x); \
        else \
            p##x = NULL; \
    }

    #define LOOKUP(x, reallyLook) { \
        if ((!reallyLook) || (!symLookup(dll, (void **) &p##x, #x))) \
            p##x = fallback##x; \
    }

    /* Apparently Win9x HAS the Unicode entry points, they just don't WORK. */
    /*  ...so don't look them up unless we're on NT+. (see osHasUnicode.) */

    dll = libUserEnv = LoadLibraryA("userenv.dll");
    if (dll != NULL)
        LOOKUP_NOFALLBACK(GetUserProfileDirectoryW, osHasUnicode);

    /* !!! FIXME: what do they call advapi32.dll on Win64? */
    dll = libAdvApi32 = LoadLibraryA("advapi32.dll");
    if (dll != NULL)
        LOOKUP(GetUserNameW, osHasUnicode);

    /* !!! FIXME: what do they call kernel32.dll on Win64? */
    dll = libKernel32 = LoadLibraryA("kernel32.dll");
    if (dll != NULL)
    {
        LOOKUP_NOFALLBACK(GetFileAttributesExA, 1);
        LOOKUP_NOFALLBACK(GetFileAttributesExW, osHasUnicode);
        LOOKUP_NOFALLBACK(FindFirstFileW, osHasUnicode);
        LOOKUP_NOFALLBACK(FindNextFileW, osHasUnicode);
        LOOKUP(GetModuleFileNameW, osHasUnicode);
        LOOKUP(FormatMessageW, osHasUnicode);
        LOOKUP(GetFileAttributesW, osHasUnicode);
        LOOKUP(GetCurrentDirectoryW, osHasUnicode);
        LOOKUP(CreateDirectoryW, osHasUnicode);
        LOOKUP(RemoveDirectoryW, osHasUnicode);
        LOOKUP(CreateFileW, osHasUnicode);
        LOOKUP(DeleteFileW, osHasUnicode);
    } /* if */

    #undef LOOKUP_NOFALLBACK
    #undef LOOKUP

    return(1);
} /* findApiSymbols */


const char *__PHYSFS_platformDirSeparator = "\\";


/*
 * Figure out what the last failing Windows API call was, and
 *  generate a human-readable string for the error message.
 *
 * The return value is a static buffer that is overwritten with
 *  each call to this function.
 */
static const char *winApiStrError(void)
{
    static char utf8buf[255];
    WCHAR msgbuf[255];
    WCHAR *ptr;
    DWORD rc = pFormatMessageW(
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    msgbuf, __PHYSFS_ARRAYLEN(msgbuf),
                    NULL);

    /* chop off newlines. */
    for (ptr = msgbuf; *ptr; ptr++)
    {
        if ((*ptr == '\n') || (*ptr == '\r'))
        {
            *ptr = '\0';
            break;
        } /* if */
    } /* for */

    /* may truncate, but oh well. */
    PHYSFS_utf8FromUcs2((PHYSFS_uint16 *) msgbuf, utf8buf, sizeof (utf8buf));
    return((const char *) utf8buf);
} /* winApiStrError */


static char *getExePath(void)
{
    DWORD buflen = 64;
    int success = 0;
    LPWSTR modpath = NULL;
    char *retval = NULL;

    while (1)
    {
        DWORD rc;
        void *ptr;

        if ( !(ptr = allocator.Realloc(modpath, buflen*sizeof(WCHAR))) )
        {
            allocator.Free(modpath);
            BAIL_MACRO(ERR_OUT_OF_MEMORY, NULL);
        } /* if */
        modpath = (LPWSTR) ptr;

        rc = pGetModuleFileNameW(NULL, modpath, buflen);
        if (rc == 0)
        {
            allocator.Free(modpath);
            BAIL_MACRO(winApiStrError(), NULL);
        } /* if */

        if (rc < buflen)
        {
            buflen = rc;
            break;
        } /* if */

        buflen *= 2;
    } /* while */

    if (buflen > 0)  /* just in case... */
    {
        WCHAR *ptr = (modpath + buflen) - 1;
        while (ptr != modpath)
        {
            if (*ptr == '\\')
                break;
            ptr--;
        } /* while */

        if ((ptr == modpath) && (*ptr != '\\'))
            __PHYSFS_setError(ERR_GETMODFN_NO_DIR);
        else
        {
            *(ptr + 1) = '\0';  /* chop off filename. */
            retval = unicodeToUtf8Heap(modpath);
        } /* else */
    } /* else */
    allocator.Free(modpath);

    return(retval);   /* w00t. */
} /* getExePath */


/*
 * Try to make use of GetUserProfileDirectoryW(), which isn't available on
 *  some common variants of Win32. If we can't use this, we just punt and
 *  use the physfs base dir for the user dir, too.
 *
 * On success, module-scope variable (userDir) will have a pointer to
 *  a malloc()'d string of the user's profile dir, and a non-zero value is
 *  returned. If we can't determine the profile dir, (userDir) will
 *  be NULL, and zero is returned.
 */
static int determineUserDir(void)
{
    if (userDir != NULL)
        return(1);  /* already good to go. */

    /*
     * GetUserProfileDirectoryW() is only available on NT 4.0 and later.
     *  This means Win95/98/ME (and CE?) users have to do without, so for
     *  them, we'll default to the base directory when we can't get the
     *  function pointer. Since this is originally an NT API, we don't
	 *  offer a non-Unicode fallback.
     */
    if (pGetUserProfileDirectoryW != NULL)
    {
        HANDLE accessToken = NULL;       /* Security handle to process */
        HANDLE processHandle = GetCurrentProcess();
        if (OpenProcessToken(processHandle, TOKEN_QUERY, &accessToken))
        {
            DWORD psize = 0;
            WCHAR dummy = 0;
            LPWSTR wstr = NULL;
            BOOL rc = 0;

            /*
             * Should fail. Will write the size of the profile path in
             *  psize. Also note that the second parameter can't be
             *  NULL or the function fails.
             */	
    		rc = pGetUserProfileDirectoryW(accessToken, &dummy, &psize);
            assert(!rc);  /* !!! FIXME: handle this gracefully. */

            /* Allocate memory for the profile directory */
            wstr = (LPWSTR) __PHYSFS_smallAlloc(psize * sizeof (WCHAR));
            if (wstr != NULL)
            {
                if (pGetUserProfileDirectoryW(accessToken, wstr, &psize))
                    userDir = unicodeToUtf8Heap(wstr);
                __PHYSFS_smallFree(wstr);
            } /* else */
        } /* if */

        CloseHandle(accessToken);
    } /* if */

    if (userDir == NULL)  /* couldn't get profile for some reason. */
    {
        /* Might just be a non-NT system; resort to the basedir. */
        userDir = getExePath();
        BAIL_IF_MACRO(userDir == NULL, NULL, 0); /* STILL failed?! */
    } /* if */

    return(1);  /* We made it: hit the showers. */
} /* determineUserDir */


static BOOL mediaInDrive(const char *drive)
{
    UINT oldErrorMode;
    DWORD tmp;
    BOOL retval;

    /* Prevent windows warning message appearing when checking media size */
    oldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    
    /* If this function succeeds, there's media in the drive */
    retval = GetVolumeInformationA(drive, NULL, 0, NULL, NULL, &tmp, NULL, 0);

    /* Revert back to old windows error handler */
    SetErrorMode(oldErrorMode);

    return(retval);
} /* mediaInDrive */


void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
    /* !!! FIXME: Can CD drives be non-drive letter paths? */
    /* !!! FIXME:  (so can they be Unicode paths?) */
    char drive_str[4] = "x:\\";
    char ch;
    for (ch = 'A'; ch <= 'Z'; ch++)
    {
        drive_str[0] = ch;
        if (GetDriveType(drive_str) == DRIVE_CDROM && mediaInDrive(drive_str))
            cb(data, drive_str);
    } /* for */
} /* __PHYSFS_platformDetectAvailableCDs */


char *__PHYSFS_platformCalcBaseDir(const char *argv0)
{
    if ((argv0 != NULL) && (strchr(argv0, '\\') != NULL))
        return(NULL); /* default behaviour can handle this. */

    return(getExePath());
} /* __PHYSFS_platformCalcBaseDir */


char *__PHYSFS_platformGetUserName(void)
{
    DWORD bufsize = 0;
    char *retval = NULL;
    
    if (pGetUserNameW(NULL, &bufsize) == 0)  /* This SHOULD fail. */
    {
        LPWSTR wbuf = (LPWSTR) __PHYSFS_smallAlloc(bufsize * sizeof (WCHAR));
        BAIL_IF_MACRO(wbuf == NULL, ERR_OUT_OF_MEMORY, NULL);
        if (pGetUserNameW(wbuf, &bufsize) == 0)  /* ?! */
            __PHYSFS_setError(winApiStrError());
        else
            retval = unicodeToUtf8Heap(wbuf);
        __PHYSFS_smallFree(wbuf);
    } /* if */

    return(retval);
} /* __PHYSFS_platformGetUserName */


char *__PHYSFS_platformGetUserDir(void)
{
    char *retval = (char *) allocator.Malloc(strlen(userDir) + 1);
    BAIL_IF_MACRO(retval == NULL, ERR_OUT_OF_MEMORY, NULL);
    strcpy(retval, userDir); /* calculated at init time. */
    return(retval);
} /* __PHYSFS_platformGetUserDir */


PHYSFS_uint64 __PHYSFS_platformGetThreadID(void)
{
    return((PHYSFS_uint64) GetCurrentThreadId());
} /* __PHYSFS_platformGetThreadID */


static int doPlatformExists(LPWSTR wpath)
{
    BAIL_IF_MACRO
    (
        pGetFileAttributesW(wpath) == PHYSFS_INVALID_FILE_ATTRIBUTES,
        winApiStrError(), 0
    );
    return(1);
} /* doPlatformExists */


int __PHYSFS_platformExists(const char *fname)
{
    int retval = 0;
    LPWSTR wpath;
    UTF8_TO_UNICODE_STACK_MACRO(wpath, fname);
    BAIL_IF_MACRO(wpath == NULL, ERR_OUT_OF_MEMORY, 0);
    retval = doPlatformExists(wpath);
    __PHYSFS_smallFree(wpath);
    return(retval);
} /* __PHYSFS_platformExists */


static int isSymlinkAttrs(const DWORD attr, const DWORD tag)
{
    return ( (attr & FILE_ATTRIBUTE_REPARSE_POINT) && 
             (tag == PHYSFS_IO_REPARSE_TAG_SYMLINK) );
} /* isSymlinkAttrs */


int __PHYSFS_platformIsSymLink(const char *fname)
{
    /* !!! FIXME:
     * Windows Vista can have NTFS symlinks. Can older Windows releases have
     *  them when talking to a network file server? What happens when you
     *  mount a NTFS partition on XP that was plugged into a Vista install
     *  that made a symlink?
     */

    int retval = 0;
    LPWSTR wpath;
    HANDLE dir;
    WIN32_FIND_DATAW entw;

    /* no unicode entry points? Probably no symlinks. */
    BAIL_IF_MACRO(pFindFirstFileW == NULL, NULL, 0);

    UTF8_TO_UNICODE_STACK_MACRO(wpath, fname);
    BAIL_IF_MACRO(wpath == NULL, ERR_OUT_OF_MEMORY, 0);

    /* !!! FIXME: filter wildcard chars? */
    dir = pFindFirstFileW(wpath, &entw);
    if (dir != INVALID_HANDLE_VALUE)
    {
        retval = isSymlinkAttrs(entw.dwFileAttributes, entw.dwReserved0);
        FindClose(dir);
    } /* if */

    __PHYSFS_smallFree(wpath);
    return(retval);
} /* __PHYSFS_platformIsSymlink */


int __PHYSFS_platformIsDirectory(const char *fname)
{
    int retval = 0;
    LPWSTR wpath;
    UTF8_TO_UNICODE_STACK_MACRO(wpath, fname);
    BAIL_IF_MACRO(wpath == NULL, ERR_OUT_OF_MEMORY, 0);
    retval = ((pGetFileAttributesW(wpath) & FILE_ATTRIBUTE_DIRECTORY) != 0);
    __PHYSFS_smallFree(wpath);
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


void __PHYSFS_platformEnumerateFiles(const char *dirname,
                                     int omitSymLinks,
                                     PHYSFS_EnumFilesCallback callback,
                                     const char *origdir,
                                     void *callbackdata)
{
    const int unicode = (pFindFirstFileW != NULL) && (pFindNextFileW != NULL);
    HANDLE dir = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA ent;
    WIN32_FIND_DATAW entw;
    size_t len = strlen(dirname);
    char *searchPath = NULL;
    WCHAR *wSearchPath = NULL;
    char *utf8 = NULL;

    /* Allocate a new string for path, maybe '\\', "*", and NULL terminator */
    searchPath = (char *) __PHYSFS_smallAlloc(len + 3);
    if (searchPath == NULL)
        return;

    /* Copy current dirname */
    strcpy(searchPath, dirname);

    /* if there's no '\\' at the end of the path, stick one in there. */
    if (searchPath[len - 1] != '\\')
    {
        searchPath[len++] = '\\';
        searchPath[len] = '\0';
    } /* if */

    /* Append the "*" to the end of the string */
    strcat(searchPath, "*");

    UTF8_TO_UNICODE_STACK_MACRO(wSearchPath, searchPath);
    if (wSearchPath == NULL)
        return;  /* oh well. */

    if (unicode)
        dir = pFindFirstFileW(wSearchPath, &entw);
    else
    {
        const int len = (int) (wStrLen(wSearchPath) + 1);
        char *cp = (char *) __PHYSFS_smallAlloc(len);
        if (cp != NULL)
        {
            WideCharToMultiByte(CP_ACP, 0, wSearchPath, len, cp, len, 0, 0);
            dir = FindFirstFileA(cp, &ent);
            __PHYSFS_smallFree(cp);
        } /* if */
    } /* else */

    __PHYSFS_smallFree(wSearchPath);
    __PHYSFS_smallFree(searchPath);
    if (dir == INVALID_HANDLE_VALUE)
        return;

    if (unicode)
    {
        do
        {
            const DWORD attr = entw.dwFileAttributes;
            const DWORD tag = entw.dwReserved0;
            const WCHAR *fn = entw.cFileName;
            if ((fn[0] == '.') && (fn[1] == '\0'))
                continue;
            if ((fn[0] == '.') && (fn[1] == '.') && (fn[2] == '\0'))
                continue;
            if ((omitSymLinks) && (isSymlinkAttrs(attr, tag)))
                continue;

            utf8 = unicodeToUtf8Heap(fn);
            if (utf8 != NULL)
            {
                callback(callbackdata, origdir, utf8);
                allocator.Free(utf8);
            } /* if */
        } while (pFindNextFileW(dir, &entw) != 0);
    } /* if */

    else  /* ANSI fallback. */
    {
        do
        {
            const DWORD attr = ent.dwFileAttributes;
            const DWORD tag = ent.dwReserved0;
            const char *fn = ent.cFileName;
            if ((fn[0] == '.') && (fn[1] == '\0'))
                continue;
            if ((fn[0] == '.') && (fn[1] == '.') && (fn[2] == '\0'))
                continue;
            if ((omitSymLinks) && (isSymlinkAttrs(attr, tag)))
                continue;

            utf8 = codepageToUtf8Heap(fn);
            if (utf8 != NULL)
            {
                callback(callbackdata, origdir, utf8);
                allocator.Free(utf8);
            } /* if */
        } while (FindNextFileA(dir, &ent) != 0);
    } /* else */

    FindClose(dir);
} /* __PHYSFS_platformEnumerateFiles */


char *__PHYSFS_platformCurrentDir(void)
{
    char *retval = NULL;
    WCHAR *wbuf = NULL;
    DWORD buflen = 0;

    buflen = pGetCurrentDirectoryW(buflen, NULL);
    wbuf = (WCHAR *) __PHYSFS_smallAlloc((buflen + 2) * sizeof (WCHAR));
    BAIL_IF_MACRO(wbuf == NULL, ERR_OUT_OF_MEMORY, NULL);
    pGetCurrentDirectoryW(buflen, wbuf);

    if (wbuf[buflen - 2] == '\\')
        wbuf[buflen-1] = '\0';  /* just in case... */
    else
    {
        wbuf[buflen - 1] = '\\'; 
        wbuf[buflen] = '\0'; 
    } /* else */

    retval = unicodeToUtf8Heap(wbuf);
    __PHYSFS_smallFree(wbuf);
    return(retval);
} /* __PHYSFS_platformCurrentDir */


/* this could probably use a cleanup. */
char *__PHYSFS_platformRealPath(const char *path)
{
    /* !!! FIXME: try GetFullPathName() instead? */
    /* this function should be UTF-8 clean. */
    char *retval = NULL;
    char *p = NULL;

    BAIL_IF_MACRO(path == NULL, ERR_INVALID_ARGUMENT, NULL);
    BAIL_IF_MACRO(*path == '\0', ERR_INVALID_ARGUMENT, NULL);

    retval = (char *) allocator.Malloc(MAX_PATH);
    BAIL_IF_MACRO(retval == NULL, ERR_OUT_OF_MEMORY, NULL);

        /*
         * If in \\server\path format, it's already an absolute path.
         *  We'll need to check for "." and ".." dirs, though, just in case.
         */
    if ((path[0] == '\\') && (path[1] == '\\'))
        strcpy(retval, path);

    else
    {
        char *currentDir = __PHYSFS_platformCurrentDir();
        if (currentDir == NULL)
        {
            allocator.Free(retval);
            BAIL_MACRO(ERR_OUT_OF_MEMORY, NULL);
        } /* if */

        if (path[1] == ':')   /* drive letter specified? */
        {
            /*
             * Apparently, "D:mypath" is the same as "D:\\mypath" if
             *  D: is not the current drive. However, if D: is the
             *  current drive, then "D:mypath" is a relative path. Ugh.
             */
            if (path[2] == '\\')  /* maybe an absolute path? */
                strcpy(retval, path);
            else  /* definitely an absolute path. */
            {
                if (path[0] == currentDir[0]) /* current drive; relative. */
                {
                    strcpy(retval, currentDir);
                    strcat(retval, path + 2);
                } /* if */

                else  /* not current drive; absolute. */
                {
                    retval[0] = path[0];
                    retval[1] = ':';
                    retval[2] = '\\';
                    strcpy(retval + 3, path + 2);
                } /* else */
            } /* else */
        } /* if */

        else  /* no drive letter specified. */
        {
            if (path[0] == '\\')  /* absolute path. */
            {
                retval[0] = currentDir[0];
                retval[1] = ':';
                strcpy(retval + 2, path);
            } /* if */
            else
            {
                strcpy(retval, currentDir);
                strcat(retval, path);
            } /* else */
        } /* else */

        allocator.Free(currentDir);
    } /* else */

    /* (whew.) Ok, now take out "." and ".." path entries... */

    p = retval;
    while ( (p = strstr(p, "\\.")) != NULL)
    {
        /* it's a "." entry that doesn't end the string. */
        if (p[2] == '\\')
            memmove(p + 1, p + 3, strlen(p + 3) + 1);

        /* it's a "." entry that ends the string. */
        else if (p[2] == '\0')
            p[0] = '\0';

        /* it's a ".." entry. */
        else if (p[2] == '.')
        {
            char *prevEntry = p - 1;
            while ((prevEntry != retval) && (*prevEntry != '\\'))
                prevEntry--;

            if (prevEntry == retval)  /* make it look like a "." entry. */
                memmove(p + 1, p + 2, strlen(p + 2) + 1);
            else
            {
                if (p[3] != '\0') /* doesn't end string. */
                    *prevEntry = '\0';
                else /* ends string. */
                    memmove(prevEntry + 1, p + 4, strlen(p + 4) + 1);

                p = prevEntry;
            } /* else */
        } /* else if */

        else
        {
            p++;  /* look past current char. */
        } /* else */
    } /* while */

    /* shrink the retval's memory block if possible... */
    p = (char *) allocator.Realloc(retval, strlen(retval) + 1);
    if (p != NULL)
        retval = p;

    return(retval);
} /* __PHYSFS_platformRealPath */


int __PHYSFS_platformMkDir(const char *path)
{
    WCHAR *wpath;
    DWORD rc;
    UTF8_TO_UNICODE_STACK_MACRO(wpath, path);
    rc = pCreateDirectoryW(wpath, NULL);
    __PHYSFS_smallFree(wpath);
    BAIL_IF_MACRO(rc == 0, winApiStrError(), 0);
    return(1);
} /* __PHYSFS_platformMkDir */


 /*
  * Get OS info and save the important parts.
  *
  * Returns non-zero if successful, otherwise it returns zero on failure.
  */
 static int getOSInfo(void)
 {
     OSVERSIONINFO osVerInfo;     /* Information about the OS */
     osVerInfo.dwOSVersionInfoSize = sizeof(osVerInfo);
     BAIL_IF_MACRO(!GetVersionEx(&osVerInfo), winApiStrError(), 0);
     osHasUnicode = (osVerInfo.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS);
     return(1);
 } /* getOSInfo */


int __PHYSFS_platformInit(void)
{
    BAIL_IF_MACRO(!getOSInfo(), NULL, 0);
    BAIL_IF_MACRO(!findApiSymbols(), NULL, 0);
    BAIL_IF_MACRO(!determineUserDir(), NULL, 0);
    return(1);  /* It's all good */
} /* __PHYSFS_platformInit */


int __PHYSFS_platformDeinit(void)
{
    HANDLE *libs[] = { &libKernel32, &libUserEnv, &libAdvApi32, NULL };
    int i;

    allocator.Free(userDir);
    userDir = NULL;

    for (i = 0; libs[i] != NULL; i++)
    {
        const HANDLE lib = *(libs[i]);
        if (lib)
            FreeLibrary(lib);
        *(libs[i]) = NULL;
    } /* for */

    return(1); /* It's all good */
} /* __PHYSFS_platformDeinit */


static void *doOpen(const char *fname, DWORD mode, DWORD creation, int rdonly)
{
    HANDLE fileHandle;
    WinApiFile *retval;
    WCHAR *wfname;

    UTF8_TO_UNICODE_STACK_MACRO(wfname, fname);
    BAIL_IF_MACRO(wfname == NULL, ERR_OUT_OF_MEMORY, NULL);
    fileHandle = pCreateFileW(wfname, mode, FILE_SHARE_READ, NULL,
                              creation, FILE_ATTRIBUTE_NORMAL, NULL);
    __PHYSFS_smallFree(wfname);

    BAIL_IF_MACRO
    (
        fileHandle == INVALID_HANDLE_VALUE,
        winApiStrError(), NULL
    );

    retval = (WinApiFile *) allocator.Malloc(sizeof (WinApiFile));
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
        HANDLE h = ((WinApiFile *) retval)->handle;
        DWORD rc = SetFilePointer(h, 0, NULL, FILE_END);
        if (rc == PHYSFS_INVALID_SET_FILE_POINTER)
        {
            const char *err = winApiStrError();
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
    HANDLE Handle = ((WinApiFile *) opaque)->handle;
    DWORD CountOfBytesRead;
    PHYSFS_sint64 retval;

    /* Read data from the file */
    /* !!! FIXME: uint32 might be a greater # than DWORD */
    if(!ReadFile(Handle, buffer, count * size, &CountOfBytesRead, NULL))
    {
        BAIL_MACRO(winApiStrError(), -1);
    } /* if */
    else
    {
        /* Return the number of "objects" read. */
        /* !!! FIXME: What if not the right amount of bytes was read to make an object? */
        retval = CountOfBytesRead / size;
    } /* else */

    return(retval);
} /* __PHYSFS_platformRead */


PHYSFS_sint64 __PHYSFS_platformWrite(void *opaque, const void *buffer,
                                     PHYSFS_uint32 size, PHYSFS_uint32 count)
{
    HANDLE Handle = ((WinApiFile *) opaque)->handle;
    DWORD CountOfBytesWritten;
    PHYSFS_sint64 retval;

    /* Read data from the file */
    /* !!! FIXME: uint32 might be a greater # than DWORD */
    if(!WriteFile(Handle, buffer, count * size, &CountOfBytesWritten, NULL))
    {
        BAIL_MACRO(winApiStrError(), -1);
    } /* if */
    else
    {
        /* Return the number of "objects" read. */
        /* !!! FIXME: What if not the right number of bytes was written? */
        retval = CountOfBytesWritten / size;
    } /* else */

    return(retval);
} /* __PHYSFS_platformWrite */


int __PHYSFS_platformSeek(void *opaque, PHYSFS_uint64 pos)
{
    HANDLE Handle = ((WinApiFile *) opaque)->handle;
    DWORD HighOrderPos;
    DWORD *pHighOrderPos;
    DWORD rc;

    /* Get the high order 32-bits of the position */
    HighOrderPos = HIGHORDER_UINT64(pos);

    /*
     * MSDN: "If you do not need the high-order 32 bits, this
     *         pointer must be set to NULL."
     */
    pHighOrderPos = (HighOrderPos) ? &HighOrderPos : NULL;

    /*
     * !!! FIXME: MSDN: "Windows Me/98/95:  If the pointer
     * !!! FIXME:  lpDistanceToMoveHigh is not NULL, then it must
     * !!! FIXME:  point to either 0, INVALID_SET_FILE_POINTER, or
     * !!! FIXME:  the sign extension of the value of lDistanceToMove.
     * !!! FIXME:  Any other value will be rejected."
     */

    /* Move pointer "pos" count from start of file */
    rc = SetFilePointer(Handle, LOWORDER_UINT64(pos),
                        pHighOrderPos, FILE_BEGIN);

    if ( (rc == PHYSFS_INVALID_SET_FILE_POINTER) &&
         (GetLastError() != NO_ERROR) )
    {
        BAIL_MACRO(winApiStrError(), 0);
    } /* if */
    
    return(1);  /* No error occured */
} /* __PHYSFS_platformSeek */


PHYSFS_sint64 __PHYSFS_platformTell(void *opaque)
{
    HANDLE Handle = ((WinApiFile *) opaque)->handle;
    DWORD HighPos = 0;
    DWORD LowPos;
    PHYSFS_sint64 retval;

    /* Get current position */
    LowPos = SetFilePointer(Handle, 0, &HighPos, FILE_CURRENT);
    if ( (LowPos == PHYSFS_INVALID_SET_FILE_POINTER) &&
         (GetLastError() != NO_ERROR) )
    {
        BAIL_MACRO(winApiStrError(), 0);
    } /* if */
    else
    {
        /* Combine the high/low order to create the 64-bit position value */
        retval = (((PHYSFS_uint64) HighPos) << 32) | LowPos;
        assert(retval >= 0);
    } /* else */

    return(retval);
} /* __PHYSFS_platformTell */


PHYSFS_sint64 __PHYSFS_platformFileLength(void *opaque)
{
    HANDLE Handle = ((WinApiFile *) opaque)->handle;
    DWORD SizeHigh;
    DWORD SizeLow;
    PHYSFS_sint64 retval;

    SizeLow = GetFileSize(Handle, &SizeHigh);
    if ( (SizeLow == PHYSFS_INVALID_SET_FILE_POINTER) &&
         (GetLastError() != NO_ERROR) )
    {
        BAIL_MACRO(winApiStrError(), -1);
    } /* if */
    else
    {
        /* Combine the high/low order to create the 64-bit position value */
        retval = (((PHYSFS_uint64) SizeHigh) << 32) | SizeLow;
        assert(retval >= 0);
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
    WinApiFile *fh = ((WinApiFile *) opaque);
    if (!fh->readonly)
        BAIL_IF_MACRO(!FlushFileBuffers(fh->handle), winApiStrError(), 0);

    return(1);
} /* __PHYSFS_platformFlush */


int __PHYSFS_platformClose(void *opaque)
{
    HANDLE Handle = ((WinApiFile *) opaque)->handle;
    BAIL_IF_MACRO(!CloseHandle(Handle), winApiStrError(), 0);
    allocator.Free(opaque);
    return(1);
} /* __PHYSFS_platformClose */


static int doPlatformDelete(LPWSTR wpath)
{
    /* If filename is a folder */
    if (pGetFileAttributesW(wpath) == FILE_ATTRIBUTE_DIRECTORY)
    {
        BAIL_IF_MACRO(!pRemoveDirectoryW(wpath), winApiStrError(), 0);
    } /* if */
    else
    {
        BAIL_IF_MACRO(!pDeleteFileW(wpath), winApiStrError(), 0);
    } /* else */

    return(1);   /* if you made it here, it worked. */
} /* doPlatformDelete */


int __PHYSFS_platformDelete(const char *path)
{
    int retval = 0;
    LPWSTR wpath;
    UTF8_TO_UNICODE_STACK_MACRO(wpath, path);
    BAIL_IF_MACRO(wpath == NULL, ERR_OUT_OF_MEMORY, 0);
    retval = doPlatformDelete(wpath);
    __PHYSFS_smallFree(wpath);
    return(retval);
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


static PHYSFS_sint64 FileTimeToPhysfsTime(const FILETIME *ft)
{
    SYSTEMTIME st_utc;
    SYSTEMTIME st_localtz;
    TIME_ZONE_INFORMATION tzi;
    DWORD tzid;
    PHYSFS_sint64 retval;
    struct tm tm;

    BAIL_IF_MACRO(!FileTimeToSystemTime(ft, &st_utc), winApiStrError(), -1);
    tzid = GetTimeZoneInformation(&tzi);
    BAIL_IF_MACRO(tzid == TIME_ZONE_ID_INVALID, winApiStrError(), -1);

    /* (This API is unsupported and fails on non-NT systems. */
    if (!SystemTimeToTzSpecificLocalTime(&tzi, &st_utc, &st_localtz))
    {
        /* do it by hand. Grumble... */
        ULARGE_INTEGER ui64;
        FILETIME new_ft;
        ui64.LowPart = ft->dwLowDateTime;
        ui64.HighPart = ft->dwHighDateTime;

        if (tzid == TIME_ZONE_ID_STANDARD)
            tzi.Bias += tzi.StandardBias;
        else if (tzid == TIME_ZONE_ID_DAYLIGHT)
            tzi.Bias += tzi.DaylightBias;

        /* convert from minutes to 100-nanosecond increments... */
        ui64.QuadPart -= (((LONGLONG) tzi.Bias) * (600000000));

        /* Move it back into a FILETIME structure... */
        new_ft.dwLowDateTime = ui64.LowPart;
        new_ft.dwHighDateTime = ui64.HighPart;

        /* Convert to something human-readable... */
        if (!FileTimeToSystemTime(&new_ft, &st_localtz))
            BAIL_MACRO(winApiStrError(), -1);
    } /* if */

    /* Convert to a format that mktime() can grok... */
    tm.tm_sec = st_localtz.wSecond;
    tm.tm_min = st_localtz.wMinute;
    tm.tm_hour = st_localtz.wHour;
    tm.tm_mday = st_localtz.wDay;
    tm.tm_mon = st_localtz.wMonth - 1;
    tm.tm_year = st_localtz.wYear - 1900;
    tm.tm_wday = -1 /*st_localtz.wDayOfWeek*/;
    tm.tm_yday = -1;
    tm.tm_isdst = -1;

    /* Convert to a format PhysicsFS can grok... */
    retval = (PHYSFS_sint64) mktime(&tm);
    BAIL_IF_MACRO(retval == -1, strerror(errno), -1);
    return(retval);
} /* FileTimeToPhysfsTime */


PHYSFS_sint64 __PHYSFS_platformGetLastModTime(const char *fname)
{
    PHYSFS_sint64 retval = -1;
    WIN32_FILE_ATTRIBUTE_DATA attr;
    int rc = 0;

    memset(&attr, '\0', sizeof (attr));

    /* GetFileAttributesEx didn't show up until Win98 and NT4. */
    if ((pGetFileAttributesExW != NULL) || (pGetFileAttributesExA != NULL))
    {
        WCHAR *wstr;
        UTF8_TO_UNICODE_STACK_MACRO(wstr, fname);
        if (wstr != NULL) /* if NULL, maybe the fallback will work. */
        {
            if (pGetFileAttributesExW != NULL)  /* NT/XP/Vista/etc system. */
                rc = pGetFileAttributesExW(wstr, GetFileExInfoStandard, &attr);
            else  /* Win98/ME system */
            {
                const int len = (int) (wStrLen(wstr) + 1);
                char *cp = (char *) __PHYSFS_smallAlloc(len);
                if (cp != NULL)
                {
                    WideCharToMultiByte(CP_ACP, 0, wstr, len, cp, len, 0, 0);
                    rc = pGetFileAttributesExA(cp, GetFileExInfoStandard, &attr);
                    __PHYSFS_smallFree(cp);
                } /* if */
            } /* else */
            __PHYSFS_smallFree(wstr);
        } /* if */
    } /* if */

    if (rc)  /* had API entry point and it worked. */
    {
        /* 0 return value indicates an error or not supported */
        if ( (attr.ftLastWriteTime.dwHighDateTime != 0) ||
             (attr.ftLastWriteTime.dwLowDateTime != 0) )
        {
            retval = FileTimeToPhysfsTime(&attr.ftLastWriteTime);
        } /* if */
    } /* if */

    /* GetFileTime() has been in the Win32 API since the start. */
    if (retval == -1)  /* try a fallback... */
    {
        FILETIME ft;
        BOOL rc;
        const char *err;
        WinApiFile *f = (WinApiFile *) __PHYSFS_platformOpenRead(fname);
        BAIL_IF_MACRO(f == NULL, NULL, -1)
        rc = GetFileTime(f->handle, NULL, NULL, &ft);
        err = winApiStrError();
        CloseHandle(f->handle);
        allocator.Free(f);
        BAIL_IF_MACRO(!rc, err, -1);
        retval = FileTimeToPhysfsTime(&ft);
    } /* if */

    return(retval);
} /* __PHYSFS_platformGetLastModTime */


/* !!! FIXME: Don't use C runtime for allocators? */
int __PHYSFS_platformSetDefaultAllocator(PHYSFS_Allocator *a)
{
    return(0);  /* just use malloc() and friends. */
} /* __PHYSFS_platformSetDefaultAllocator */

#endif  /* PHYSFS_PLATFORM_WINDOWS */

/* end of windows.c ... */


