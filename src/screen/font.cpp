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
#include "font.h"
#include "drawing_context.h"

Font::Font(const std::string& file, int kind_, int w_, int h_, int shadowsize_)
{
  kind = kind_;
  w = w_;
  h = h_;
  shadowsize = shadowsize_;

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
  conv = SDL_DisplayFormatAlpha(chars->impl->get_sdl_surface());
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

Font::~Font()
{
  delete chars;
  delete shadow_chars;
}

float
Font::get_height() const
{
  return h;
}

float
Font::get_text_width(const std::string& text) const
{
  return text.size() * w;
}

void
Font::draw(const std::string& text, const Vector& pos)
{
  if(shadowsize != 0)
    draw_chars(shadow_chars, text, pos + Vector(shadowsize, shadowsize));

  draw_chars(chars, text, pos);
}

void
Font::draw_chars(Surface* pchars, const std::string& text, const Vector& pos)
{
  size_t i, j;

  SurfaceImpl* impl = pchars->impl;

  int x = int(pos.x);
  int y = int(pos.y);
  if(kind == TEXT_TEXT)
    {
      for( i = 0, j = 0; i < text.size(); ++i,++j)
        {
          if( text[i] >= ' ' && text[i] <= '/')
            impl->draw_part((int)(text[i] - ' ')*w,  0 , x+(j*w), y, w, h, 255);
          else if( text[i] >= '0' && text[i] <= '?')
            impl->draw_part((int)(text[i] - '0')*w, h*1, x+(j*w), y, w, h, 255);
          else if ( text[i] >= '@' && text[i] <= 'O')
            impl->draw_part((int)(text[i] - '@')*w, h*2, x+(j*w), y, w, h, 255);
          else if ( text[i] >= 'P' && text[i] <= '_')
            impl->draw_part((int)(text[i] - 'P')*w, h*3, x+(j*w), y, w, h, 255);
          else if ( text[i] >= '`' && text[i] <= 'o')
            impl->draw_part((int)(text[i] - '`')*w, h*4, x+(j*w), y, w, h, 255);
          else if ( text[i] >= 'p' && text[i] <= '~')
            impl->draw_part((int)(text[i] - 'p')*w, h*5, x+(j*w), y, w, h, 255);
          else if ( text[i] == '\n')
            {
              y += h + 2;
              j = 0;
            }
        }
    }
  else if(kind == TEXT_NUM)
    {
      for( i = 0, j = 0; i < text.size(); ++i, ++j)
        {
          if ( text[i] >= '0' && text[i] <= '9')
            impl->draw_part((int)(text[i] - '0')*w, 0, x+(j*w), y, w, h, 255);
          else if ( text[i] == '\n')
            {
              y += h + 2;
              j = 0;
            }
        }
    }
}

/* --- SCROLL TEXT FUNCTION --- */

#define MAX_VEL     10
#define SPEED_INC   0.01
#define SCROLL      60
#define ITEMS_SPACE 4

void display_text_file(const std::string& file, const std::string& surface, float scroll_speed)
{
  Surface* sur = new Surface(datadir + surface, IGNORE_ALPHA);
  display_text_file(file, sur, scroll_speed);
  delete sur;
}

void display_text_file(const std::string& file, Surface* surface, float scroll_speed)
{
  int done;
  float scroll;
  float speed;
  FILE* fi;
  char temp[1024];
  std::vector<std::string> names;
  char filename[1024];
  
  sprintf(filename,"%s/%s", datadir.c_str(), file.c_str());
  if((fi = fopen(filename,"r")) != NULL)
    {
      while(fgets(temp, sizeof(temp), fi) != NULL)
        {
          temp[strlen(temp)-1]='\0';
          names.push_back(temp);
        }
      fclose(fi);
    }
  else
    {
      names.push_back("File was not found!");
      names.push_back(filename);
      names.push_back("Shame on the guy, who");
      names.push_back("forgot to include it");
      names.push_back("in your SuperTux distribution.");
    }

  scroll = 0;
  speed = scroll_speed / 50;
  done = 0;

  DrawingContext context;
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  Uint32 lastticks = SDL_GetTicks();
  while(!done)
    {
      /* in case of input, exit */
      SDL_Event event;
      while(SDL_PollEvent(&event))
        switch(event.type)
          {
          case SDL_KEYDOWN:
            switch(event.key.keysym.sym)
              {
              case SDLK_UP:
                speed -= SPEED_INC;
                break;
              case SDLK_DOWN:
                speed += SPEED_INC;
                break;
              case SDLK_SPACE:
              case SDLK_RETURN:
                if(speed >= 0)
                  scroll += SCROLL;
                break;
              case SDLK_ESCAPE:
                done = 1;
                break;
              default:
                break;
              }
            break;
          case SDL_QUIT:
            done = 1;
            break;
          default:
            break;
          }

      if(speed > MAX_VEL)
        speed = MAX_VEL;
      else if(speed < -MAX_VEL)
        speed = -MAX_VEL;

      /* draw the credits */
      context.draw_surface(surface, Vector(0,0), 0);

      float y = 0;
      for(size_t i = 0; i < names.size(); i++) {
        if(names[i].size() == 0) {
          y += white_text->get_height() + ITEMS_SPACE;
          continue;
        }

        Font* font = 0;
        switch(names[i][0])
        {
          case ' ': font = white_small_text; break;
          case '\t': font = white_text; break;
          case '-': font = white_big_text; break;
          case '*': font = blue_text; break;
          default: font = blue_text; break;
        }

        context.draw_text_center(font,
            names[i].substr(1, names[i].size()-1),
            Vector(0, screen->h + y - scroll), LAYER_FOREGROUND1);
        y += font->get_height() + ITEMS_SPACE;
      }

      context.do_drawing();

      if(screen->h+y-scroll < 0 && 20+screen->h+y-scroll < 0)
        done = 1;

      Uint32 ticks = SDL_GetTicks();
      scroll += speed * (ticks - lastticks);
      lastticks = ticks;
      if(scroll < 0)
        scroll = 0;

      SDL_Delay(10);
    }

  SDL_EnableKeyRepeat(0, 0);    // disables key repeating
  Menu::set_current(main_menu);
}

