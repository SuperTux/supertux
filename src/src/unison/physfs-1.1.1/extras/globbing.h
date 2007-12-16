/** \file globbing.h */

/**
 * \mainpage PhysicsFS globbing
 *
 * This is an extension to PhysicsFS to let you search for files with basic
 *  wildcard matching, regardless of what sort of filesystem or archive they
 *  reside in. It does this by enumerating directories as needed and manually
 *  locating matching entries.
 *
 * Usage: Set up PhysicsFS as you normally would, then use
 *  PHYSFSEXT_enumerateFilesPattern() when enumerating files. This is just
 *  like PHYSFS_enumerateFiles(), but it returns a subset that matches your
 *  wildcard pattern. You must call PHYSFS_freeList() on the results, just
 *  like you would with PHYSFS_enumerateFiles().
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


/**
 * \fn char **PHYSFS_enumerateFilesWildcard(const char *dir, const char *wildcard, int caseSensitive)
 * \brief Get a file listing of a search path's directory.
 *
 * Matching directories are interpolated. That is, if "C:\mydir" is in the
 *  search path and contains a directory "savegames" that contains "x.sav",
 *  "y.Sav", and "z.txt", and there is also a "C:\userdir" in the search path
 *  that has a "savegames" subdirectory with "w.sav", then the following code:
 *
 * \code
 * char **rc = PHYSFS_enumerateFilesWildcard("savegames", "*.sav", 0);
 * char **i;
 *
 * for (i = rc; *i != NULL; i++)
 *     printf(" * We've got [%s].\n", *i);
 *
 * PHYSFS_freeList(rc);
 * \endcode
 *
 *  ...will print:
 *
 * \verbatim
 * We've got [x.sav].
 * We've got [y.Sav].
 * We've got [w.sav].\endverbatim
 *
 * Feel free to sort the list however you like. We only promise there will
 *  be no duplicates, but not what order the final list will come back in.
 *
 * Wildcard strings can use the '*' and '?' characters, currently.
 * Matches can be case-insensitive if you pass a zero for argument 3.
 *
 * Don't forget to call PHYSFS_freeList() with the return value from this
 *  function when you are done with it.
 *
 *    \param dir directory in platform-independent notation to enumerate.
 *   \return Null-terminated array of null-terminated strings.
 */
__EXPORT__ char **PHYSFSEXT_enumerateFilesWildcard(const char *dir,
                                                   const char *wildcard,
                                                   int caseSensitive);

/* end of globbing.h ... */

