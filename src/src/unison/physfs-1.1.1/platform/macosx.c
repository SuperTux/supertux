/*
 * Mac OS X support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_platforms.h"

#ifdef PHYSFS_PLATFORM_MACOSX

#include <Carbon/Carbon.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOCDMedia.h>
#include <IOKit/storage/IODVDMedia.h>
#include <sys/mount.h>

/* Seems to get defined in some system header... */
#ifdef Free
#undef Free
#endif

#include "physfs_internal.h"


/* Wrap PHYSFS_Allocator in a CFAllocator... */
static CFAllocatorRef cfallocator = NULL;

CFStringRef cfallocDesc(const void *info)
{
    return(CFStringCreateWithCString(cfallocator, "PhysicsFS",
                                     kCFStringEncodingASCII));
} /* cfallocDesc */


static void *cfallocMalloc(CFIndex allocSize, CFOptionFlags hint, void *info)
{
    return allocator.Malloc(allocSize);
} /* cfallocMalloc */


static void cfallocFree(void *ptr, void *info)
{
    allocator.Free(ptr);
} /* cfallocFree */


static void *cfallocRealloc(void *ptr, CFIndex newsize,
                            CFOptionFlags hint, void *info)
{
    if ((ptr == NULL) || (newsize <= 0))
        return NULL;  /* ADC docs say you should always return NULL here. */
    return allocator.Realloc(ptr, newsize);
} /* cfallocRealloc */


int __PHYSFS_platformInit(void)
{
    /* set up a CFAllocator, so Carbon can use the physfs allocator, too. */
    CFAllocatorContext ctx;
    memset(&ctx, '\0', sizeof (ctx));
    ctx.copyDescription = cfallocDesc;
    ctx.allocate = cfallocMalloc;
    ctx.reallocate = cfallocRealloc;
    ctx.deallocate = cfallocFree;
    cfallocator = CFAllocatorCreate(kCFAllocatorUseContext, &ctx);
    BAIL_IF_MACRO(cfallocator == NULL, ERR_OUT_OF_MEMORY, 0);
    return(1);  /* success. */
} /* __PHYSFS_platformInit */


int __PHYSFS_platformDeinit(void)
{
    CFRelease(cfallocator);
    cfallocator = NULL;
    return(1);  /* always succeed. */
} /* __PHYSFS_platformDeinit */


/* CD-ROM detection code... */

/*
 * Code based on sample from Apple Developer Connection:
 *  http://developer.apple.com/samplecode/Sample_Code/Devices_and_Hardware/Disks/VolumeToBSDNode/VolumeToBSDNode.c.htm
 */

static int darwinIsWholeMedia(io_service_t service)
{
    int retval = 0;
    CFTypeRef wholeMedia;

    if (!IOObjectConformsTo(service, kIOMediaClass))
        return(0);
        
    wholeMedia = IORegistryEntryCreateCFProperty(service,
                                                 CFSTR(kIOMediaWholeKey),
                                                 cfallocator, 0);
    if (wholeMedia == NULL)
        return(0);

    retval = CFBooleanGetValue(wholeMedia);
    CFRelease(wholeMedia);

    return retval;
} /* darwinIsWholeMedia */


static int darwinIsMountedDisc(char *bsdName, mach_port_t masterPort)
{
    int retval = 0;
    CFMutableDictionaryRef matchingDict;
    kern_return_t rc;
    io_iterator_t iter;
    io_service_t service;

    if ((matchingDict = IOBSDNameMatching(masterPort, 0, bsdName)) == NULL)
        return(0);

    rc = IOServiceGetMatchingServices(masterPort, matchingDict, &iter);
    if ((rc != KERN_SUCCESS) || (!iter))
        return(0);

    service = IOIteratorNext(iter);
    IOObjectRelease(iter);
    if (!service)
        return(0);

    rc = IORegistryEntryCreateIterator(service, kIOServicePlane,
             kIORegistryIterateRecursively | kIORegistryIterateParents, &iter);
    
    if (!iter)
        return(0);

    if (rc != KERN_SUCCESS)
    {
        IOObjectRelease(iter);
        return(0);
    } /* if */

    IOObjectRetain(service);  /* add an extra object reference... */

    do
    {
        if (darwinIsWholeMedia(service))
        {
            if ( (IOObjectConformsTo(service, kIOCDMediaClass)) ||
                 (IOObjectConformsTo(service, kIODVDMediaClass)) )
            {
                retval = 1;
            } /* if */
        } /* if */
        IOObjectRelease(service);
    } while ((service = IOIteratorNext(iter)) && (!retval));
                
    IOObjectRelease(iter);
    IOObjectRelease(service);

    return(retval);
} /* darwinIsMountedDisc */


void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
    const char *devPrefix = "/dev/";
    const int prefixLen = strlen(devPrefix);
    mach_port_t masterPort = 0;
    struct statfs *mntbufp;
    int i, mounts;

    if (IOMasterPort(MACH_PORT_NULL, &masterPort) != KERN_SUCCESS)
        BAIL_MACRO(ERR_OS_ERROR, ) /*return void*/;

    mounts = getmntinfo(&mntbufp, MNT_WAIT);  /* NOT THREAD SAFE! */
    for (i = 0; i < mounts; i++)
    {
        char *dev = mntbufp[i].f_mntfromname;
        char *mnt = mntbufp[i].f_mntonname;
        if (strncmp(dev, devPrefix, prefixLen) != 0)  /* a virtual device? */
            continue;

        dev += prefixLen;
        if (darwinIsMountedDisc(dev, masterPort))
            cb(data, mnt);
    } /* for */
} /* __PHYSFS_platformDetectAvailableCDs */


static char *convertCFString(CFStringRef cfstr)
{
    CFIndex len = CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfstr),
                                                    kCFStringEncodingUTF8) + 1;
    char *retval = (char *) allocator.Malloc(len);
    BAIL_IF_MACRO(retval == NULL, ERR_OUT_OF_MEMORY, NULL);

    if (CFStringGetCString(cfstr, retval, len, kCFStringEncodingUTF8))
    {
        /* shrink overallocated buffer if possible... */
        CFIndex newlen = strlen(retval) + 1;
        if (newlen < len)
        {
            void *ptr = allocator.Realloc(retval, newlen);
            if (ptr != NULL)
                retval = (char *) ptr;
        } /* if */
    } /* if */

    else  /* probably shouldn't fail, but just in case... */
    {
        allocator.Free(retval);
        BAIL_MACRO(ERR_OUT_OF_MEMORY, NULL);
    } /* else */

    return(retval);
} /* convertCFString */


char *__PHYSFS_platformCalcBaseDir(const char *argv0)
{
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    FSRef fsref;
    CFRange cfrange;
    CFURLRef cfurl = NULL;
    CFStringRef cfstr = NULL;
    CFMutableStringRef cfmutstr = NULL;
    char *retval = NULL;

    BAIL_IF_MACRO(GetProcessBundleLocation(&psn, &fsref) != noErr, NULL, NULL);
    cfurl = CFURLCreateFromFSRef(cfallocator, &fsref);
    BAIL_IF_MACRO(cfurl == NULL, NULL, NULL);
    cfstr = CFURLCopyFileSystemPath(cfurl, kCFURLPOSIXPathStyle);
    CFRelease(cfurl);
    BAIL_IF_MACRO(cfstr == NULL, NULL, NULL);
    cfmutstr = CFStringCreateMutableCopy(cfallocator, 0, cfstr);
    CFRelease(cfstr);
    BAIL_IF_MACRO(cfmutstr == NULL, NULL, NULL);

    /* Find last dirsep so we can chop the binary's filename from the path. */
    cfrange = CFStringFind(cfmutstr, CFSTR("/"), kCFCompareBackwards);
    if (cfrange.location == kCFNotFound)
    {
        assert(0);  /* shouldn't ever hit this... */
        CFRelease(cfmutstr);
        return(NULL);
    } /* if */

    /* chop the "/exename" from the end of the path string... */
    cfrange.length = CFStringGetLength(cfmutstr) - cfrange.location;
    CFStringDelete(cfmutstr, cfrange);

    /* If we're an Application Bundle, chop everything but the base. */
    cfrange = CFStringFind(cfmutstr, CFSTR("/Contents/MacOS"),
                           kCFCompareCaseInsensitive |
                           kCFCompareBackwards |
                           kCFCompareAnchored);

    if (cfrange.location != kCFNotFound)
        CFStringDelete(cfmutstr, cfrange);  /* chop that, too. */

    retval = convertCFString(cfmutstr);
    CFRelease(cfmutstr);

    return(retval);  /* whew. */
} /* __PHYSFS_platformCalcBaseDir */


/* !!! FIXME */
#define osxerr(x) x

char *__PHYSFS_platformRealPath(const char *path)
{
    /* The symlink and relative path resolving happens in FSPathMakeRef() */
    FSRef fsref;
    CFURLRef cfurl = NULL;
    CFStringRef cfstr = NULL;
    char *retval = NULL;
    OSStatus rc = osxerr(FSPathMakeRef((UInt8 *) path, &fsref, NULL));
    BAIL_IF_MACRO(rc != noErr, NULL, NULL);

    /* Now get it to spit out a full path. */
    cfurl = CFURLCreateFromFSRef(cfallocator, &fsref);
    BAIL_IF_MACRO(cfurl == NULL, ERR_OUT_OF_MEMORY, NULL);
    cfstr = CFURLCopyFileSystemPath(cfurl, kCFURLPOSIXPathStyle);
    CFRelease(cfurl);
    BAIL_IF_MACRO(cfstr == NULL, ERR_OUT_OF_MEMORY, NULL);
    retval = convertCFString(cfstr);
    CFRelease(cfstr);

    return(retval);
} /* __PHYSFS_platformRealPath */


char *__PHYSFS_platformCurrentDir(void)
{
    return(__PHYSFS_platformRealPath("."));  /* let CFURL sort it out. */
} /* __PHYSFS_platformCurrentDir */


/* Platform allocator uses default CFAllocator at PHYSFS_init() time. */

static CFAllocatorRef cfallocdef = NULL;

static int macosxAllocatorInit(void)
{
    int retval = 0;
    cfallocdef = CFAllocatorGetDefault();
    retval = (cfallocdef != NULL);
    if (retval)
        CFRetain(cfallocdef);
    return(retval);
} /* macosxAllocatorInit */


static void macosxAllocatorDeinit(void)
{
    if (cfallocdef != NULL)
    {
        CFRelease(cfallocdef);
        cfallocdef = NULL;
    } /* if */
} /* macosxAllocatorDeinit */


static void *macosxAllocatorMalloc(PHYSFS_uint64 s)
{
    BAIL_IF_MACRO(__PHYSFS_ui64FitsAddressSpace(s), ERR_OUT_OF_MEMORY, NULL);
    return(CFAllocatorAllocate(cfallocdef, (CFIndex) s, 0));
} /* macosxAllocatorMalloc */


static void *macosxAllocatorRealloc(void *ptr, PHYSFS_uint64 s)
{
    BAIL_IF_MACRO(__PHYSFS_ui64FitsAddressSpace(s), ERR_OUT_OF_MEMORY, NULL);
    return(CFAllocatorReallocate(cfallocdef, ptr, (CFIndex) s, 0));
} /* macosxAllocatorRealloc */


static void macosxAllocatorFree(void *ptr)
{
    CFAllocatorDeallocate(cfallocdef, ptr);
} /* macosxAllocatorFree */


int __PHYSFS_platformSetDefaultAllocator(PHYSFS_Allocator *a)
{
    allocator.Init = macosxAllocatorInit;
    allocator.Deinit = macosxAllocatorDeinit;
    allocator.Malloc = macosxAllocatorMalloc;
    allocator.Realloc = macosxAllocatorRealloc;
    allocator.Free = macosxAllocatorFree;
    return(1);  /* return non-zero: we're supplying custom allocator. */
} /* __PHYSFS_platformSetDefaultAllocator */


PHYSFS_uint64 __PHYSFS_platformGetThreadID(void)
{
    return( (PHYSFS_uint64) ((size_t) MPCurrentTaskID()) );
} /* __PHYSFS_platformGetThreadID */


void *__PHYSFS_platformCreateMutex(void)
{
    MPCriticalRegionID m = NULL;
    if (osxerr(MPCreateCriticalRegion(&m)) != noErr)
        return NULL;
    return m;
} /* __PHYSFS_platformCreateMutex */


void __PHYSFS_platformDestroyMutex(void *mutex)
{
    MPCriticalRegionID m = (MPCriticalRegionID) mutex;
    MPDeleteCriticalRegion(m);
} /* __PHYSFS_platformDestroyMutex */


int __PHYSFS_platformGrabMutex(void *mutex)
{
    MPCriticalRegionID m = (MPCriticalRegionID) mutex;
    if (MPEnterCriticalRegion(m, kDurationForever) != noErr)
        return(0);
    return(1);
} /* __PHYSFS_platformGrabMutex */


void __PHYSFS_platformReleaseMutex(void *mutex)
{
    MPCriticalRegionID m = (MPCriticalRegionID) mutex;
    MPExitCriticalRegion(m);
} /* __PHYSFS_platformReleaseMutex */

#endif /* PHYSFS_PLATFORM_MACOSX */

/* end of macosx.c ... */

