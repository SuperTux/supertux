/**
 * PhysicsFS; a portable, flexible file i/o abstraction.
 *
 * Documentation is in physfs.h. It's verbose, honest.  :)
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#include <stdio.h>
#include <stdlib.h>

#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

/* The macros used to swap values */
/* Try to use superfast macros on systems that support them */
#ifdef linux
#include <asm/byteorder.h>
#ifdef __arch__swab16
#define PHYSFS_Swap16  __arch__swab16
#endif
#ifdef __arch__swab32
#define PHYSFS_Swap32  __arch__swab32
#endif
#endif /* linux */

#if (defined macintosh) && !(defined __MWERKS__)
#define __inline__
#endif

#if (defined _MSC_VER)
#define __inline__ __inline
#endif

#ifndef PHYSFS_Swap16
static __inline__ PHYSFS_uint16 PHYSFS_Swap16(PHYSFS_uint16 D)
{
    return((D<<8)|(D>>8));
}
#endif
#ifndef PHYSFS_Swap32
static __inline__ PHYSFS_uint32 PHYSFS_Swap32(PHYSFS_uint32 D)
{
    return((D<<24)|((D<<8)&0x00FF0000)|((D>>8)&0x0000FF00)|(D>>24));
}
#endif
#ifndef PHYSFS_NO_64BIT_SUPPORT
#ifndef PHYSFS_Swap64
static __inline__ PHYSFS_uint64 PHYSFS_Swap64(PHYSFS_uint64 val) {
    PHYSFS_uint32 hi, lo;

    /* Separate into high and low 32-bit values and swap them */
    lo = (PHYSFS_uint32)(val&0xFFFFFFFF);
    val >>= 32;
    hi = (PHYSFS_uint32)(val&0xFFFFFFFF);
    val = PHYSFS_Swap32(lo);
    val <<= 32;
    val |= PHYSFS_Swap32(hi);
    return(val);
}
#endif
#else
#ifndef PHYSFS_Swap64
/* This is mainly to keep compilers from complaining in PHYSFS code.
   If there is no real 64-bit datatype, then compilers will complain about
   the fake 64-bit datatype that PHYSFS provides when it compiles user code.
*/
#define PHYSFS_Swap64(X)    (X)
#endif
#endif /* PHYSFS_NO_64BIT_SUPPORT */


/* Byteswap item from the specified endianness to the native endianness */
#if PHYSFS_BYTEORDER == PHYSFS_LIL_ENDIAN
PHYSFS_uint16 PHYSFS_swapULE16(PHYSFS_uint16 x) { return(x); }
PHYSFS_sint16 PHYSFS_swapSLE16(PHYSFS_sint16 x) { return(x); }
PHYSFS_uint32 PHYSFS_swapULE32(PHYSFS_uint32 x) { return(x); }
PHYSFS_sint32 PHYSFS_swapSLE32(PHYSFS_sint32 x) { return(x); }
PHYSFS_uint64 PHYSFS_swapULE64(PHYSFS_uint64 x) { return(x); }
PHYSFS_sint64 PHYSFS_swapSLE64(PHYSFS_sint64 x) { return(x); }

PHYSFS_uint16 PHYSFS_swapUBE16(PHYSFS_uint16 x) { return(PHYSFS_Swap16(x)); }
PHYSFS_sint16 PHYSFS_swapSBE16(PHYSFS_sint16 x) { return(PHYSFS_Swap16(x)); }
PHYSFS_uint32 PHYSFS_swapUBE32(PHYSFS_uint32 x) { return(PHYSFS_Swap32(x)); }
PHYSFS_sint32 PHYSFS_swapSBE32(PHYSFS_sint32 x) { return(PHYSFS_Swap32(x)); }
PHYSFS_uint64 PHYSFS_swapUBE64(PHYSFS_uint64 x) { return(PHYSFS_Swap64(x)); }
PHYSFS_sint64 PHYSFS_swapSBE64(PHYSFS_sint64 x) { return(PHYSFS_Swap64(x)); }
#else
PHYSFS_uint16 PHYSFS_swapULE16(PHYSFS_uint16 x) { return(PHYSFS_Swap16(x)); }
PHYSFS_sint16 PHYSFS_swapSLE16(PHYSFS_sint16 x) { return(PHYSFS_Swap16(x)); }
PHYSFS_uint32 PHYSFS_swapULE32(PHYSFS_uint32 x) { return(PHYSFS_Swap32(x)); }
PHYSFS_sint32 PHYSFS_swapSLE32(PHYSFS_sint32 x) { return(PHYSFS_Swap32(x)); }
PHYSFS_uint64 PHYSFS_swapULE64(PHYSFS_uint64 x) { return(PHYSFS_Swap64(x)); }
PHYSFS_sint64 PHYSFS_swapSLE64(PHYSFS_sint64 x) { return(PHYSFS_Swap64(x)); }

PHYSFS_uint16 PHYSFS_swapUBE16(PHYSFS_uint16 x) { return(x); }
PHYSFS_sint16 PHYSFS_swapSBE16(PHYSFS_sint16 x) { return(x); }
PHYSFS_uint32 PHYSFS_swapUBE32(PHYSFS_uint32 x) { return(x); }
PHYSFS_sint32 PHYSFS_swapSBE32(PHYSFS_sint32 x) { return(x); }
PHYSFS_uint64 PHYSFS_swapUBE64(PHYSFS_uint64 x) { return(x); }
PHYSFS_sint64 PHYSFS_swapSBE64(PHYSFS_sint64 x) { return(x); }
#endif


int PHYSFS_readSLE16(PHYSFS_File *file, PHYSFS_sint16 *val)
{
    PHYSFS_sint16 in;
    BAIL_IF_MACRO(val == NULL, ERR_INVALID_ARGUMENT, 0);
    BAIL_IF_MACRO(PHYSFS_read(file, &in, sizeof (in), 1) != 1, NULL, 0);
    *val = PHYSFS_swapSLE16(in);
    return(1);
} /* PHYSFS_readSLE16 */


int PHYSFS_readULE16(PHYSFS_File *file, PHYSFS_uint16 *val)
{
    PHYSFS_uint16 in;
    BAIL_IF_MACRO(val == NULL, ERR_INVALID_ARGUMENT, 0);
    BAIL_IF_MACRO(PHYSFS_read(file, &in, sizeof (in), 1) != 1, NULL, 0);
    *val = PHYSFS_swapULE16(in);
    return(1);
} /* PHYSFS_readULE16 */


int PHYSFS_readSBE16(PHYSFS_File *file, PHYSFS_sint16 *val)
{
    PHYSFS_sint16 in;
    BAIL_IF_MACRO(val == NULL, ERR_INVALID_ARGUMENT, 0);
    BAIL_IF_MACRO(PHYSFS_read(file, &in, sizeof (in), 1) != 1, NULL, 0);
    *val = PHYSFS_swapSBE16(in);
    return(1);
} /* PHYSFS_readSBE16 */


int PHYSFS_readUBE16(PHYSFS_File *file, PHYSFS_uint16 *val)
{
    PHYSFS_uint16 in;
    BAIL_IF_MACRO(val == NULL, ERR_INVALID_ARGUMENT, 0);
    BAIL_IF_MACRO(PHYSFS_read(file, &in, sizeof (in), 1) != 1, NULL, 0);
    *val = PHYSFS_swapUBE16(in);
    return(1);
} /* PHYSFS_readUBE16 */


int PHYSFS_readSLE32(PHYSFS_File *file, PHYSFS_sint32 *val)
{
    PHYSFS_sint32 in;
    BAIL_IF_MACRO(val == NULL, ERR_INVALID_ARGUMENT, 0);
    BAIL_IF_MACRO(PHYSFS_read(file, &in, sizeof (in), 1) != 1, NULL, 0);
    *val = PHYSFS_swapSLE32(in);
    return(1);
} /* PHYSFS_readSLE32 */


int PHYSFS_readULE32(PHYSFS_File *file, PHYSFS_uint32 *val)
{
    PHYSFS_uint32 in;
    BAIL_IF_MACRO(val == NULL, ERR_INVALID_ARGUMENT, 0);
    BAIL_IF_MACRO(PHYSFS_read(file, &in, sizeof (in), 1) != 1, NULL, 0);
    *val = PHYSFS_swapULE32(in);
    return(1);
} /* PHYSFS_readULE32 */


int PHYSFS_readSBE32(PHYSFS_File *file, PHYSFS_sint32 *val)
{
    PHYSFS_sint32 in;
    BAIL_IF_MACRO(val == NULL, ERR_INVALID_ARGUMENT, 0);
    BAIL_IF_MACRO(PHYSFS_read(file, &in, sizeof (in), 1) != 1, NULL, 0);
    *val = PHYSFS_swapSBE32(in);
    return(1);
} /* PHYSFS_readSBE32 */


int PHYSFS_readUBE32(PHYSFS_File *file, PHYSFS_uint32 *val)
{
    PHYSFS_uint32 in;
    BAIL_IF_MACRO(val == NULL, ERR_INVALID_ARGUMENT, 0);
    BAIL_IF_MACRO(PHYSFS_read(file, &in, sizeof (in), 1) != 1, NULL, 0);
    *val = PHYSFS_swapUBE32(in);
    return(1);
} /* PHYSFS_readUBE32 */


int PHYSFS_readSLE64(PHYSFS_File *file, PHYSFS_sint64 *val)
{
    PHYSFS_sint64 in;
    BAIL_IF_MACRO(val == NULL, ERR_INVALID_ARGUMENT, 0);
    BAIL_IF_MACRO(PHYSFS_read(file, &in, sizeof (in), 1) != 1, NULL, 0);
    *val = PHYSFS_swapSLE64(in);
    return(1);
} /* PHYSFS_readSLE64 */


int PHYSFS_readULE64(PHYSFS_File *file, PHYSFS_uint64 *val)
{
    PHYSFS_uint64 in;
    BAIL_IF_MACRO(val == NULL, ERR_INVALID_ARGUMENT, 0);
    BAIL_IF_MACRO(PHYSFS_read(file, &in, sizeof (in), 1) != 1, NULL, 0);
    *val = PHYSFS_swapULE64(in);
    return(1);
} /* PHYSFS_readULE64 */


int PHYSFS_readSBE64(PHYSFS_File *file, PHYSFS_sint64 *val)
{
    PHYSFS_sint64 in;
    BAIL_IF_MACRO(val == NULL, ERR_INVALID_ARGUMENT, 0);
    BAIL_IF_MACRO(PHYSFS_read(file, &in, sizeof (in), 1) != 1, NULL, 0);
    *val = PHYSFS_swapSBE64(in);
    return(1);
} /* PHYSFS_readSBE64 */


int PHYSFS_readUBE64(PHYSFS_File *file, PHYSFS_uint64 *val)
{
    PHYSFS_uint64 in;
    BAIL_IF_MACRO(val == NULL, ERR_INVALID_ARGUMENT, 0);
    BAIL_IF_MACRO(PHYSFS_read(file, &in, sizeof (in), 1) != 1, NULL, 0);
    *val = PHYSFS_swapUBE64(in);
    return(1);
} /* PHYSFS_readUBE64 */



int PHYSFS_writeSLE16(PHYSFS_File *file, PHYSFS_sint16 val)
{
    PHYSFS_sint16 out = PHYSFS_swapSLE16(val);
    BAIL_IF_MACRO(PHYSFS_write(file, &out, sizeof (out), 1) != 1, NULL, 0);
    return(1);
} /* PHYSFS_writeSLE16 */


int PHYSFS_writeULE16(PHYSFS_File *file, PHYSFS_uint16 val)
{
    PHYSFS_uint16 out = PHYSFS_swapULE16(val);
    BAIL_IF_MACRO(PHYSFS_write(file, &out, sizeof (out), 1) != 1, NULL, 0);
    return(1);
} /* PHYSFS_writeULE16 */


int PHYSFS_writeSBE16(PHYSFS_File *file, PHYSFS_sint16 val)
{
    PHYSFS_sint16 out = PHYSFS_swapSBE16(val);
    BAIL_IF_MACRO(PHYSFS_write(file, &out, sizeof (out), 1) != 1, NULL, 0);
    return(1);
} /* PHYSFS_writeSBE16 */


int PHYSFS_writeUBE16(PHYSFS_File *file, PHYSFS_uint16 val)
{
    PHYSFS_uint16 out = PHYSFS_swapUBE16(val);
    BAIL_IF_MACRO(PHYSFS_write(file, &out, sizeof (out), 1) != 1, NULL, 0);
    return(1);
} /* PHYSFS_writeUBE16 */


int PHYSFS_writeSLE32(PHYSFS_File *file, PHYSFS_sint32 val)
{
    PHYSFS_sint32 out = PHYSFS_swapSLE32(val);
    BAIL_IF_MACRO(PHYSFS_write(file, &out, sizeof (out), 1) != 1, NULL, 0);
    return(1);
} /* PHYSFS_writeSLE32 */


int PHYSFS_writeULE32(PHYSFS_File *file, PHYSFS_uint32 val)
{
    PHYSFS_uint32 out = PHYSFS_swapULE32(val);
    BAIL_IF_MACRO(PHYSFS_write(file, &out, sizeof (out), 1) != 1, NULL, 0);
    return(1);
} /* PHYSFS_writeULE32 */


int PHYSFS_writeSBE32(PHYSFS_File *file, PHYSFS_sint32 val)
{
    PHYSFS_sint32 out = PHYSFS_swapSBE32(val);
    BAIL_IF_MACRO(PHYSFS_write(file, &out, sizeof (out), 1) != 1, NULL, 0);
    return(1);
} /* PHYSFS_writeSBE32 */


int PHYSFS_writeUBE32(PHYSFS_File *file, PHYSFS_uint32 val)
{
    PHYSFS_uint32 out = PHYSFS_swapUBE32(val);
    BAIL_IF_MACRO(PHYSFS_write(file, &out, sizeof (out), 1) != 1, NULL, 0);
    return(1);
} /* PHYSFS_writeUBE32 */


int PHYSFS_writeSLE64(PHYSFS_File *file, PHYSFS_sint64 val)
{
    PHYSFS_sint64 out = PHYSFS_swapSLE64(val);
    BAIL_IF_MACRO(PHYSFS_write(file, &out, sizeof (out), 1) != 1, NULL, 0);
    return(1);
} /* PHYSFS_writeSLE64 */


int PHYSFS_writeULE64(PHYSFS_File *file, PHYSFS_uint64 val)
{
    PHYSFS_uint64 out = PHYSFS_swapULE64(val);
    BAIL_IF_MACRO(PHYSFS_write(file, &out, sizeof (out), 1) != 1, NULL, 0);
    return(1);
} /* PHYSFS_writeULE64 */


int PHYSFS_writeSBE64(PHYSFS_File *file, PHYSFS_sint64 val)
{
    PHYSFS_sint64 out = PHYSFS_swapSBE64(val);
    BAIL_IF_MACRO(PHYSFS_write(file, &out, sizeof (out), 1) != 1, NULL, 0);
    return(1);
} /* PHYSFS_writeSBE64 */


int PHYSFS_writeUBE64(PHYSFS_File *file, PHYSFS_uint64 val)
{
    PHYSFS_uint64 out = PHYSFS_swapUBE64(val);
    BAIL_IF_MACRO(PHYSFS_write(file, &out, sizeof (out), 1) != 1, NULL, 0);
    return(1);
} /* PHYSFS_writeUBE64 */

/* end of physfs_byteorder.c ... */

