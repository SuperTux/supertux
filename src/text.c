//
// C Implementation: text
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "defines.h"
#include "screen.h"
#include "text.h"

void text_load(text_type* ptext, char* file, int kind, int w, int h)
{
  int x, y;
  int mx, my;
  SDL_Surface *conv;

  if(kind == TEXT_TEXT)
    {
      ptext->chars = (texture_type*) malloc(sizeof(texture_type) * 79);
      ptext->shadow_chars = (texture_type*) malloc(sizeof(texture_type) * 79);
      mx = 26;
      my = 3;
    }
  else if(kind == TEXT_NUM)
    {
      ptext->chars = (texture_type*) malloc(sizeof(texture_type) * 10);
      ptext->shadow_chars = (texture_type*) malloc(sizeof(texture_type) * 10);
      mx = 10;
      my = 1;
    }
  else
    {
      ptext->chars = NULL;
      ptext->shadow_chars = NULL;
      mx = 0;
      my = 0;
    }
  ptext->kind = kind;
  ptext->w = w;
  ptext->h = h;

  for(y = 0; y < my ; ++y)
    {
      for(x = 0; x < mx ; ++x)
        {
          texture_load_part(&ptext->chars[y*mx+x],file,x*w,y*h,w,h, USE_ALPHA);
        }
    }

  /* Load shadow font. */
  for(y = 0; y < my ; ++y)
    {
      for(x = 0; x < mx ; ++x)
        {
	  int pixels;
          int i;
          conv = SDL_DisplayFormatAlpha(ptext->chars[y*mx+x].sdl_surface);
	  pixels = conv->w * conv->h;
	  SDL_LockSurface(conv);
          for(i = 0; i < pixels; ++i)
            {
              Uint32 *p = (Uint32 *)conv->pixels + i;
	      *p = *p & conv->format->Amask;
            }
	    SDL_UnlockSurface(conv);
	   SDL_SetAlpha(conv, SDL_SRCALPHA, 128);
          texture_from_sdl_surface(&ptext->shadow_chars[y*mx+x],conv,USE_ALPHA);
        }
    }

}

void text_draw(text_type* ptext, char* text, int x, int y, int shadowsize, int update)
{
  if(text != NULL)
    {
      if(shadowsize != 0)
        text_draw_chars(ptext,&ptext->shadow_chars[0], text,x+shadowsize,y+shadowsize, update);

      text_draw_chars(ptext,ptext->chars, text,x,y, update);
    }
}

void text_draw_chars(text_type* ptext, texture_type* pchars, char* text, int x, int y, int update)
{
  int i,len;

  len = strlen(text);

  if(ptext->kind == TEXT_TEXT)
    {
      for( i = 0; i < len; ++i)
        {
          if( text[i] >= 'A' && text[i] <= 'Z')
            {
              texture_draw(&pchars[(int)(text[i] - 'A')],x+i*ptext->w,y,update);
            }
          else if( text[i] >= 'a' && text[i] <= 'z')
            {
              texture_draw(&pchars[(int)(text[i] - 'a') + 26],x+i*ptext->w,y,update);
            }
          else if ( text[i] >= '!' && text[i] <= '9')
            {
              texture_draw(&pchars[(int)(text[i] - '!') + 52],x+i*ptext->w,y,update);
            }
          else if ( text[i] == '?')
            {
              texture_draw(&pchars[77],x+i*ptext->w,y,update);
            }
          else if ( text[i] == '\n')
            {
              y += ptext->h + 2;
            }
        }
    }
  else if(ptext->kind == TEXT_NUM)
    {
      for( i = 0; i < len; ++i)
        {
          if ( text[i] >= '0' && text[i] <= '9')
            {
              texture_draw(&pchars[(int)(text[i] - '0')],x+i*ptext->w,y,update);
            }
          else if ( text[i] == '\n')
            {
              y += ptext->h + 2;
            }
        }
    }
}

void text_drawf(text_type* ptext, char* text, int x, int y, int halign, int valign, int shadowsize, int update)
{
  if(text != NULL)
    {
      if(halign == A_RIGHT)
        x += screen->w;
      else if(halign == A_HMIDDLE)
        x += screen->w/2 - ((strlen(text)*ptext->w)/2);

      if(valign == A_BOTTOM)
        y += screen->h - ptext->h;
      else if(valign == A_VMIDDLE)
        y += screen->h/2 - ptext->h/2;

      text_draw(ptext,text,x,y,shadowsize,update);
    }
}

void text_free(text_type* ptext)
{
  int c;
  if(ptext->kind == TEXT_TEXT)
    {
      for( c = 0; c < 78; ++c)
        texture_free(&ptext->chars[c]);
    }
  else if(ptext->kind == TEXT_NUM)
    {
      for( c = 0; c < 10; ++c)
        texture_free(&ptext->chars[c]);
    }
}
