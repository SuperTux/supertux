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

#include <config.h>

#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include "app/globals.h"
#include "lisp/parser.h"
#include "lisp/lisp.h"
#include "screen.h"
#include "font.h"
#include "drawing_context.h"

using namespace SuperTux;

Font::Font(const std::string& file, FontType ntype, int nw, int nh,
        int nshadowsize)
    : chars(0), shadow_chars(0), type(ntype), w(nw), h(nh),
      shadowsize(nshadowsize)
{
  chars = new Surface(file, true);
 
  switch(type) {
    case TEXT:
      first_char = 32;
      break;
    case NUM:
      first_char = 48;
      break;
  }
  last_char = first_char + (chars->h / h) * 16;
  if(last_char > 127) // we have left out some control chars at 128-159
    last_char += 32;
   
  // Load shadow font.
  if(shadowsize > 0) {
    SDL_Surface* conv = SDL_DisplayFormatAlpha(chars->impl->get_sdl_surface());
    int pixels = conv->w * conv->h;
    SDL_LockSurface(conv);
    for(int i = 0; i < pixels; ++i) {
      Uint32 *p = (Uint32 *)conv->pixels + i;
      *p = *p & conv->format->Amask;
    }
    SDL_UnlockSurface(conv);
    SDL_SetAlpha(conv, SDL_SRCALPHA, 128);
    shadow_chars = new Surface(conv, true);
    SDL_FreeSurface(conv);
  }
}

Font::~Font()
{
  delete chars;
  delete shadow_chars;
}

float
Font::get_text_width(const std::string& text) const
{
  /** Let's calculate the size of the biggest paragraph */
  std::string::size_type l, hl, ol;
  hl = 0; l = 0;
  while(true)
    {
    ol = l;
    l = text.find("\n", l+1);
    if(l == std::string::npos)
      break;
    if(hl < l-ol)
      hl = l-ol;
    }
  if(hl == 0)
    hl = text.size();

  return hl * w;
}

float
Font::get_text_height(const std::string& text) const
{
  /** Let's calculate height of the text */
  std::string::size_type l, hh;
  hh = h; l = 0;
  while(true)
    {
    l = text.find("\n", l+1);
    if(l == std::string::npos)
      break;
    hh += h + 2;
    }

  return hh;
}

float
Font::get_height() const
{
  return h;
}

void
Font::draw(const std::string& text, const Vector& pos_, int allignment, Uint32 drawing_effect, int alpha)
{
  /* Cut lines changes into seperate strings, needed to support center/right text
     allignments with break lines.
     Feel free to replace this hack with a more elegant solution
  */
  char temp[1024];
  std::string::size_type l, i, y;
  bool done = false;
  i = y = 0;

  while(!done)
    {
    l = text.find("\n", i);
    if(l == std::string::npos)
      {
      l = text.size();
      done = true;
      }

    temp[text.copy(temp, l - i, i)] = '\0';

    // calculate X positions based on the allignment type
    Vector pos = Vector(pos_);
    if(allignment == CENTER_ALLIGN)
      pos.x -= get_text_width(temp) / 2;
    else if(allignment == RIGHT_ALLIGN)
      pos.x -= get_text_width(temp);

    draw_text(temp, pos + Vector(0,y), drawing_effect, alpha);

    i = l+1;
    y += h + 2;
    }
}

void
Font::draw_text(const std::string& text, const Vector& pos, Uint32 drawing_effect, int alpha)
{
  if(shadowsize > 0)
    draw_chars(shadow_chars, text, pos + Vector(shadowsize, shadowsize),
               drawing_effect, alpha);

  draw_chars(chars, text, pos, drawing_effect, alpha);
}

void
Font::draw_chars(Surface* pchars, const std::string& text, const Vector& pos,
                 Uint32 drawing_effect, int alpha)
{
  SurfaceImpl* impl = pchars->impl;

  Vector p = pos;
  for(size_t i = 0; i < text.size(); ++i)
  {
    int c = (unsigned char) text[i];
    if(c > 127) // correct for the 32 controlchars at 128-159
      c -= 32;
    // a non-printable character?
    if(c == '\n') {
      p.x = pos.x;
      p.y += h + 2;
      continue;
    }
    if(c == ' ' || c < first_char || c > last_char) {
      p.x += w;
      continue;
    }
    
    int index = c - first_char;
    int source_x = (index % 16) * w;
    int source_y = (index / 16) * h;

    impl->draw_part(source_x, source_y, p.x, p.y, w, h, alpha, drawing_effect);
    p.x += w;
  }
}

/* --- SCROLL TEXT FUNCTION --- */

#define MAX_VEL     10
#define SPEED_INC   0.01
#define SCROLL      60
#define ITEMS_SPACE 4

void SuperTux::display_text_file(const std::string& file, float scroll_speed,
    Font* heading_font, Font* normal_font, Font* small_font,
    Font* reference_font)
{
  std::string text;
  std::string background_file;
  std::vector<std::string> names;

  std::string filename = datadir + "/" + file;
  lisp::Parser parser;
  try {
    std::auto_ptr<lisp::Lisp> root (parser.parse(filename));

    const lisp::Lisp* text_lisp = root->get_lisp("supertux-text");
    if(!text_lisp)
      throw std::runtime_error("File isn't a supertux-text file");
    
    if(!text_lisp->get("text", text))
      throw std::runtime_error("file doesn't contain a text field");
  } catch(std::exception& e) {
    std::cerr << "Couldn't load file '" << filename << "': " << e.what() <<
      "\n";
    return;
  }

  // Split text string lines into a vector
  names.clear();
  std::string::size_type i, l;
  i = 0;
  while(true)
    {
    l = text.find("\n", i);

    if(l == std::string::npos)
      {
      char temp[1024];
      temp[text.copy(temp, text.size() - i, i)] = '\0';
      names.push_back(temp);
      break;
      }

    char temp[1024];
    temp[text.copy(temp, l-i, i)] = '\0';
    names.push_back(temp);

    i = l+1;
    }

  // load background image
  Surface* background = new Surface(datadir + "/images/background/" + background_file, false);

  int done = 0;
  float scroll = 0;
  float speed = scroll_speed / 50;

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
      context.draw_surface(background, Vector(0,0), 0);

      float y = 0;
      for(size_t i = 0; i < names.size(); i++) {
        if(names[i].size() == 0) {
          y += normal_font->get_height() + ITEMS_SPACE;
          continue;
        }

        Font* font = 0;
        switch(names[i][0])
        {
          case ' ': font = small_font; break;
          case '\t': font = normal_font; break;
          case '-': font = heading_font; break;
          case '*': font = reference_font; break;
          default: font = reference_font; break;
        }

        context.draw_text(font,
            names[i].substr(1, names[i].size()-1),
            Vector(screen->w/2, screen->h + y - scroll), CENTER_ALLIGN, LAYER_FOREGROUND1);
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
  delete background;
}

