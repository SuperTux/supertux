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
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, r, g, b));
}


/* --- UPDATE SCREEN --- */

void updatescreen(void)
{
  SDL_UpdateRect(screen, 0, 0, 640, 480);
}


/* --- LOAD AN IMAGE --- */

SDL_Surface * load_image(char * file, int use_alpha)
{
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


/* --- DRAW AN IMAGE ONTO THE SCREEN --- */

void drawimage(SDL_Surface * surf, int x, int y, int update)
{
  SDL_Rect dest;
  
  dest.x = x;
  dest.y = y;
  dest.w = surf->w;
  dest.h = surf->h;
  
  SDL_BlitSurface(surf, NULL, screen, &dest);
  
  if (update == UPDATE)
    SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);
}


/* --- DRAW PART OF AN IMAGE ONTO THE SCREEN --- */

void drawpart(SDL_Surface * surf, int x, int y, int w, int h, int update)
{
  SDL_Rect src, dest;
  
  src.x = x;
  src.y = y;
  src.w = w;
  src.h = h;

  dest.x = x;
  dest.y = y;
  dest.w = w;
  dest.h = h;
  
  SDL_BlitSurface(surf, &src, screen, &dest);
  
  if (update == UPDATE)
    SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);
}


/* --- DRAW TEXT ONTO THE SCREEN --- */

void drawtext(char * text, int x, int y, SDL_Surface * surf, int update)
{
  int i;
  char c;
  SDL_Rect src, dest;
  
  
  /* For each letter in the string... */
  
  for (i = 0; i < strlen(text); i++)
    {
      /* Set source rectangle: */
      
      c = text[i];
      
      if (c >= 'A' && c <= 'Z')
	{
	  /* Capital letter - first row: */
	  
	  src.x = (c - 'A') * 16;
	  src.y = 0;
	}
      else if (c >= 'a' && c <= 'z')
	{
	  /* Lowercase letter - first row: */
	  
	  src.x = (c - 'a') * 16;
	  src.y = 16;
	}
      else if (c >= '!' && c <= '9')
	{
	  /* Punctuation (except '?') or number - third row: */
	  
	  src.x = (c - '!') * 16;
	  src.y = 32;
	}
      else if (c == '?')
	{
	  /* Question mark - third row, last character: */
	  
	  src.x = 400;
	  src.y = 24;
	}
      else
	src.x = -1;
      
      src.w = 16;
      src.h = 16;
      

      /* Draw character: */
      
      if (src.x != -1)
	{
	  /* Set destination rectangle for shadow: */
	  
	  dest.x = x + (i * 16) + 1;
	  dest.y = y + 1;
	  dest.w = src.w;
	  dest.h = src.h;
	  
	  
	  /* Shadow: */
	  
	  SDL_BlitSurface(letters_black, &src, screen, &dest);
	  
	  
	  /* Set destination rectangle for text: */
	  
	  dest.x = x + (i * 16);
	  dest.y = y;
	  dest.w = src.w;
	  dest.h = src.h;
	  
	  
	  /* Shadow: */
	  
	  SDL_BlitSurface(surf, &src, screen, &dest);
	}
    }
  
  
  /* Update */
  
  if (update == UPDATE)
    {
      dest.w = strlen(text) * 16 + 1;
      
      if (dest.w > 640)
	dest.w = 640;
      
      SDL_UpdateRect(screen, x, y, dest.w, 17);
    }
}


/* --- DRAW HORIZONTALLY-CENTERED TEXT: --- */

void drawcenteredtext(char * text, int y, SDL_Surface * surf, int update)
{
  drawtext(text, 320 - (strlen(text) * 8), y, surf, update);
}


/* --- ERASE TEXT: --- */

void erasetext(char * text, int x, int y, SDL_Surface * surf, int update)
{
  SDL_Rect dest;
  
  
  dest.x = x;
  dest.y = y;
  dest.w = strlen(text) * 16 + 1;
  dest.h = 17;
  
  if (dest.w > 640)
    dest.w = 640;
  
  SDL_BlitSurface(surf, &dest, screen, &dest);
  
  if (update == UPDATE)
    SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);
}


/* --- ERASE CENTERED TEXT: --- */

void erasecenteredtext(char * text, int y, SDL_Surface * surf, int update)
{
  erasetext(text, 320 - (strlen(text) * 8), y, surf, update);
}
