/*
 * This is a quick and dirty HTTP server that uses PhysicsFS to retrieve
 *  files. It is not robust at all, probably buggy, and definitely poorly
 *  designed. It's just meant to show that it can be done.
 *
 * Basically, you compile this code, and run it:
 *   ./physfshttpd archive1.zip archive2.zip /path/to/a/real/dir etc...
 *
 * The files are appended in order to the PhysicsFS search path, and when
 *  a client request comes it, it looks for the file in said search path.
 *
 * My goal was to make this work in less than 300 lines of C, so again, it's
 *  not to be used for any serious purpose. Patches to make this application
 *  suck less will be readily and gratefully accepted.
 *
 * Command line I used to build this on Linux:
 *  gcc -Wall -Werror -g -o bin/physfshttpd extras/physfshttpd.c -lphysfs
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef LACKING_SIGNALS
#include <signal.h>
#endif

#ifndef LACKING_PROTOENT
#include <netdb.h>
#endif

#include "physfs.h"


#define DEFAULT_PORTNUM  6667

typedef struct
{
    int sock;
    struct sockaddr *addr;
    socklen_t addrlen;
} http_args;


static char *txt404 =
"HTTP/1.0 404 Not Found\n"
"Connection: close\n"
"Content-type: text/html\n"
"\n"
"<html><head><title>404 Not Found</title></head>\n"
"<body>Can't find that.</body></html>\n\n";


static void feed_file_http(const char *ipstr, int sock, const char *fname)
{
    PHYSFS_File *in = PHYSFS_openRead(fname);
    char buffer[1024];
    printf("%s: requested [%s].\n", ipstr, fname);
    if (in == NULL)
    {
        printf("%s: Can't open [%s]: %s.\n",
               ipstr, fname, PHYSFS_getLastError());
        write(sock, txt404, strlen(txt404));  /* !!! FIXME: Check retval */
    } /* if */
    else
    {
        do
        {
            PHYSFS_sint64 br = PHYSFS_read(in, buffer, 1, sizeof (buffer));
            if (br == -1)
            {
                printf("%s: Read error: %s.\n", ipstr, PHYSFS_getLastError());
                break;
            } /* if */

            write(sock, buffer, (int) br);   /* !!! FIXME: CHECK THIS RETVAL! */
        } while (!PHYSFS_eof(in));

        PHYSFS_close(in);
    } /* else */
} /* feed_file_http */


static void *do_http(void *_args)
{
    http_args *args = (http_args *) _args;
    char ipstr[128];
    char buffer[512];
    char *ptr;
    strncpy(ipstr, inet_ntoa(((struct sockaddr_in *) args->addr)->sin_addr),
            sizeof (ipstr));
    ipstr[sizeof (ipstr) - 1] = '\0';

    printf("%s: connected.\n", ipstr);
    read(args->sock, buffer, sizeof (buffer));
    buffer[sizeof (buffer) - 1] = '\0';
    ptr = strchr(buffer, '\n');
    if (!ptr)
        printf("%s: potentially bogus request.\n", ipstr);
    else
    {
        *ptr = '\0';
        ptr = strchr(buffer, '\r');
        if (ptr != NULL)
            *ptr = '\0';

        if ((toupper(buffer[0]) == 'G') &&
            (toupper(buffer[1]) == 'E') &&
            (toupper(buffer[2]) == 'T') &&
            (toupper(buffer[3]) == ' ') &&
            (toupper(buffer[4]) == '/'))
        {
            ptr = strchr(buffer + 5, ' ');
            if (ptr != NULL)
                *ptr = '\0';
            feed_file_http(ipstr, args->sock, buffer + 4);
        } /* if */
    } /* else */

    /* !!! FIXME: Time the transfer. */
    printf("%s: closing connection.\n", ipstr);
    close(args->sock);
    free(args->addr);
    free(args);
    return(NULL);
} /* do_http */


static void serve_http_request(int sock, struct sockaddr *addr,
                               socklen_t addrlen)
{
    http_args *args = (http_args *) malloc(sizeof (http_args));
    if (args == NULL)
    {
        printf("out of memory.\n");
        return;
    } // if
    args->addr = (struct sockaddr *) malloc(addrlen);
    if (args->addr == NULL)
    {
        free(args);
        printf("out of memory.\n");
        return;
    } // if

    args->sock = sock;
    args->addrlen = addrlen;
    memcpy(args->addr, addr, addrlen);

    /* !!! FIXME: optionally spin a thread... */
    do_http((void *) args);
} /* server_http_request */


static int create_listen_socket(short portnum)
{
    int retval = -1;
    int protocol = 0;  /* pray this is right. */

#ifndef LACKING_PROTOENT
    struct protoent *prot;
    setprotoent(0);
    prot = getprotobyname("tcp");
    if (prot != NULL)
        protocol = prot->p_proto;
#endif

    retval = socket(PF_INET, SOCK_STREAM, protocol);
    if (retval >= 0)
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(portnum);
        addr.sin_addr.s_addr = INADDR_ANY;
        if ((bind(retval, &addr, (socklen_t) sizeof (addr)) == -1) ||
            (listen(retval, 5) == -1))
        {
            close(retval);
            retval = -1;
        } /* if */
    } /* if */

    return(retval);
} /* create_listen_socket */


static int listensocket = -1;

void at_exit_cleanup(void)
{
    /*
     * !!! FIXME: If thread support, signal threads to terminate and
     * !!! FIXME:  wait for them to clean up.
     */

    if (listensocket >= 0)
        close(listensocket);

    if (!PHYSFS_deinit())
        printf("PHYSFS_deinit() failed: %s\n", PHYSFS_getLastError());
} /* at_exit_cleanup */


int main(int argc, char **argv)
{
    int i;
    int portnum = DEFAULT_PORTNUM;

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

#ifndef LACKING_SIGNALS
    /* I'm not sure if this qualifies as a cheap trick... */
    signal(SIGTERM, exit);
    signal(SIGINT, exit);
    signal(SIGFPE, exit);
    signal(SIGSEGV, exit);
    signal(SIGPIPE, exit);
    signal(SIGILL, exit);
#endif

    if (argc == 1)
    {
        printf("USAGE: %s <archive1> [archive2 [... archiveN]]\n", argv[0]);
        return(42);
    } /* if */

    if (!PHYSFS_init(argv[0]))
    {
        printf("PHYSFS_init() failed: %s\n", PHYSFS_getLastError());
        return(42);
    } /* if */

    /* normally, this is bad practice, but oh well. */
    atexit(at_exit_cleanup);

    for (i = 1; i < argc; i++)
    {
        if (!PHYSFS_addToSearchPath(argv[i], 1))
            printf(" WARNING: failed to add [%s] to search path.\n", argv[i]);
    } /* else */

    listensocket = create_listen_socket(portnum);
    if (listensocket < 0)
    {
        printf("listen socket failed to create.\n");
        return(42);
    } /* if */

    while (1)  /* infinite loop for now. */
    {
        struct sockaddr addr;
        socklen_t len;
        int s = accept(listensocket, &addr, &len);
        if (s < 0)
        {
            printf("accept() failed: %s\n", strerror(errno));
            close(listensocket);
            return(42);
        } /* if */

        serve_http_request(s, &addr, len);
    } /* while */

    return(0);
} /* main */

/* end of physfshttpd.c ... */

