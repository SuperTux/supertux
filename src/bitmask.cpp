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

#include <config.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
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

/* (C) Tobias Glaesser <tobi.web@gmx.de>, 2004.
   This function isn't available in the original bitmask library. */
bitmask *bitmask_create_SDL(SDL_Surface* surf)
{
  bitmask* pbitmask = bitmask_create(surf->w, surf->h);
  int w,h;
  SDL_PixelFormat* fmt;
  Uint32 temp, pixel;
  Uint8 alpha;

  if(surf->format->BytesPerPixel != 4) //This function only works for true-color surfaces with an alpha channel
    return 0;

  fmt = surf->format;
  for(h = 0; h <= surf->h; ++h)
    {
      for(w = 0; w <= surf->w; ++w)
        {

          pixel = *((Uint32*)((Uint8 *)surf->pixels + h * surf->pitch + w * 4));
          /* Get Alpha component */
          temp=pixel&fmt->Amask; /* Isolate alpha component */
          temp=temp>>fmt->Ashift;/* Shift it down to 8-bit */
          temp=temp<<fmt->Aloss; /* Expand to a full 8-bit number */
          alpha=(Uint8)temp;
          if (fmt->Amask == 0 || alpha != 0)
            bitmask_setbit(pbitmask,w,h);
        }
    }
  return pbitmask;
}
void bitmask_free(bitmask *m)
{
  free(m->bits);
  free(m);
}

void bitmask_clear(bitmask *m)
{
  memset(m->bits,0,m->h*((m->w - 1)/BITW_LEN + 1)*sizeof(BITW));
}

void bitmask_fill(bitmask *m)
{
  memset(m->bits,255,m->h*((m->w - 1)/BITW_LEN + 1)*sizeof(BITW));
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
  int shift,rshift,i,astripes,bstripes,xbase;

  if ((xoffset >= a->w) || (yoffset >= a->h) || (yoffset <= - b->h))
    return 0;

  if (xoffset >= 0)
    {
      xbase = xoffset/BITW_LEN; /* first stripe from mask a */    
      if (yoffset >= 0)
        {
          a_entry = a->bits + a->h*xbase + yoffset;
          a_end = a_entry + MIN(b->h,a->h - yoffset);
          b_entry = b->bits;
        }
      else
        {
          a_entry = a->bits + a->h*xbase;
          a_end = a_entry + MIN(b->h + yoffset,a->h);
          b_entry = b->bits - yoffset;
          yoffset = 0; /* relied on below */
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
                        *y = ap - a_entry + yoffset;
                        *x = (xbase + i)*BITW_LEN + firstsetbit(*ap & (*bp << shift));
                        return 1;
                      }
                  a_entry += a->h;
                  a_end += a->h;
                  for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                    if (*ap & (*bp >> rshift))
                      {
                        *y = ap - a_entry + yoffset;
                        *x = (xbase + i + 1)*BITW_LEN + firstsetbit(*ap & (*bp >> rshift));
                        return 1;
                      }
                  b_entry += b->h;
                }
              for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                if (*ap & (*bp << shift))
                  {
                    *y = ap - a_entry + yoffset;
                    *x = (xbase + astripes)*BITW_LEN + firstsetbit(*ap & (*bp << shift));
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
                        *y = ap - a_entry + yoffset;
                        *x = (xbase + i)*BITW_LEN + firstsetbit(*ap & (*bp << shift));
                        return 1;
                      }
                  a_entry += a->h;
                  a_end += a->h;
                  for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                    if (*ap & (*bp >> rshift))
                      {
                        *y = ap - a_entry + yoffset;
                        *x = (xbase + i + 1)*BITW_LEN + firstsetbit(*ap & (*bp >> rshift));
                        return 1;
                      }
                  b_entry += b->h;
                }
              return 0;
            }
        }
      else /* xoffset is a multiple of the stripe width, and the above routines
           won't work. This way is also slightly faster. */
        {
          astripes = (MIN(b->w,a->w - xoffset) - 1)/BITW_LEN + 1;
          for (i=0;i<astripes;i++)
            {
              for (ap = a_entry,bp = b_entry;ap < a_end;ap++,bp++)
                {
                  if (*ap & *bp)
                    {
                       *y = ap - a_entry + yoffset;
                       *x = (xbase + i)*BITW_LEN + firstsetbit(*ap & *bp);
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

static INLINE int bitcount(unsigned long n)
{
  if (BITW_LEN == 32)
    {
      /* (C) Donald W. Gillies, 1992.  All rights reserved.  You may reuse
         this bitcount() function anywhere you please as long as you retain
         this Copyright Notice. */
      register unsigned long tmp;
      return (tmp = (n) - (((n) >> 1) & 033333333333) -
                    (((n) >> 2) & 011111111111),
              tmp = ((tmp + (tmp >> 3)) & 030707070707),
              tmp =  (tmp + (tmp >> 6)),
              tmp = (tmp + (tmp >> 12) + (tmp >> 24)) & 077);
      /* End of Donald W. Gillies bitcount code */
    }
  else
    {
      /* Handle non-32 bit case the slow way */
      int nbits = 0;
      while (n)
        {
          if (n & 1)
            nbits++;
          n = n >> 1;
        }
      return nbits;
    }
}


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
      /* 'Swapping' arguments to be able to almost reuse the code above,
       should be taken care of by the compiler efficiently. */
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
