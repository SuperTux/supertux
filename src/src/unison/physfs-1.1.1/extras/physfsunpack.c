#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "physfs.h"


static int failure = 0;

static void modTimeToStr(PHYSFS_sint64 modtime, char *modstr, size_t strsize)
{
    const char *str = "unknown modtime";
    if (modtime != -1)
    {
        time_t t = (time_t) modtime;
        str = ctime(&t);
    } /* if */

    strncpy(modstr, str, strsize);
    modstr[strsize-1] = '\0';
    strsize = strlen(modstr);
    while ((modstr[strsize-1] == '\n') || (modstr[strsize-1] == '\r'))
        modstr[--strsize] = '\0';
} /* modTimeToStr */


static void fail(const char *what, const char *why)
{
    if (why == NULL)
        why = PHYSFS_getLastError();
    fprintf(stderr, "%s failed: %s\n", what, why);
    failure = 1;
} /* fail */


static void dumpFile(const char *fname)
{
    const int origfailure = failure;
    PHYSFS_File *out = NULL;
    PHYSFS_File *in = NULL;

    failure = 0;

    if ((in = PHYSFS_openRead(fname)) == NULL)
        fail("\nPHYSFS_openRead", NULL);
    else if ((out = PHYSFS_openWrite(fname)) == NULL)
        fail("\nPHYSFS_openWrite", NULL);
    else
    {
        char modstr[64];
        PHYSFS_sint64 size = PHYSFS_fileLength(in);

        printf("(");
        if (size == -1)
            printf("?");
        else
            printf("%lld", (long long) size);
        printf(" bytes");

        modTimeToStr(PHYSFS_getLastModTime(fname), modstr, sizeof (modstr));
        printf(", %s)\n", modstr);

        while ( (!failure) && (!PHYSFS_eof(in)) )
        {
            static char buf[64 * 1024];
            PHYSFS_sint64 br = PHYSFS_read(in, buf, 1, sizeof (buf));
            if (br == -1)
                fail("PHYSFS_read", NULL);
            else
            {
                PHYSFS_sint64 bw = PHYSFS_write(out, buf, 1, br);
                if (bw != br)
                    fail("PHYSFS_write", NULL);
                else
                    size -= bw;
            } /* else */
        } /* while */

        if ((!failure) && (size != 0))
            fail("PHYSFS_eof", "BUG! eof != PHYSFS_fileLength bytes!");
    } /* else */

    if (in != NULL)
        PHYSFS_close(in);

    if (out != NULL)
    {
        if (!PHYSFS_close(out))
            fail("PHYSFS_close", NULL);
    } /* if */

    if (failure)
        PHYSFS_delete(fname);
    else
        failure = origfailure;
} /* dumpFile */


static void unpackCallback(void *_depth, const char *origdir, const char *str)
{
    int depth = *((int *) _depth);
    const int len = strlen(origdir) + strlen(str) + 2;
    char *fname = (char *) malloc(len);
    if (fname == NULL)
        fail("malloc", "Out of memory!");
    else
    {
        if (strcmp(origdir, "/") == 0)
            origdir = "";

        snprintf(fname, len, "%s/%s", origdir, str);

        printf("%s ", fname);
        if (PHYSFS_isDirectory(fname))
        {
            depth++;
            printf("(directory)\n");
            if (!PHYSFS_mkdir(fname))
                fail("PHYSFS_mkdir", NULL);
            else
                PHYSFS_enumerateFilesCallback(fname, unpackCallback, &depth);
        } /* if */

        else if (PHYSFS_isSymbolicLink(fname))
        {
            printf("(symlink)\n");
            /* !!! FIXME: ?  if (!symlink(fname, */
        } /* else if */

        else  /* ...file. */
        {
            dumpFile(fname);
        } /* else */

        free(fname);
    } /* else */
} /* unpackCallback */


int main(int argc, char **argv)
{
    int zero = 0;

    if (argc != 3)
    {
        fprintf(stderr, "USAGE: %s <archive> <unpackDirectory>\n", argv[0]);
        return 1;
    } /* if */

    if (!PHYSFS_init(argv[0]))
    {
        fprintf(stderr, "PHYSFS_init() failed: %s\n", PHYSFS_getLastError());
        return 2;
    } /* if */

    if (!PHYSFS_setWriteDir(argv[2]))
    {
        fprintf(stderr, "PHYSFS_setWriteDir('%s') failed: %s\n",
                argv[2], PHYSFS_getLastError());
        return 3;
    } /* if */

    if (!PHYSFS_mount(argv[1], NULL, 1))
    {
        fprintf(stderr, "PHYSFS_mount('%s') failed: %s\n",
                argv[1], PHYSFS_getLastError());
        return 4;
    } /* if */

    PHYSFS_permitSymbolicLinks(1);
    PHYSFS_enumerateFilesCallback("/", unpackCallback, &zero);
    PHYSFS_deinit();
    if (failure)
        return 5;

    return 0;
} /* main */

/* end of physfsunpack.c ... */

