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
  if(use_gl)
  {
  glClearColor(r/256, g/256, b/256, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  }
  else
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, r, g, b));
 
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

void create_gl_texture(SDL_Surface * surf, GLint * tex)
{
SDL_Surface *conv;
conv = SDL_CreateRGBSurface(SDL_SWSURFACE , surf->w, surf->h, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#else
            0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#endif
    SDL_BlitSurface(surf, 0, conv, 0);
          	     glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
       glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
  glGenTextures(1, &*tex);

    glBindTexture(GL_TEXTURE_RECTANGLE_NV , *tex);
             glEnable(GL_TEXTURE_RECTANGLE_NV);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, conv->pitch / conv->format->BytesPerPixel);
    glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 3, conv->w, conv->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, conv->pixels);
    //glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, conv->w, conv->h);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    SDL_FreeSurface(conv);
}

/* --- DRAW AN IMAGE ONTO THE SCREEN --- */
/*
void drawimage(SDL_Surface * surf, float x, float y, int update)
{
if(use_gl)
{
GLint gl_tex;
create_gl_texture(surf,&gl_tex);
    glColor4ub(255, 255, 255,255);
glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
glEnable (GL_BLEND);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, gl_tex);

    glBegin(GL_QUADS);
        glTexCoord2f(0, 0);    glVertex2f(x, y);
        glTexCoord2f((float)surf->w, 0);    glVertex2f((float)surf->w+x, y);
        glTexCoord2f((float)surf->w, (float)surf->h);    glVertex2f((float)surf->w+x, (float)surf->h+y);
        glTexCoord2f(0, (float)surf->h);    glVertex2f(x, (float)surf->h+y);
    glEnd();
 glDeleteTextures(1, &gl_tex);
 }
else
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
}
*/
/*
drawbgimage(SDL_Surface * surf, int update)
{
if(use_gl)
{
GLint gl_tex;
create_gl_texture(surf,&gl_tex);
    //glColor3ub(255, 255, 255);

    glEnable(GL_TEXTURE_RECTANGLE_NV);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, gl_tex);

    glBegin(GL_QUADS);
        glTexCoord2f(0, 0);    glVertex2f(0, 0);
        glTexCoord2f((float)surf->w, 0);    glVertex2f(screen->w, 0);
        glTexCoord2f((float)surf->w, (float)surf->h);    glVertex2f(screen->w, screen->h);
        glTexCoord2f(0, (float)surf->h); glVertex2f(0, screen->h);
    glEnd();
 glDeleteTextures(1, &gl_tex);
 
}
else
{
  SDL_Rect dest;
  
  dest.x = 0;
  dest.y = 0;
  dest.w = screen->w;
  dest.h = screen->h;
  
  SDL_BlitSurface(surf, NULL, screen, &dest);
  
  if (update == UPDATE)
    SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);
}
}
*/
void update_rect(SDL_Surface *scr, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
if(!use_gl)
SDL_UpdateRect(scr, x, y, w, h);
}


/* --- DRAW PART OF AN IMAGE ONTO THE SCREEN --- */
/*
void drawpart(SDL_Surface * surf, float x, float y, float w, float h, int update)
{
if(use_gl)
{
GLint gl_tex;
create_gl_texture(surf,&gl_tex);
    glColor3ub(255, 255, 255);

    glEnable(GL_TEXTURE_RECTANGLE_NV);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, gl_tex);

    glBegin(GL_QUADS);
        glTexCoord2f(x, y);    glVertex2f(x, y);
        glTexCoord2f(x+w, y);    glVertex2f(w+x, y);
        glTexCoord2f(x+w, y+h);    glVertex2f(w+x, h+y);
        glTexCoord2f(x, y+h);     glVertex2f(x, h+y);
    glEnd();
 glDeleteTextures(1, &gl_tex);
 }
else
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
    update_rect(screen, dest.x, dest.y, dest.w, dest.h);
    }
}
*/
/* --- DRAW TEXT ONTO THE SCREEN --- */

void drawtext(char * text, int x, int y, SDL_Surface * surf, int update, int shadowsize)
{
	/* i - helps to keep tracking of the all string length
		j - helps to keep track of the length of the current line */
  int i, j;
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
      dest.w = strlen(text) * 16 + 1;
      
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
