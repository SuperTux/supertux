/*
  screen.c
  
  Super Tux - Screen Functions
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - April 22, 2000
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

/* --- LOAD AND DISPLAY AN IMAGE --- */

void load_and_display_image(char * file)
{
  SDL_Surface * img;

  img = load_image(file, IGNORE_ALPHA);
  SDL_BlitSurface(img, NULL, screen, NULL);
  SDL_FreeSurface(img);
}


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
#endif

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, r, g, b));

}

/* --- FILL A RECT --- */

void fillrect(float x, float y, float w, float h, int r, int g, int b, int a)
{
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

/* --- LOAD AN IMAGE --- */

SDL_Surface * load_image(char * file, int use_alpha)
{
  /*
  if(!faccessible(file))
  {
  if(!faccessible(st_dir,
  */

  SDL_Surface * temp, * surf;

  temp = IMG_Load(file);

  if (temp == NULL)
    st_abort("Can't load", file);

  surf = SDL_DisplayFormatAlpha(temp);

  if (surf == NULL)
    st_abort("Can't covert to display format", file);

  if (use_alpha == IGNORE_ALPHA)
    SDL_SetAlpha(surf, 0, 0);

  SDL_FreeSurface(temp);

  return(surf);
}

void update_rect(SDL_Surface *scr, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
  if(!use_gl)
    SDL_UpdateRect(scr, x, y, w, h);
}


/* --- ERASE TEXT: --- */

void erasetext(char * text, int x, int y, texture_type * ptexture, int update, int shadowsize)
{
  SDL_Rect dest;


  dest.x = x;
  dest.y = y;
  dest.w = strlen(text) * 16 + shadowsize;
  dest.h = 17;

  if (dest.w > screen->w)
    dest.w = screen->w;

  texture_draw_part(ptexture,dest.x,dest.y,dest.x,dest.y,dest.w,dest.h,update);

  if (update == UPDATE)
    update_rect(screen, dest.x, dest.y, dest.w, dest.h);
}


/* --- ERASE CENTERED TEXT: --- */

void erasecenteredtext(char * text, int y, texture_type * ptexture, int update, int shadowsize)
{
  erasetext(text, screen->w / 2 - (strlen(text) * 8), y, ptexture, update, shadowsize);
}
