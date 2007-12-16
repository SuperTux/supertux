/*
 * Unix support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_platforms.h"

#ifdef PHYSFS_PLATFORM_UNIX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <sys/mount.h>

#if (!defined PHYSFS_NO_PTHREADS_SUPPORT)
#include <pthread.h>
#endif

#ifdef PHYSFS_HAVE_SYS_UCRED_H
#  ifdef PHYSFS_HAVE_MNTENT_H
#    undef PHYSFS_HAVE_MNTENT_H /* don't do both... */
#  endif
#  include <sys/ucred.h>
#endif

#ifdef PHYSFS_HAVE_MNTENT_H
#include <mntent.h>
#endif

#include "physfs_internal.h"


int __PHYSFS_platformInit(void)
{
    return(1);  /* always succeed. */
} /* __PHYSFS_platformInit */


int __PHYSFS_platformDeinit(void)
{
    return(1);  /* always succeed. */
} /* __PHYSFS_platformDeinit */


#ifdef PHYSFS_NO_CDROM_SUPPORT

/* Stub version for platforms without CD-ROM support. */
void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
} /* __PHYSFS_platformDetectAvailableCDs */

#elif (defined PHYSFS_HAVE_SYS_UCRED_H)

void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
    int i;
    struct statfs *mntbufp = NULL;
    int mounts = getmntinfo(&mntbufp, MNT_WAIT);

    for (i = 0; i < mounts; i++)
    {
        int add_it = 0;

        if (strcmp(mntbufp[i].f_fstypename, "iso9660") == 0)
            add_it = 1;
        else if (strcmp( mntbufp[i].f_fstypename, "cd9660") == 0)
            add_it = 1;

        /* add other mount types here */

        if (add_it)
            cb(data, mntbufp[i].f_mntonname);
    } /* for */
} /* __PHYSFS_platformDetectAvailableCDs */

#elif (defined PHYSFS_HAVE_MNTENT_H)

void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
    FILE *mounts = NULL;
    struct mntent *ent = NULL;

    mounts = setmntent("/etc/mtab", "r");
    BAIL_IF_MACRO(mounts == NULL, ERR_IO_ERROR, /*return void*/);

    while ( (ent = getmntent(mounts)) != NULL )
    {
        int add_it = 0;
        if (strcmp(ent->mnt_type, "iso9660") == 0)
            add_it = 1;

        /* add other mount types here */

        if (add_it)
            cb(data, ent->mnt_dir);
    } /* while */

    endmntent(mounts);

} /* __PHYSFS_platformDetectAvailableCDs */

#endif


/* this is in posix.c ... */
extern char *__PHYSFS_platformCopyEnvironmentVariable(const char *varname);


/*
 * See where program (bin) resides in the $PATH specified by (envr).
 *  returns a copy of the first element in envr that contains it, or NULL
 *  if it doesn't exist or there were other problems. PHYSFS_SetError() is
 *  called if we have a problem.
 *
 * (envr) will be scribbled over, and you are expected to allocator.Free() the
 *  return value when you're done with it.
 */
static char *findBinaryInPath(const char *bin, char *envr)
{
    size_t alloc_size = 0;
    char *exe = NULL;
    char *start = envr;
    char *ptr;

    BAIL_IF_MACRO(bin == NULL, ERR_INVALID_ARGUMENT, NULL);
    BAIL_IF_MACRO(envr == NULL, ERR_INVALID_ARGUMENT, NULL);

    do
    {
        size_t size;
        ptr = strchr(start, ':');  /* find next $PATH separator. */
        if (ptr)
            *ptr = '\0';

        size = strlen(start) + strlen(bin) + 2;
        if (size > alloc_size)
        {
            char *x = (char *) allocator.Realloc(exe, size);
            if (x == NULL)
            {
                if (exe != NULL)
                    allocator.Free(exe);
                BAIL_MACRO(ERR_OUT_OF_MEMORY, NULL);
            } /* if */

            alloc_size = size;
            exe = x;
        } /* if */

        /* build full binary path... */
        strcpy(exe, start);
        if ((exe[0] == '\0') || (exe[strlen(exe) - 1] != '/'))
            strcat(exe, "/");
        strcat(exe, bin);

        if (access(exe, X_OK) == 0)  /* Exists as executable? We're done. */
        {
            strcpy(exe, start);  /* i'm lazy. piss off. */
            return(exe);
        } /* if */

        start = ptr + 1;  /* start points to beginning of next element. */
    } while (ptr != NULL);

    if (exe != NULL)
        allocator.Free(exe);

    return(NULL);  /* doesn't exist in path. */
} /* findBinaryInPath */


char *__PHYSFS_platformCalcBaseDir(const char *argv0)
{
    const char *PROC_SELF_EXE = "/proc/self/exe";
    char *retval = NULL;
    char *envr = NULL;
    struct stat stbuf;

    /* fast path: default behaviour can handle this. */
    if ( (argv0 != NULL) && (strchr(argv0, '/') != NULL) )
        return(NULL);  /* higher level will parse out real path from argv0. */

    /*
     * Try to avoid using argv0 unless forced to. If there's a Linux-like
     *  /proc filesystem, you can get the full path to the current process from
     *  the /proc/self/exe symlink.
     */
    if ((lstat(PROC_SELF_EXE, &stbuf) != -1) && (S_ISLNK(stbuf.st_mode)))
    {
        const size_t len = stbuf.st_size;
        char *buf = (char *) allocator.Malloc(len+1);
        if (buf != NULL)  /* if NULL, maybe you'll get lucky later. */
        {
            if (readlink(PROC_SELF_EXE, buf, len) != len)
                allocator.Free(buf);
            else
            {
                buf[len] = '\0';  /* readlink doesn't null-terminate. */
                retval = buf;  /* we're good to go. */
            } /* else */
        } /* if */
    } /* if */

    if ((retval == NULL) && (argv0 != NULL))
    {
        /* If there's no dirsep on argv0, then look through $PATH for it. */
        envr = __PHYSFS_platformCopyEnvironmentVariable("PATH");
        BAIL_IF_MACRO(!envr, NULL, NULL);
        retval = findBinaryInPath(argv0, envr);
        allocator.Free(envr);
    } /* if */

    return(retval);
} /* __PHYSFS_platformCalcBaseDir */


char *__PHYSFS_platformRealPath(const char *path)
{
    char resolved_path[MAXPATHLEN];
    char *retval = NULL;

    errno = 0;
    BAIL_IF_MACRO(!realpath(path, resolved_path), strerror(errno), NULL);
    retval = (char *) allocator.Malloc(strlen(resolved_path) + 1);
    BAIL_IF_MACRO(retval == NULL, ERR_OUT_OF_MEMORY, NULL);
    strcpy(retval, resolved_path);

    return(retval);
} /* __PHYSFS_platformRealPath */


char *__PHYSFS_platformCurrentDir(void)
{
    /*
     * This can't just do platformRealPath("."), since that would eventually
     *  just end up calling back into here.
     */

    int allocSize = 0;
    char *retval = NULL;
    char *ptr;

    do
    {
        allocSize += 100;
        ptr = (char *) allocator.Realloc(retval, allocSize);
        if (ptr == NULL)
        {
            if (retval != NULL)
                allocator.Free(retval);
            BAIL_MACRO(ERR_OUT_OF_MEMORY, NULL);
        } /* if */

        retval = ptr;
        ptr = getcwd(retval, allocSize);
    } while (ptr == NULL && errno == ERANGE);

    if (ptr == NULL && errno)
    {
            /*
             * getcwd() failed for some reason, for example current
             * directory not existing.
             */
        if (retval != NULL)
            allocator.Free(retval);
        BAIL_MACRO(ERR_NO_SUCH_FILE, NULL);
    } /* if */

    return(retval);
} /* __PHYSFS_platformCurrentDir */


int __PHYSFS_platformSetDefaultAllocator(PHYSFS_Allocator *a)
{
    return(0);  /* just use malloc() and friends. */
} /* __PHYSFS_platformSetDefaultAllocator */


#if (defined PHYSFS_NO_PTHREADS_SUPPORT)

PHYSFS_uint64 __PHYSFS_platformGetThreadID(void) { return(0x0001); }
void *__PHYSFS_platformCreateMutex(void) { return((void *) 0x0001); }
void __PHYSFS_platformDestroyMutex(void *mutex) {}
int __PHYSFS_platformGrabMutex(void *mutex) { return(1); }
void __PHYSFS_platformReleaseMutex(void *mutex) {}

#else

typedef struct
{
    pthread_mutex_t mutex;
    pthread_t owner;
    PHYSFS_uint32 count;
} PthreadMutex;

/* Just in case; this is a panic value. */
#if ((!defined SIZEOF_INT) || (SIZEOF_INT <= 0))
#  define SIZEOF_INT 4
#endif

#if (SIZEOF_INT == 4)
#  define PHTREAD_TO_UI64(thr) ( (PHYSFS_uint64) ((PHYSFS_uint32) (thr)) )
#elif (SIZEOF_INT == 2)
#  define PHTREAD_TO_UI64(thr) ( (PHYSFS_uint64) ((PHYSFS_uint16) (thr)) )
#elif (SIZEOF_INT == 1)
#  define PHTREAD_TO_UI64(thr) ( (PHYSFS_uint64) ((PHYSFS_uint8) (thr)) )
#else
#  define PHTREAD_TO_UI64(thr) ((PHYSFS_uint64) (thr))
#endif

PHYSFS_uint64 __PHYSFS_platformGetThreadID(void)
{
    return(PHTREAD_TO_UI64(pthread_self()));
} /* __PHYSFS_platformGetThreadID */


void *__PHYSFS_platformCreateMutex(void)
{
    int rc;
    PthreadMutex *m = (PthreadMutex *) allocator.Malloc(sizeof (PthreadMutex));
    BAIL_IF_MACRO(m == NULL, ERR_OUT_OF_MEMORY, NULL);
    rc = pthread_mutex_init(&m->mutex, NULL);
    if (rc != 0)
    {
        allocator.Free(m);
        BAIL_MACRO(strerror(rc), NULL);
    } /* if */

    m->count = 0;
    m->owner = (pthread_t) 0xDEADBEEF;
    return((void *) m);
} /* __PHYSFS_platformCreateMutex */


void __PHYSFS_platformDestroyMutex(void *mutex)
{
    PthreadMutex *m = (PthreadMutex *) mutex;

    /* Destroying a locked mutex is a bug, but we'll try to be helpful. */
    if ((m->owner == pthread_self()) && (m->count > 0))
        pthread_mutex_unlock(&m->mutex);

    pthread_mutex_destroy(&m->mutex);
    allocator.Free(m);
} /* __PHYSFS_platformDestroyMutex */


int __PHYSFS_platformGrabMutex(void *mutex)
{
    PthreadMutex *m = (PthreadMutex *) mutex;
    pthread_t tid = pthread_self();
    if (m->owner != tid)
    {
        if (pthread_mutex_lock(&m->mutex) != 0)
            return(0);
        m->owner = tid;
    } /* if */

    m->count++;
    return(1);
} /* __PHYSFS_platformGrabMutex */


void __PHYSFS_platformReleaseMutex(void *mutex)
{
    PthreadMutex *m = (PthreadMutex *) mutex;
    if (m->owner == pthread_self())
    {
        if (--m->count == 0)
        {
            m->owner = (pthread_t) 0xDEADBEEF;
            pthread_mutex_unlock(&m->mutex);
        } /* if */
    } /* if */
} /* __PHYSFS_platformReleaseMutex */

#endif /* !PHYSFS_NO_PTHREADS_SUPPORT */

#endif /* PHYSFS_PLATFORM_UNIX */

/* end of unix.c ... */

