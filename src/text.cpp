//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "defines.h"
#include "screen.h"
#include "text.h"

Text::Text(const std::string& file, int kind_, int w_, int h_)
{
  kind = kind_;
  w = w_;
  h = h_;

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

  chars = new Surface(file, USE_ALPHA);

  // Load shadow font.
  conv = SDL_DisplayFormatAlpha(chars->impl->sdl_surface);
  pixels = conv->w * conv->h;
  SDL_LockSurface(conv);
  for(i = 0; i < pixels; ++i)
    {
      Uint32 *p = (Uint32 *)conv->pixels + i;
      *p = *p & conv->format->Amask;
    }
  SDL_UnlockSurface(conv);
  SDL_SetAlpha(conv, SDL_SRCALPHA, 128);
  shadow_chars = new Surface(conv, USE_ALPHA);

  SDL_FreeSurface(conv);
}

void
Text::draw(const  char* text, int x, int y, int shadowsize, int update)
{
  if(text != NULL)
    {
      if(shadowsize != 0)
        draw_chars(shadow_chars, text,x+shadowsize,y+shadowsize, update);

      draw_chars(chars, text,x,y, update);
    }
}

void
Text::draw_chars(Surface* pchars,const  char* text, int x, int y, int update)
{
  int i,j,len;

  len = strlen(text);
  int w = this->w;
  int h = this->h;

  if(kind == TEXT_TEXT)
    {
      for( i = 0, j = 0; i < len; ++i,++j)
        {
          if( text[i] >= 'A' && text[i] <= 'Z')
            pchars->draw_part((int)(text[i] - 'A')*w, 0, x+(j*w), y, w, h, 255,  update);
          else if( text[i] >= 'a' && text[i] <= 'z')
            pchars->draw_part((int)(text[i] - 'a')*w, h, x+(j*w), y, w, h, 255,  update);
          else if ( text[i] >= '!' && text[i] <= '9')
            pchars->draw_part((int)(text[i] - '!')*w, h*2, x+(j*w), y, w, h, 255,  update);
          else if ( text[i] == '?')
            pchars->draw_part(25*w, h*2, x+(j*w), y, w, h, 255,  update);
          else if ( text[i] == '\n')
            {
              y += h + 2;
              j = 0;
            }
        }
    }
  else if(kind == TEXT_NUM)
    {
      for( i = 0, j = 0; i < len; ++i, ++j)
        {
          if ( text[i] >= '0' && text[i] <= '9')
            pchars->draw_part((int)(text[i] - '0')*w, 0, x+(j*w), y, w, h, 255, update);
          else if ( text[i] == '\n')
            {
              y += h + 2;
              j = 0;
            }
        }
    }
}

void
Text::draw_align(const char* text, int x, int y,
                      TextHAlign halign, TextVAlign valign, int shadowsize, int update)
{
  if(text != NULL)
    {
      switch (halign)
        {
        case A_RIGHT:
          x += -(strlen(text)*w);
          break;
        case A_HMIDDLE:
          x += -((strlen(text)*w)/2);
          break;
        case A_LEFT:
          // default
          break;
        }

      switch (valign)
        {
        case A_BOTTOM:
          y -= h;
          break;
          
        case A_VMIDDLE:
          y -= h/2;

        case A_TOP:
          // default
          break;
        }

      draw(text, x, y, shadowsize, update);
    }
}

void
Text::drawf(const  char* text, int x, int y,
                 TextHAlign halign, TextVAlign valign, int shadowsize, int update)
{
  if(text != NULL)
    {
      if(halign == A_RIGHT)  /* FIXME: this doesn't work correctly for strings with newlines.*/
        x += screen->w - (strlen(text)*w);
      else if(halign == A_HMIDDLE)
        x += screen->w/2 - ((strlen(text)*w)/2);

      if(valign == A_BOTTOM)
        y += screen->h - h;
      else if(valign == A_VMIDDLE)
        y += screen->h/2 - h/2;

      draw(text,x,y,shadowsize, update);
    }
}

Text::~Text()
{
  if(kind == TEXT_TEXT)
    delete chars;
  else if(kind == TEXT_NUM)
    delete chars;
}

/* --- ERASE TEXT: --- */

void
Text::erasetext(const  char * text, int x, int y, Surface * ptexture, int update, int shadowsize)
{
  SDL_Rect dest;

  dest.x = x;
  dest.y = y;
  dest.w = strlen(text) * w + shadowsize;
  dest.h = h;

  if (dest.w > screen->w)
    dest.w = screen->w;

  ptexture->draw_part(dest.x,dest.y,dest.x,dest.y,dest.w,dest.h, 255, update);

  if (update == UPDATE)
    update_rect(screen, dest.x, dest.y, dest.w, dest.h);
}


/* --- ERASE CENTERED TEXT: --- */

void
Text::erasecenteredtext(const  char * text, int y, Surface * ptexture, int update, int shadowsize)
{
  erasetext(text, screen->w / 2 - (strlen(text) * 8), y, ptexture, update, shadowsize);
}
