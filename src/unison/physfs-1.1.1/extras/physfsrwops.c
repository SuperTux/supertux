/*
 * This code provides a glue layer between PhysicsFS and Simple Directmedia
 *  Layer's (SDL) RWops i/o abstraction.
 *
 * License: this code is public domain. I make no warranty that it is useful,
 *  correct, harmless, or environmentally safe.
 *
 * This particular file may be used however you like, including copying it
 *  verbatim into a closed-source project, exploiting it commercially, and
 *  removing any trace of my name from the source (although I hope you won't
 *  do that). I welcome enhancements and corrections to this file, but I do
 *  not require you to send me patches if you make changes. This code has
 *  NO WARRANTY.
 *
 * Unless otherwise stated, the rest of PhysicsFS falls under the zlib license.
 *  Please see LICENSE.txt in the root of the source tree.
 *
 * SDL falls under the LGPL license. You can get SDL at http://www.libsdl.org/
 *
 *  This file was written by Ryan C. Gordon. (icculus@icculus.org).
 */

#include <stdio.h>  /* used for SEEK_SET, SEEK_CUR, SEEK_END ... */
#include "physfsrwops.h"

static int physfsrwops_seek(SDL_RWops *rw, int offset, int whence)
{
    PHYSFS_File *handle = (PHYSFS_File *) rw->hidden.unknown.data1;
    int pos = 0;

    if (whence == SEEK_SET)
    {
        pos = offset;
    } /* if */

    else if (whence == SEEK_CUR)
    {
        PHYSFS_sint64 current = PHYSFS_tell(handle);
        if (current == -1)
        {
            SDL_SetError("Can't find position in file: %s",
                          PHYSFS_getLastError());
            return(-1);
        } /* if */

        pos = (int) current;
        if ( ((PHYSFS_sint64) pos) != current )
        {
            SDL_SetError("Can't fit current file position in an int!");
            return(-1);
        } /* if */

        if (offset == 0)  /* this is a "tell" call. We're done. */
            return(pos);

        pos += offset;
    } /* else if */

    else if (whence == SEEK_END)
    {
        PHYSFS_sint64 len = PHYSFS_fileLength(handle);
        if (len == -1)
        {
            SDL_SetError("Can't find end of file: %s", PHYSFS_getLastError());
            return(-1);
        } /* if */

        pos = (int) len;
        if ( ((PHYSFS_sint64) pos) != len )
        {
            SDL_SetError("Can't fit end-of-file position in an int!");
            return(-1);
        } /* if */

        pos += offset;
    } /* else if */

    else
    {
        SDL_SetError("Invalid 'whence' parameter.");
        return(-1);
    } /* else */

    if ( pos < 0 )
    {
        SDL_SetError("Attempt to seek past start of file.");
        return(-1);
    } /* if */
    
    if (!PHYSFS_seek(handle, (PHYSFS_uint64) pos))
    {
        SDL_SetError("PhysicsFS error: %s", PHYSFS_getLastError());
        return(-1);
    } /* if */

    return(pos);
} /* physfsrwops_seek */


static int physfsrwops_read(SDL_RWops *rw, void *ptr, int size, int maxnum)
{
    PHYSFS_File *handle = (PHYSFS_File *) rw->hidden.unknown.data1;
    PHYSFS_sint64 rc = PHYSFS_read(handle, ptr, size, maxnum);
    if (rc != maxnum)
    {
        if (!PHYSFS_eof(handle)) /* not EOF? Must be an error. */
            SDL_SetError("PhysicsFS error: %s", PHYSFS_getLastError());
    } /* if */

    return((int) rc);
} /* physfsrwops_read */


static int physfsrwops_write(SDL_RWops *rw, const void *ptr, int size, int num)
{
    PHYSFS_File *handle = (PHYSFS_File *) rw->hidden.unknown.data1;
    PHYSFS_sint64 rc = PHYSFS_write(handle, ptr, size, num);
    if (rc != num)
        SDL_SetError("PhysicsFS error: %s", PHYSFS_getLastError());

    return((int) rc);
} /* physfsrwops_write */


static int physfsrwops_close(SDL_RWops *rw)
{
    PHYSFS_File *handle = (PHYSFS_File *) rw->hidden.unknown.data1;
    if (!PHYSFS_close(handle))
    {
        SDL_SetError("PhysicsFS error: %s", PHYSFS_getLastError());
        return(-1);
    } /* if */

    SDL_FreeRW(rw);
    return(0);
} /* physfsrwops_close */


static SDL_RWops *create_rwops(PHYSFS_File *handle)
{
    SDL_RWops *retval = NULL;

    if (handle == NULL)
        SDL_SetError("PhysicsFS error: %s", PHYSFS_getLastError());
    else
    {
        retval = SDL_AllocRW();
        if (retval != NULL)
        {
            retval->seek  = physfsrwops_seek;
            retval->read  = physfsrwops_read;
            retval->write = physfsrwops_write;
            retval->close = physfsrwops_close;
            retval->hidden.unknown.data1 = handle;
        } /* if */
    } /* else */

    return(retval);
} /* create_rwops */


SDL_RWops *PHYSFSRWOPS_makeRWops(PHYSFS_File *handle)
{
    SDL_RWops *retval = NULL;
    if (handle == NULL)
        SDL_SetError("NULL pointer passed to PHYSFSRWOPS_makeRWops().");
    else
        retval = create_rwops(handle);

    return(retval);
} /* PHYSFSRWOPS_makeRWops */


SDL_RWops *PHYSFSRWOPS_openRead(const char *fname)
{
    return(create_rwops(PHYSFS_openRead(fname)));
} /* PHYSFSRWOPS_openRead */


SDL_RWops *PHYSFSRWOPS_openWrite(const char *fname)
{
    return(create_rwops(PHYSFS_openWrite(fname)));
} /* PHYSFSRWOPS_openWrite */


SDL_RWops *PHYSFSRWOPS_openAppend(const char *fname)
{
    return(create_rwops(PHYSFS_openAppend(fname)));
} /* PHYSFSRWOPS_openAppend */


/* end of physfsrwops.c ... */

