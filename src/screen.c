/*
  screen.c
  
  Super Tux - Screen Functions
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - March 15, 2004
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>

#ifdef LINUX
#include <pwd.h>
#include <sys/types.h>
#include <ctype.h>
#endif

#include "defines.h"
#include "globals.h"
#include "screen.h"
#include "setup.h"
#include "type.h"

/* Needed for line calculations */
#define SGN(x) ((x)>0 ? 1 : ((x)==0 ? 0:(-1)))
#define ABS(x) ((x)>0 ? (x) : (-x))

/* --- CLEAR SCREEN --- */

void clearscreen(int r, int g, int b)
{
#ifndef NOOPENGL
  if(use_gl)
    {
      glClearColor(r/256, g/256, b/256, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);
    }
  else
  {
#endif

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, r, g, b));
#ifndef NOOPENGL

    }
#endif
}

/* 'Stolen' from the SDL documentation.
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 */
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
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
void drawpixel(int x, int y, Uint32 pixel)
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

void drawline(int x1, int y1, int x2, int y2, int r, int g, int b, int a)
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

      lg_delta = x2 - x1;
      sh_delta = y2 - y1;
      lg_step = SGN(lg_delta);
      lg_delta = ABS(lg_delta);
      sh_step = SGN(sh_delta);
      sh_delta = ABS(sh_delta);
      if (sh_delta < lg_delta)
        {
          cycle = lg_delta >> 1;
          while (x1 != x2)
            {
              drawpixel(x1, y1, color);
              cycle += sh_delta;
              if (cycle > lg_delta)
                {
                  cycle -= lg_delta;
                  y1 += sh_step;
                }
              x1 += lg_step;
            }
          drawpixel(x1, y1, color);
        }
      cycle = sh_delta >> 1;
      while (y1 != y2)
        {
          drawpixel(x1, y1, color);
          cycle += lg_delta;
          if (cycle > sh_delta)
            {
              cycle -= sh_delta;
              x1 += lg_step;
            }
          y1 += sh_step;
        }
      drawpixel(x1, y1, color);
#ifndef NOOPENGL

    }
#endif
}

/* --- FILL A RECT --- */

void fillrect(float x, float y, float w, float h, int r, int g, int b, int a)
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


/* --- UPDATE SCREEN --- */

void updatescreen(void)
{
  if(use_gl)  /*clearscreen(0,0,0);*/
    SDL_GL_SwapBuffers();
  else
    SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
}

void flipscreen(void)
{
  if(use_gl)
    SDL_GL_SwapBuffers();
  else
    SDL_Flip(screen);
}

void update_rect(SDL_Surface *scr, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
  if(!use_gl)
    SDL_UpdateRect(scr, x, y, w, h);
}

