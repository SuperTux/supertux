//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <config.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <unistd.h>

#include "SDL.h"
#include "SDL_image.h"

#ifndef WIN32
#include <sys/types.h>
#include <ctype.h>
#endif

#include "screen.h"
#include "app/globals.h"
#include "video/drawing_context.h"
#include "math/vector.h"

using namespace SuperTux;

/* 'Stolen' from the SDL documentation.
 * Return the pixel value at (x, y)
 * NOTE: The surface must be locked before calling this!
 */
Uint32 SuperTux::getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}

/* 'Stolen' from the SDL documentation.
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 */
void SuperTux::putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to set */
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp)
    {
    case 1:
      *p = pixel;
      break;

    case 2:
      *(Uint16 *)p = pixel;
      break;

    case 3:
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
        {
          p[0] = (pixel >> 16) & 0xff;
          p[1] = (pixel >> 8) & 0xff;
          p[2] = pixel & 0xff;
        }
      else
        {
          p[0] = pixel & 0xff;
          p[1] = (pixel >> 8) & 0xff;
          p[2] = (pixel >> 16) & 0xff;
        }
      break;

    case 4:
      *(Uint32 *)p = pixel;
      break;
    }
}

/* Draw a single pixel on the screen. */
void SuperTux::drawpixel(int x, int y, Uint32 pixel)
{
  /* Lock the screen for direct access to the pixels */
  if ( SDL_MUSTLOCK(screen) )
    {
      if ( SDL_LockSurface(screen) < 0 )
        {
          fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
          return;
        }
    }

  if(!(x < 0 || y < 0 || x > screen->w || y > screen->h))
    putpixel(screen, x, y, pixel);

  if ( SDL_MUSTLOCK(screen) )
    {
      SDL_UnlockSurface(screen);
    }
  /* Update just the part of the display that we've changed */
  SDL_UpdateRect(screen, x, y, 1, 1);
}

/* --- FILL A RECT --- */

void SuperTux::fillrect(float x, float y, float w, float h, int r, int g, int b, int a)
{
if(w < 0)
	{
	x += w;
	w = -w;
	}
if(h < 0)
	{
	y += h;
	h = -h;
	}

#ifndef NOOPENGL
  if(use_gl)
    {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glColor4ub(r, g, b,a);

      glBegin(GL_POLYGON);
      glVertex2f(x, y);
      glVertex2f(x+w, y);
      glVertex2f(x+w, y+h);
      glVertex2f(x, y+h);
      glEnd();
      glDisable(GL_BLEND);
    }
  else
    {
#endif
      SDL_Rect src, rect;
      SDL_Surface *temp = NULL;

      rect.x = (int)x;
      rect.y = (int)y;
      rect.w = (int)w;
      rect.h = (int)h;

      if(a != 255)
        {
          temp = SDL_CreateRGBSurface(screen->flags, rect.w, rect.h, screen->format->BitsPerPixel,
                                      screen->format->Rmask,
                                      screen->format->Gmask,
                                      screen->format->Bmask,
                                      screen->format->Amask);


          src.x = 0;
          src.y = 0;
          src.w = rect.w;
          src.h = rect.h;

          SDL_FillRect(temp, &src, SDL_MapRGB(screen->format, r, g, b));

          SDL_SetAlpha(temp, SDL_SRCALPHA, a);

          SDL_BlitSurface(temp,0,screen,&rect);

          SDL_FreeSurface(temp);
        }
      else
        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, r, g, b));

#ifndef NOOPENGL

    }
#endif
}

/* Needed for line calculations */
#define SGN(x) ((x)>0 ? 1 : ((x)==0 ? 0:(-1)))
#define ABS(x) ((x)>0 ? (x) : (-x))

void SuperTux::draw_line(float x1, float y1, float x2, float y2, int r, int g, int b, int a)
{
#ifndef NOOPENGL
  if(use_gl)
    {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glColor4ub(r, g, b,a);

      glBegin(GL_LINES);
      glVertex2f(x1, y1);
      glVertex2f(x2, y2);
      glEnd();
      glDisable(GL_BLEND);
    }
  else
    {
#endif
      /* Basic unantialiased Bresenham line algorithm */
      int lg_delta, sh_delta, cycle, lg_step, sh_step;
      Uint32 color = SDL_MapRGBA(screen->format, r, g, b, a);

      lg_delta = (int)(x2 - x1);
      sh_delta = (int)(y2 - y1);
      lg_step = SGN(lg_delta);
      lg_delta = ABS(lg_delta);
      sh_step = SGN(sh_delta);
      sh_delta = ABS(sh_delta);
      if (sh_delta < lg_delta)
        {
          cycle = lg_delta >> 1;
          while (x1 != x2)
            {
              drawpixel((int)x1, (int)y1, color);
              cycle += sh_delta;
              if (cycle > lg_delta)
                {
                  cycle -= lg_delta;
                  y1 += sh_step;
                }
              x1 += lg_step;
            }
          drawpixel((int)x1, (int)y1, color);
        }
      cycle = sh_delta >> 1;
      while (y1 != y2)
        {
          drawpixel((int)x1, (int)y1, color);
          cycle += lg_delta;
          if (cycle > sh_delta)
            {
              cycle -= sh_delta;
              x1 += lg_step;
            }
          y1 += sh_step;
        }
      drawpixel((int)x1, (int)y1, color);
#ifndef NOOPENGL

    }
#endif
}

#define LOOP_DELAY 20.0

void SuperTux::fadeout(int fade_time)
{
  float alpha_inc  = 256 / (fade_time / LOOP_DELAY);
  float alpha = 256;

  while(alpha > 0)
    {
    alpha -= alpha_inc;
    fillrect(0, 0, screen->w, screen->h, 0,0,0, (int)alpha_inc);  // left side
                                                   
    DrawingContext context; // ugly...
    context.do_drawing();

    SDL_Delay(int(LOOP_DELAY));
    }

  fillrect(0, 0, screen->w, screen->h, 0, 0, 0, 255);

}

void SuperTux::shrink_fade(const Vector& point, int fade_time)
{
  float left_inc  = point.x / ((float)fade_time / LOOP_DELAY);
  float right_inc = (screen->w - point.x) / ((float)fade_time / LOOP_DELAY);
  float up_inc    = point.y / ((float)fade_time / LOOP_DELAY);
  float down_inc  = (screen->h - point.y) / ((float)fade_time / LOOP_DELAY);
                                                                                
  float left_cor = 0, right_cor = 0, up_cor = 0, down_cor = 0;
                                                                                
  while(left_cor < point.x && right_cor < screen->w - point.x &&
      up_cor < point.y && down_cor < screen->h - point.y)
  {
    left_cor  += left_inc;
    right_cor += right_inc;
    up_cor    += up_inc;
    down_cor  += down_inc;
                                                                                
    fillrect(0, 0, left_cor, screen->h, 0,0,0);  // left side
    fillrect(screen->w - right_cor, 0, right_cor, screen->h, 0,0,0);  // right side
    fillrect(0, 0, screen->w, up_cor, 0,0,0);  // up side
    fillrect(0, screen->h - down_cor, screen->w, down_cor+1, 0,0,0);  // down side                                                                                
    DrawingContext context; // ugly...
    context.do_drawing();
    
    SDL_Delay(int(LOOP_DELAY));
  }
}

