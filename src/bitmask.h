/*
 *                         bitmask.c 1.0 
 *                         -------------
 *    Simple and efficient bitmask collision detection routines
 *  Copyright (C) 2002 Ulf Ekstrom except for the bitcount function.
 *
 *  A bitmask is a simple array of bits, which can be used for 
 *  2d collision detection. Set 'unoccupied' area to zero and
 *  occupies areas to one and use the bitmask_overlap*() functions
 *  to check for collisions.
 *  The current implementation uses 32 bit wide stripes to hold  
 *  the masks, but should work just as well with 64 bit sizes.
 *  (Note that the current bitcount function is 32 bit only!)
 *
 *  The overlap tests uses the following offsets (which may be negative):
 *
 *   +----+----------..
 *   |A   | yoffset   
 *   |  +-+----------..
 *   +--|B        
 *   |xoffset      
 *   |  |
 *   :  :  
 *
 *  For optimal collision detection performance combine these functions
 *  with some kind of pre-sorting to avoid comparing objects far from 
 *  each other.
 *
 *  BUGS: No known bugs, even though they may certainly be in here somewhere.
 *  Possible performance improvements could be to remove the div in 
 *  bitmask_overlap_pos() and to implement wider stripes if the masks used
 *  are wider than 64 bits on the average.
 *
 *  Performance of the various functions goes something like: 
 *  (relative timings, smaller is better)
 * 
 *  bitmask_overlap()        1.0
 *  bitmask_overlap_pos()    1.3
 *  bitmask_overlap_area()   1.6
 *
 *  For maximum performance on my machine I use gcc with
 *  -O2 -fomit-frame-pointer -funroll-loops 
 *
 *
 *  If you can make these functions faster or have found any bugs please
 *  email Ulf Ekstrom, ulfek@ifm.liu.se 
 *
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef SUPERTUX_BITMASK_H
#define SUPERTUX_BITMASK_H

#include <SDL.h>

/* Define INLINE for different compilers. */
#ifndef INLINE
# ifdef __GNUC__
#  define INLINE __inline__
# else
#  ifdef _MSC_VER
#   define INLINE __inline
#  else
#   define INLINE
#  endif
# endif
#endif

#define BITW unsigned long int
#define BITW_LEN 32
#define BITW_MASK 31
#define BITN(n) ((BITW)1 << (n))

struct bitmask
{
  int w,h;
  BITW *bits;
};

/* Creates a bitmask of width w and height h.
 * The mask is automatically cleared when created.
 */
bitmask *bitmask_create(int w, int h);
void bitmask_free(bitmask *m);

/* Returns nonzero if the bit at (x,y) is set. 
 * Coordinates start at (0,0)
 */
static INLINE int bitmask_getbit(const bitmask *m,int x,int y) 
{ 
  return m->bits[x/BITW_LEN*m->h + y] & BITN(x & BITW_MASK); 
}


/* Sets the bit at (x,y) */
static INLINE void bitmask_setbit(bitmask *m,int x,int y)
{ 
  m->bits[x/BITW_LEN*m->h + y] |= BITN(x & BITW_MASK); 
}


/* Clears the bit at (x,y) */
static INLINE void bitmask_clearbit(bitmask *m,int x,int y)
{ 
  m->bits[x/BITW_LEN*m->h + y] &= ~BITN(x & BITW_MASK); 
}


/* Returns nonzero if the masks overlap with the given offset. */
int bitmask_overlap(const bitmask *a,const bitmask *b,int xoffset, int yoffset);

/* Like bitmask_overlap(), but will also give a point of intersection.
 * x and y are given in the coordinates of mask a, and are untouched if the is 
 * no overlap.
 */
int bitmask_overlap_pos(const bitmask *a,const bitmask *b,int xoffset, int yoffset, int *x, int *y);

/* Returns the number of overlapping 'pixels' */
int bitmask_overlap_area(const bitmask *a,const bitmask *b,int xoffset, int yoffset);

/* Draws mask b onto mask a (bitwise OR) 
 * Can be used to compose large (game background?) mask from 
 * several submasks, which may speed up the testing. 
 */
void bitmask_draw(bitmask *a,bitmask *b,int xoffset, int yoffset);

/* Create a bitmask from a SDL_Surface */
bitmask* bitmask_create_SDL(SDL_Surface* surf);

#endif /*SUPERTUX_BITMASK_H*/
