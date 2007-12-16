/*
 * BeOS platform-dependent support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_platforms.h"

#ifdef PHYSFS_PLATFORM_BEOS

#include <be/kernel/OS.h>
#include <be/app/Roster.h>
#include <be/storage/Volume.h>
#include <be/storage/VolumeRoster.h>
#include <be/storage/Directory.h>
#include <be/storage/Entry.h>
#include <be/storage/Path.h>
#include <be/kernel/fs_info.h>
#include <be/device/scsi.h>
#include <be/support/Locker.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "physfs_internal.h"


int __PHYSFS_platformInit(void)
{
    return(1);  /* always succeed. */
} /* __PHYSFS_platformInit */


int __PHYSFS_platformDeinit(void)
{
    return(1);  /* always succeed. */
} /* __PHYSFS_platformDeinit */


static char *getMountPoint(const char *devname)
{
    BVolumeRoster mounts;
    BVolume vol;

    mounts.Rewind();
    while (mounts.GetNextVolume(&vol) == B_NO_ERROR)
    {
        fs_info fsinfo;
        fs_stat_dev(vol.Device(), &fsinfo);
        if (strcmp(devname, fsinfo.device_name) == 0)
        {
            //char buf[B_FILE_NAME_LENGTH];
            BDirectory directory;
            BEntry entry;
            BPath path;
            status_t rc;
            rc = vol.GetRootDirectory(&directory);
            BAIL_IF_MACRO(rc < B_OK, strerror(rc), NULL);
            rc = directory.GetEntry(&entry);
            BAIL_IF_MACRO(rc < B_OK, strerror(rc), NULL);
            rc = entry.GetPath(&path);
            BAIL_IF_MACRO(rc < B_OK, strerror(rc), NULL);
            const char *str = path.Path();
            BAIL_IF_MACRO(str == NULL, ERR_OS_ERROR, NULL);  /* ?! */
            char *retval = (char *) allocator.Malloc(strlen(str) + 1);
            BAIL_IF_MACRO(retval == NULL, ERR_OUT_OF_MEMORY, NULL);
            strcpy(retval, str);
            return(retval);
        } /* if */
    } /* while */

    return(NULL);
} /* getMountPoint */


    /*
     * This function is lifted from Simple Directmedia Layer (SDL):
     *  http://www.libsdl.org/
     */
static void tryDir(const char *d, PHYSFS_StringCallback callback, void *data)
{
    BDirectory dir;
    dir.SetTo(d);
    if (dir.InitCheck() != B_NO_ERROR)
        return;

    dir.Rewind();
    BEntry entry;
    while (dir.GetNextEntry(&entry) >= 0)
    {
        BPath path;
        const char *name;
        entry_ref e;

        if (entry.GetPath(&path) != B_NO_ERROR)
            continue;

        name = path.Path();

        if (entry.GetRef(&e) != B_NO_ERROR)
            continue;

        if (entry.IsDirectory())
        {
            if (strcmp(e.name, "floppy") != 0)
                tryDir(name, callback, data);
        } /* if */

        else
        {
            bool add_it = false;
            int devfd;
            device_geometry g;

            if (strcmp(e.name, "raw") == 0)  /* ignore partitions. */
            {
                int devfd = open(name, O_RDONLY);
                if (devfd >= 0)
                {
                    if (ioctl(devfd, B_GET_GEOMETRY, &g, sizeof(g)) >= 0)
                    {
                        if (g.device_type == B_CD)
                        {
                            char *mntpnt = getMountPoint(name);
                            if (mntpnt != NULL)
                            {
                                callback(data, mntpnt);
                                allocator.Free(mntpnt);  /* !!! FIXME: lose this malloc! */
                            } /* if */
                        } /* if */
                    } /* if */
                } /* if */
            } /* if */

            close(devfd);
        } /* else */
    } /* while */
} /* tryDir */


void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
    tryDir("/dev/disk", cb, data);
} /* __PHYSFS_platformDetectAvailableCDs */


static team_id getTeamID(void)
{
    thread_info info;
    thread_id tid = find_thread(NULL);
    get_thread_info(tid, &info);
    return(info.team);
} /* getTeamID */


char *__PHYSFS_platformCalcBaseDir(const char *argv0)
{
    /* in case there isn't a BApplication yet, we'll construct a roster. */
    BRoster roster; 
    app_info info;
    status_t rc = roster.GetRunningAppInfo(getTeamID(), &info);
    BAIL_IF_MACRO(rc < B_OK, strerror(rc), NULL);
    BEntry entry(&(info.ref), true);
    BPath path;
    rc = entry.GetPath(&path);  /* (path) now has binary's path. */
    assert(rc == B_OK);
    rc = path.GetParent(&path); /* chop filename, keep directory. */
    assert(rc == B_OK);
    const char *str = path.Path();
    assert(str != NULL);
    char *retval = (char *) allocator.Malloc(strlen(str) + 1);
    BAIL_IF_MACRO(retval == NULL, ERR_OUT_OF_MEMORY, NULL);
    strcpy(retval, str);
    return(retval);
} /* __PHYSFS_platformCalcBaseDir */


PHYSFS_uint64 __PHYSFS_platformGetThreadID(void)
{
    return((PHYSFS_uint64) find_thread(NULL));
} /* __PHYSFS_platformGetThreadID */


char *__PHYSFS_platformRealPath(const char *path)
{
    BPath normalized(path, NULL, true);  /* force normalization of path. */
    const char *resolved_path = normalized.Path();
    BAIL_IF_MACRO(resolved_path == NULL, ERR_NO_SUCH_FILE, NULL);
    char *retval = (char *) allocator.Malloc(strlen(resolved_path) + 1);
    BAIL_IF_MACRO(retval == NULL, ERR_OUT_OF_MEMORY, NULL);
    strcpy(retval, resolved_path);
    return(retval);
} /* __PHYSFS_platformRealPath */


char *__PHYSFS_platformCurrentDir(void)
{
    return(__PHYSFS_platformRealPath("."));  /* let BPath sort it out. */
} /* __PHYSFS_platformCurrentDir */


void *__PHYSFS_platformCreateMutex(void)
{
    return(new BLocker("PhysicsFS lock", true));
} /* __PHYSFS_platformCreateMutex */


void __PHYSFS_platformDestroyMutex(void *mutex)
{
    delete ((BLocker *) mutex);
} /* __PHYSFS_platformDestroyMutex */


int __PHYSFS_platformGrabMutex(void *mutex)
{
    return ((BLocker *) mutex)->Lock() ? 1 : 0;
} /* __PHYSFS_platformGrabMutex */


void __PHYSFS_platformReleaseMutex(void *mutex)
{
    ((BLocker *) mutex)->Unlock();
} /* __PHYSFS_platformReleaseMutex */


int __PHYSFS_platformSetDefaultAllocator(PHYSFS_Allocator *a)
{
    return(0);  /* just use malloc() and friends. */
} /* __PHYSFS_platformSetDefaultAllocator */

#endif  /* PHYSFS_PLATFORM_BEOS */

/* end of beos.cpp ... */

