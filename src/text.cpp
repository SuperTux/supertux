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

void text_load(text_type* ptext, const std::string& file, int kind, int w, int h)
{
  int mx, my;
  SDL_Surface *conv;
  int pixels;
  int i;
  
  if(kind == TEXT_TEXT)
    {
      mx = 26;
      my = 3;
    }
  else if(kind == TEXT_NUM)
    {
      mx = 10;
      my = 1;
    }
  else
    {
      mx = 0;
      my = 0;
    }
  ptext->kind = kind;
  ptext->w = w;
  ptext->h = h;

  ptext->chars = new Surface(file, USE_ALPHA);

  /* Load shadow font. */
  conv = SDL_DisplayFormatAlpha(ptext->chars->impl->sdl_surface);
  pixels = conv->w * conv->h;
  SDL_LockSurface(conv);
  for(i = 0; i < pixels; ++i)
    {
      Uint32 *p = (Uint32 *)conv->pixels + i;
      *p = *p & conv->format->Amask;
    }
  SDL_UnlockSurface(conv);
  SDL_SetAlpha(conv, SDL_SRCALPHA, 128);
  ptext->shadow_chars = new Surface(conv, USE_ALPHA);

  SDL_FreeSurface(conv);
}

void text_draw(text_type* ptext,const  char* text, int x, int y, int shadowsize, int update)
{
  if(text != NULL)
    {
      if(shadowsize != 0)
        text_draw_chars(ptext, ptext->shadow_chars, text,x+shadowsize,y+shadowsize, update);

      text_draw_chars(ptext, ptext->chars, text,x,y, update);
    }
}

void text_draw_chars(text_type* ptext, Surface* pchars,const  char* text, int x, int y, int update)
{
  int i,j,len;
  int w, h;

  len = strlen(text);
  w = ptext->w;
  h = ptext->h;

  if(ptext->kind == TEXT_TEXT)
    {
      for( i = 0, j = 0; i < len; ++i,++j)
        {
          if( text[i] >= 'A' && text[i] <= 'Z')
            pchars->draw_part((int)(text[i] - 'A')*w, 0, x+(j*w), y, ptext->w, ptext->h, 255,  update);
          else if( text[i] >= 'a' && text[i] <= 'z')
            pchars->draw_part((int)(text[i] - 'a')*w, h, x+(j*w), y, ptext->w, ptext->h, 255,  update);
          else if ( text[i] >= '!' && text[i] <= '9')
            pchars->draw_part((int)(text[i] - '!')*w, h*2, x+(j*w), y, ptext->w, ptext->h, 255,  update);
          else if ( text[i] == '?')
            pchars->draw_part(25*w, h*2, x+(j*w), y, ptext->w, ptext->h, 255,  update);
          else if ( text[i] == '\n')
            {
              y += ptext->h + 2;
              j = 0;
            }
        }
    }
  else if(ptext->kind == TEXT_NUM)
    {
      for( i = 0, j = 0; i < len; ++i, ++j)
        {
          if ( text[i] >= '0' && text[i] <= '9')
            pchars->draw_part((int)(text[i] - '0')*w, 0, x+(j*w), y, w, h, 255, update);
          else if ( text[i] == '\n')
            {
              y += ptext->h + 2;
              j = 0;
            }
        }
    }
}

void text_draw_align(text_type* ptext, const char* text, int x, int y,
                     TextHAlign halign, TextVAlign valign, int shadowsize, int update)
{
  if(text != NULL)
    {
      switch (halign)
        {
        case A_RIGHT:
          x += -(strlen(text)*ptext->w);
          break;
        case A_HMIDDLE:
          x += -((strlen(text)*ptext->w)/2);
          break;
        case A_LEFT:
          // default
          break;
        }

      switch (valign)
        {
        case A_BOTTOM:
          y -= ptext->h;
          break;
          
        case A_VMIDDLE:
          y -= ptext->h/2;

        case A_TOP:
          // default
          break;
        }

      text_draw(ptext, text, x, y, shadowsize, update);
    }
}

void text_drawf(text_type* ptext,const  char* text, int x, int y, TextHAlign halign, TextVAlign valign, int shadowsize, int update)
{
  if(text != NULL)
    {
      if(halign == A_RIGHT)  /* FIXME: this doesn't work correctly for strings with newlines.*/
        x += screen->w - (strlen(text)*ptext->w);
      else if(halign == A_HMIDDLE)
        x += screen->w/2 - ((strlen(text)*ptext->w)/2);

      if(valign == A_BOTTOM)
        y += screen->h - ptext->h;
      else if(valign == A_VMIDDLE)
        y += screen->h/2 - ptext->h/2;

      text_draw(ptext,text,x,y,shadowsize, update);
    }
}

void text_free(text_type* ptext)
{
  if(ptext->kind == TEXT_TEXT)
    delete ptext->chars;
  else if(ptext->kind == TEXT_NUM)
    delete ptext->chars;
}

/* --- ERASE TEXT: --- */

void erasetext(text_type* ptext,const  char * text, int x, int y, Surface * ptexture, int update, int shadowsize)
{
  SDL_Rect dest;


  dest.x = x;
  dest.y = y;
  dest.w = strlen(text) * ptext->w + shadowsize;
  dest.h = ptext->h;

  if (dest.w > screen->w)
    dest.w = screen->w;

  ptexture->draw_part(dest.x,dest.y,dest.x,dest.y,dest.w,dest.h, 255, update);

  if (update == UPDATE)
    update_rect(screen, dest.x, dest.y, dest.w, dest.h);
}


/* --- ERASE CENTERED TEXT: --- */

void erasecenteredtext(text_type* ptext,const  char * text, int y, Surface * ptexture, int update, int shadowsize)
{
  erasetext(ptext, text, screen->w / 2 - (strlen(text) * 8), y, ptexture, update, shadowsize);
}
