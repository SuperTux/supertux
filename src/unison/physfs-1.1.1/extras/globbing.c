/** \file globbing.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "physfs.h"
#include "globbing.h"

/**
 * Please see globbing.h for details.
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
 *  \author Ryan C. Gordon.
 */


static int matchesPattern(const char *fname, const char *wildcard,
                          int caseSensitive)
{
    char x, y;
    const char *fnameptr = fname;
    const char *wildptr = wildcard;

    while ((*wildptr) && (*fnameptr))
    {
        y = *wildptr;
        if (y == '*')
        {
            do
            {
                wildptr++;  /* skip multiple '*' in a row... */
            } while (*wildptr == '*');

            y = (caseSensitive) ? *wildptr : (char) tolower(*wildptr);

            while (1)
            {
                x = (caseSensitive) ? *fnameptr : (char) tolower(*fnameptr);
                if ((!x) || (x == y))
                    break;
                else
                    fnameptr++;
            } /* while */
        } /* if */

        else if (y == '?')
        {
            wildptr++;
            fnameptr++;
        } /* else if */

        else
        {
            if (caseSensitive)
                x = *fnameptr;
            else
            {
                x = tolower(*fnameptr);
                y = tolower(y);
            } /* if */

            wildptr++;
            fnameptr++;

            if (x != y)
                return(0);
        } /* else */
    } /* while */

    while (*wildptr == '*')
        wildptr++;

    return(*fnameptr == *wildptr);
} /* matchesPattern */


char **PHYSFSEXT_enumerateFilesWildcard(const char *dir, const char *wildcard,
                                        int caseSensitive)
{
    char **rc = PHYSFS_enumerateFiles(dir);
    char **i = rc;
    char **j;

    while (*i != NULL)
    {
        if (matchesPattern(*i, wildcard, caseSensitive))
            i++;
        else
        {
            /* FIXME: This counts on physfs's allocation method not changing! */
            free(*i);
            for (j = i; *j != NULL; j++)
                j[0] = j[1];
        } /* else */
    } /* for */

    return(rc);
} /* PHYSFSEXT_enumerateFilesWildcard */


#ifdef TEST_PHYSFSEXT_ENUMERATEFILESWILDCARD
int main(int argc, char **argv)
{
    int rc;
    char **flist;
    char **i;

    if (argc != 3)
    {
        printf("USAGE: %s <pattern> <caseSen>\n"
               "   where <caseSen> is 1 or 0.\n", argv[0]);
        return(1);
    } /* if */

    if (!PHYSFS_init(argv[0]))
    {
        fprintf(stderr, "PHYSFS_init(): %s\n", PHYSFS_getLastError());
        return(1);
    } /* if */

    if (!PHYSFS_addToSearchPath(".", 1))
    {
        fprintf(stderr, "PHYSFS_addToSearchPath(): %s\n", PHYSFS_getLastError());
        PHYSFS_deinit();
        return(1);
    } /* if */

    flist = PHYSFSEXT_enumerateFilesWildcard("/", argv[1], atoi(argv[2]));
    rc = 0;
    for (i = flist; *i; i++)
    {
        printf("%s\n", *i);
        rc++;
    } /* for */
    printf("\n  total %d files.\n\n", rc);

    PHYSFS_freeList(flist);
    PHYSFS_deinit();

    return(0);
} /* main */
#endif

/* end of globbing.c ... */

