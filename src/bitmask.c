/*
 *                          bitmask.c 1.0
 *                          -------------
 *    Simple and efficient bitmask collision detection routines
 *  Copyright (C) 2002 Ulf Ekstrom except for the bitcount function.
 *
 *           >  See the header file for more info. < 
 *  
 *  Please email bugs and comments to Ulf Ekstrom, ulfek@ifm.liu.se
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

#include <stdlib.h>
#include <stdio.h>
#include "bitmask.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))

bitmask *bitmask_create(int w, int h)
{
  bitmask *temp = (bitmask*) malloc(sizeof(bitmask));
  if (! temp)
    return 0;
  temp->w = w;
  temp->h = h;
  temp->bits = (long unsigned int*) calloc(h*((w - 1)/BITW_LEN + 1),sizeof(BITW));
  if (! temp->bits)
    {
      free(temp);
      return 0;
    }
  else
    return temp;
}

bitmask *bitmask_create_SDL(SDL_Surface* surf)
{
  int w,h;
  int bpp;
  Uint8* p;

  bitmask *temp = (bitmask*) malloc(sizeof(bitmask));
  if (! temp)
    return 0;
  temp->w = surf->w;
  temp->h = surf->h;
  temp->bits = (long unsigned int*) calloc(surf->h*((surf->w - 1)/BITW_LEN + 1),sizeof(BITW));
  if (! temp->bits)
    {
      free(temp);
      return 0;
    }
  else
    return temp;

  bpp = surf->format->BytesPerPixel;
  for(h = 0; h <= surf->h; ++h)
    {
      for(w = 0; w <= surf->h; ++w)
        {

          p = (Uint8 *)surf->pixels + h*surf->pitch + w*bpp;
          if(*p == 255)
            bitmask_setbit(temp,w,h);
        }
    }

}

void bitmask_free(bitmask *m)
{
  free(m->bits);
  free(m);
}

int bitmask_overlap(const bitmask *a,const bitmask *b,int xoffset, int yoffset)
{
  BITW *a_entry,*a_end;
  BITW *b_entry;
  BITW *ap,*bp;
  int shift,rshift,i,astripes,bstripes;

  if ((xoffset >= a->w) || (yoffset >= a->h) || (yoffset <= - b->h))
    return 0;

  if (xoffset >= 0)
    {
      if (yoffset >= 0)
        {
          a_entry = a->bits + a->h*(xoffset/BITW_LEN) + yoffset;
          a_end = a_entry + MIN(b->h,a->h - yoffset);
          b_entry = b->bits;
        }
      else
        {
          a_entry = a->bits + a->h*(xoffset/BITW_LEN);
          a_end = a_entry + MIN(b->h + yoffset,a->h);
          b_entry = b->bits - yoffset;
        }
      shift = xoffset & BITW_MASK;
      if (shift)
        {
          rshift = BITW_LEN - shift;
          astripes = (a->w - 1)/BITW_LEN - xoffset/BITW_LEN;
          bstripes = (b->w - 1)/BITW_LEN + 1;
          if (bstripes > astripes) /* zig-zag .. zig*/
            {
              for (i=0;i<astripes;i++)
                {
                  for (ap = a_entry,bp = b_entry;ap < a_end;) /* Join these two to one loop */
                    if ((*ap++ >> shift) & *bp++)
                      return 1;
                  a_entry += a->h;
                  a_end += a->h;
                  for (ap = a_entry,bp = b_entry;ap < a_end;)
                    if ((*ap++ << rshift) & *bp++)
                      return 1;
                  b_entry += b->h;
                }
              for (ap = a_entry,bp = b_entry;ap < a_end;)
                if ((*ap++ >> shift) & *bp++)
                  return 1;
              return 0;
            }
          else /* zig-zag */
            {
              for (i=0;i<bstripes;i++)
                {
                  for (ap = a_entry,bp = b_entry;ap < a_end;)
                    if ((*ap++ >> shift) & *bp++)
                      return 1;
                  a_entry += a->h;
                  a_end += a->h;
                  for (ap = a_entry,bp = b_entry;ap < a_end;)
                    if ((*ap++  << rshift) & *bp++)
                      return 1;
                  b_entry += b->h;
                }
              return 0;
            }
        }
      else /* xoffset is a multiple of the stripe width, and the above routines wont work */
        {
          astripes = (MIN(b->w,a->w - xoffset) - 1)/BITW_LEN + 1;
          for (i=0;i<astripes;i++)
            {
              for (ap = a_entry,bp = b_entry;ap < a_end;)
                if (*ap++ & *bp++)
                  return 1;
              a_entry += a->h;
              a_end += a->h;
              b_entry += b->h;
            }
          return 0;
        }
    }
  else
    return bitmask_overlap(b,a,-xoffset,-yoffset);
}

/* Will hang if there are no bits set in w! */
static INLINE int firstsetbit(BITW w)
{
  int i = 0;
  while ((w & 1) == 0)
    {
      i++;
      w/=2;
    }
  return i;
}

/* x and y are given in the coordinates of mask a, and are untouched if the is no overlap */
int bitmask_overlap_pos(const bitmask *a,const bitmask *b,int xoffset, int yoffset, int *x, int *y)
{
  BITW *a_entry,*a_end;
  BITW *b_entry;
  BITW *ap,*bp;
  int shift,rshift,i,astripes,bstripes;

  if ((xoffset >= a->w) || (yoffset >= a->h) || (yoffset <= - b->h))
    return 0;

  if (xoffset >= 0)
    {
      if (yoffset >= 0)
        {
          a_entry = a->bits + a->h*(xoffset/BITW_LEN) + yoffset;
          a_end = a_entry + MIN(b->h,a->h - yoffset);
          b_entry = b->bits;
        }
      else
        {
          a_entry = a->bits + a->h*(xoffset/BITW_LEN);
          a_end = a_entry + MIN(b->h + yoffset,a->h);
          b_entry = b->bits - yoffset;
        }
      shift = xoffset & BITW_MASK;
      if (shift)
        {
          rshift = BITW_LEN - shift;
          astripes = (a->w - 1)/BITW_LEN - xoffset/BITW_LEN;
          bstripes = (b->w - 1)/BITW_LEN + 1;
          if (bstripes > astripes) /* zig-zag .. zig*/
            {
              for (i=0;i<astripes;i++)
                {
                  for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                    if (*ap & (*bp << shift))
                      {
                        *y = (ap - a->bits) % a->h;
                        *x = ((ap - a->bits) / a->h)*BITW_LEN + firstsetbit(*ap & (*bp << shift));
                        return 1;
                      }
                  a_entry += a->h;
                  a_end += a->h;
                  for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                    if (*ap & (*bp >> rshift))
                      {
                        *y = (ap - a->bits) % a->h;
                        *x = ((ap - a->bits) / a->h)*BITW_LEN + firstsetbit(*ap & (*bp >> rshift));
                        return 1;
                      }
                  b_entry += b->h;
                }
              for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                if (*ap & (*bp << shift))
                  {
                    *y = (ap - a->bits) % a->h;
                    *x = ((ap - a->bits) / a->h)*BITW_LEN + firstsetbit(*ap & (*bp << shift));
                    return 1;
                  }
              return 0;
            }
          else /* zig-zag */
            {
              for (i=0;i<bstripes;i++)
                {
                  for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                    if (*ap & (*bp << shift))
                      {
                        *y = (ap - a->bits) % a->h;
                        *x = ((ap - a->bits) / a->h)*BITW_LEN + firstsetbit(*ap & (*bp << shift));
                        return 1;
                      }
                  a_entry += a->h;
                  a_end += a->h;
                  for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                    if (*ap & (*bp >> rshift))
                      {
                        *y = (ap - a->bits) % a->h;
                        *x = ((ap - a->bits) / a->h)*BITW_LEN + firstsetbit(*ap & (*bp >> rshift));
                        return 1;
                      }
                  b_entry += b->h;
                }
              return 0;
            }
        }
      else /* xoffset is a multiple of the stripe width, and the above routines won't work. */
        {
          astripes = (MIN(b->w,a->w - xoffset) - 1)/BITW_LEN + 1;
          for (i=0;i<astripes;i++)
            {
              for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                {
                  if (*ap & *bp)
                    {
                      *y = (ap - a->bits) % a->h;
                      *x = ((ap - a->bits) / a->h)*BITW_LEN + firstsetbit(*ap & *bp);
                      return 1;
                    }
                }
              a_entry += a->h;
              a_end += a->h;
              b_entry += b->h;
            }
          return 0;
        }
    }
  else
    {
      if (bitmask_overlap_pos(b,a,-xoffset,-yoffset,x,y))
        {
          *x += xoffset;
          *y += yoffset;
          return 1;
        }
      else
        return 0;
    }
}



/* (C) Donald W. Gillies, 1992.  All rights reserved.  You may reuse
   this bitcount() function anywhere you please as long as you retain
   this Copyright Notice. */
static INLINE int bitcount(unsigned long n)
{
  register unsigned long tmp;
  return (tmp = (n) - (((n) >> 1) & 033333333333) - (((n) >> 2) & 011111111111),\
          tmp = ((tmp + (tmp >> 3)) & 030707070707),			\
          tmp =  (tmp + (tmp >> 6)),					\
          tmp = (tmp + (tmp >> 12) + (tmp >> 24)) & 077);
}
/* End of Donald W. Gillies bitcount code */


int bitmask_overlap_area(const bitmask *a,const bitmask *b,int xoffset, int yoffset)
{
  BITW *a_entry,*a_end;
  BITW *b_entry;
  BITW *ap,*bp;
  int shift,rshift,i,astripes,bstripes;
  int count = 0;

  if ((xoffset >= a->w) || (yoffset >= a->h) || (yoffset <= - b->h))
    return 0;

  if (xoffset >= 0)
    {
      if (yoffset >= 0)
        {
          a_entry = a->bits + a->h*(xoffset/BITW_LEN) + yoffset;
          a_end = a_entry + MIN(b->h,a->h - yoffset);
          b_entry = b->bits;
        }
      else
        {
          a_entry = a->bits + a->h*(xoffset/BITW_LEN);
          a_end = a_entry + MIN(b->h + yoffset,a->h);
          b_entry = b->bits - yoffset;
        }
      shift = xoffset & BITW_MASK;
      if (shift)
        {
          rshift = BITW_LEN - shift;
          astripes = (a->w - 1)/BITW_LEN - xoffset/BITW_LEN;
          bstripes = (b->w - 1)/BITW_LEN + 1;
          if (bstripes > astripes) /* zig-zag .. zig*/
            {
              for (i=0;i<astripes;i++)
                {
                  for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                    count += bitcount(((*ap >> shift) | (*(ap + a->h) << rshift)) & *bp);
                  a_entry += a->h;
                  a_end += a->h;
                  b_entry += b->h;
                }
              for (ap = a_entry,bp = b_entry;ap < a_end;)
                count += bitcount((*ap++ >> shift) & *bp++);
              return count;
            }
          else /* zig-zag */
            {
              for (i=0;i<bstripes;i++)
                {
                  for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                    count += bitcount(((*ap >> shift) | (*(ap + a->h) << rshift)) & *bp);
                  a_entry += a->h;
                  a_end += a->h;
                  b_entry += b->h;
                }
              return count;
            }
        }
      else /* xoffset is a multiple of the stripe width, and the above routines wont work */
        {
          astripes = (MIN(b->w,a->w - xoffset) - 1)/BITW_LEN + 1;
          for (i=0;i<astripes;i++)
            {
              for (ap = a_entry,bp = b_entry;ap < a_end;)
                count += bitcount(*ap++ & *bp++);

              a_entry += a->h;
              a_end += a->h;
              b_entry += b->h;
            }
          return count;
        }
    }
  else
    return bitmask_overlap_area(b,a,-xoffset,-yoffset);
}


/* Draws mask b onto mask a (bitwise OR) */
void bitmask_draw(bitmask *a,bitmask *b,int xoffset, int yoffset)
{
  BITW *a_entry,*a_end;
  BITW *b_entry;
  BITW *ap,*bp;
  bitmask *swap;
  int shift,rshift,i,astripes,bstripes;

  if ((xoffset >= a->w) || (yoffset >= a->h) || (yoffset <= - b->h))
    return;

  if (xoffset >= 0)
    {
      if (yoffset >= 0)
        {
          a_entry = a->bits + a->h*(xoffset/BITW_LEN) + yoffset;
          a_end = a_entry + MIN(b->h,a->h - yoffset);
          b_entry = b->bits;
        }
      else
        {
          a_entry = a->bits + a->h*(xoffset/BITW_LEN);
          a_end = a_entry + MIN(b->h + yoffset,a->h);
          b_entry = b->bits - yoffset;
        }
      shift = xoffset & BITW_MASK;
      if (shift)
        {
          rshift = BITW_LEN - shift;
          astripes = (a->w - 1)/BITW_LEN - xoffset/BITW_LEN;
          bstripes = (b->w - 1)/BITW_LEN + 1;
          if (bstripes > astripes) /* zig-zag .. zig*/
            {
              for (i=0;i<astripes;i++)
                {
                  for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                    *ap |= (*bp << shift);
                  a_entry += a->h;
                  a_end += a->h;
                  for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                    *ap |= (*bp >> rshift);
                  b_entry += b->h;
                }
              for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                *ap |= (*bp << shift);
              return;
            }
          else /* zig-zag */
            {
              for (i=0;i<bstripes;i++)
                {
                  for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                    *ap |= (*bp << shift);
                  a_entry += a->h;
                  a_end += a->h;
                  for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                    *ap |= (*bp >> rshift);
                  b_entry += b->h;
                }
              return;
            }
        }
      else /* xoffset is a multiple of the stripe width, and the above routines won't work. */
        {
          astripes = (MIN(b->w,a->w - xoffset) - 1)/BITW_LEN + 1;
          for (i=0;i<astripes;i++)
            {
              for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                {
                  *ap |= *bp;
                }
              a_entry += a->h;
              a_end += a->h;
              b_entry += b->h;
            }
          return;
        }
    }
  else
    {
      /* 'Swapping' arguments to be able to almost reuse the code above */
      swap = a;
      a = b;
      b = swap;
      xoffset *= -1;
      yoffset *= -1;

      if (yoffset >= 0)
        {
          a_entry = a->bits + a->h*(xoffset/BITW_LEN) + yoffset;
          a_end = a_entry + MIN(b->h,a->h - yoffset);
          b_entry = b->bits;
        }
      else
        {
          a_entry = a->bits + a->h*(xoffset/BITW_LEN);
          a_end = a_entry + MIN(b->h + yoffset,a->h);
          b_entry = b->bits - yoffset;
        }
      shift = xoffset & BITW_MASK;
      if (shift)
        {
          rshift = BITW_LEN - shift;
          astripes = (a->w - 1)/BITW_LEN - xoffset/BITW_LEN;
          bstripes = (b->w - 1)/BITW_LEN + 1;
          if (bstripes > astripes) /* zig-zag .. zig*/
            {
              for (i=0;i<astripes;i++)
                {
                  for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                    *bp |= (*ap >> shift);
                  a_entry += a->h;
                  a_end += a->h;
                  for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                    *bp |= (*ap <<rshift);
                  b_entry += b->h;
                }
              for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                *bp |= (*ap >> shift);
              return;
            }
          else /* zig-zag */
            {
              for (i=0;i<bstripes;i++)
                {
                  for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                    *bp |= (*ap >> shift);
                  a_entry += a->h;
                  a_end += a->h;
                  for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                    *bp |= (*ap << rshift);
                  b_entry += b->h;
                }
              return;
            }
        }
      else /* xoffset is a multiple of the stripe width, and the above routines won't work. */
        {
          astripes = (MIN(b->w,a->w - xoffset) - 1)/BITW_LEN + 1;
          for (i=0;i<astripes;i++)
            {
              for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                {
                  *bp |= *ap;
                }
              a_entry += a->h;
              a_end += a->h;
              b_entry += b->h;
            }
          return;
        }
    }
}
