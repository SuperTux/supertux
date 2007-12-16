/*
 * This code shows how to read a zipfile included in an app's binary.
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
 *  This file was written by Ryan C. Gordon. (icculus@icculus.org).
 */

/*
 * Compile this program and then attach a .zip file to the end of the
 *  compiled binary.
 *
 * On Linux, something like this will build the final binary:
 *   gcc -o selfextract.tmp selfextract.c -lphysfs && \
 *   cat selfextract.tmp myzipfile.zip >> selfextract && \
 *   chmod a+x selfextract && \
 *   rm -f selfextract.tmp
 *
 * This may not work on all platforms, and it probably only works with
 *  .zip files, since they are designed to be appended to another file.
 */

#include <stdio.h>
#include "physfs.h"

int main(int argc, char **argv)
{
    int rc = 0;

    if (!PHYSFS_init(argv[0]))
    {
        printf("PHYSFS_init() failed: %s\n", PHYSFS_getLastError());
        return(42);
    } /* if */

    rc = PHYSFS_addToSearchPath(argv[0], 0);
    if (!rc)
    {
        printf("Couldn't find self-extract data: %s\n", PHYSFS_getLastError());
        printf("This might mean you didn't append a zipfile to the binary.\n");
        return(42);
    } /* if */

    char **files = PHYSFS_enumerateFiles("/");
    char **i;
    for (i = files; *i != NULL; i++)
    {
        const char *dirorfile = PHYSFS_isDirectory(*i) ? "Directory" : "File";
        printf(" * %s '%s' is in root of attached data.\n", dirorfile, *i);
    } /* for */
    PHYSFS_freeList(files);

    return(0);
} /* main */

