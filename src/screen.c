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

void clearscreen(float r, float g, float b)
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

void fillrect(float x, float y, float w, float h, float r, float g, float b)
{
#ifndef NOOPENGL
if(use_gl)
	{
	glBegin(GL_QUADS);
		glColor3ub(r/256, g/256, b/256);
		glVertex2i(x, y);
		glVertex2i(x+w, y);
		glVertex2i(x+w, y+h);
		glVertex2i(x, y+h);
	glEnd();
	}
else
	{
#endif
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

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

void drawtext(char * text, int x, int y, SDL_Surface * surf, int update, int shadowsize)
{
	/* i - helps to keep tracking of the all string length
		j - helps to keep track of the length of the current line */
  int i, j, len;
  char c;
  SDL_Rect src, dest;
  
  len = strlen(text);
  
  /* For each letter in the string... */
  
  for (i = 0; i < len; ++i)
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
	else if (c == '\n')		/* support for multi-lines */
	{
	j = i + 1;
	y += 18;
	continue;
	}
      else
	src.x = -1;
      
      src.w = 16;
      src.h = 16;
      

      /* Draw character: */
      
      if (src.x != -1)
	{
	  /* Set destination rectangle for shadow: */
	  
	  dest.x = x + (i * 16) + shadowsize;
	  dest.y = y + shadowsize;
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
	  

/* FIXME: Text doesn't work in OpenGL mode, below is experimental code */
/*

	  dest.x = 0;
	  dest.y = 0; 
	  dest.w = src.w;
	  
	  dest.h = src.h;
	  
	  temp = SDL_CreateRGBSurface(SDL_SWSURFACE, dest.w, dest.h, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#else
            0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#endif
	  SDL_BlitSurface(letters_black, &src, temp, NULL);
	  texture_type xyz;
	  texture_from_sdl_surface(&xyz,temp,IGNORE_ALPHA);
	  texture_draw(&xyz,x + (i * 16) + shadowsize,y + shadowsize, update);
	  texture_free(&xyz);
	  / * Set destination rectangle for text: * /
	  
	  dest.x = x + (i * 16);
	  dest.y = y;
	  dest.w = src.w;
	  dest.h = src.h;	  
	  
	  / * Text: * /
	  
	  SDL_BlitSurface(surf, &src, temp, NULL);
	  texture_from_sdl_surface(&xyz,temp,IGNORE_ALPHA);
	  SDL_FreeSurface(temp);
	  texture_draw(&xyz,x + (i * 16) + shadowsize,y + shadowsize, update);
	  texture_free(&xyz);*/
	}
    }
  
  
  /* Update */
  
  if (update == UPDATE)
    {
      dest.w = len * 16 + 1;
      
      if (dest.w > screen->w)
	dest.w = screen->w;
      
      update_rect(screen, x, y, dest.w, 17);
    }
    
}


/* --- DRAW HORIZONTALLY-CENTERED TEXT: --- */

void drawcenteredtext(char * text, int y, SDL_Surface * surf, int update, int shadowsize)
{
  drawtext(text, screen->w / 2 - (strlen(text) * 8), y, surf, update, shadowsize);
}

/* --- ERASE TEXT: --- */

void erasetext(char * text, int x, int y, SDL_Surface * surf, int update, int shadowsize)
{
  SDL_Rect dest;
  
  
  dest.x = x;
  dest.y = y;
  dest.w = strlen(text) * 16 + shadowsize;
  dest.h = 17;
  
  if (dest.w > screen->w)
    dest.w = screen->w;
  
  SDL_BlitSurface(surf, &dest, screen, &dest);
  
  if (update == UPDATE)
    update_rect(screen, dest.x, dest.y, dest.w, dest.h);
}


/* --- ERASE CENTERED TEXT: --- */

void erasecenteredtext(char * text, int y, SDL_Surface * surf, int update, int shadowsize)
{
  erasetext(text, screen->w / 2 - (strlen(text) * 8), y, surf, update, shadowsize);
}
