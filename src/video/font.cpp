//  $Id: font.cpp 2298 2005-03-30 12:01:02Z matzebraun $
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

#include "lisp/parser.h"
#include "lisp/lisp.h"
#include "screen.h"
#include "font.h"
#include "drawing_context.h"

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
Font::draw(const std::string& text, const Vector& pos_, FontAlignment alignment,
    uint32_t drawing_effect, uint8_t alpha) const
{
  /* Cut lines changes into seperate strings, needed to support center/right text
     alignments with break lines.
     Feel free to replace this hack with a more elegant solution
  */
  char temp[1024];
  std::string::size_type l, i, y;
  bool done = false;
  i = y = 0;

  while(!done) {
    l = text.find("\n", i);
    if(l == std::string::npos) {
      l = text.size();
      done = true;
    }
    
    temp[text.copy(temp, l - i, i)] = '\0';
    
    // calculate X positions based on the alignment type
    Vector pos = Vector(pos_);
    if(alignment == CENTER_ALLIGN)
      pos.x -= get_text_width(temp) / 2;
    else if(alignment == RIGHT_ALLIGN)
      pos.x -= get_text_width(temp);

    draw_text(temp, pos + Vector(0,y), drawing_effect, alpha);

    i = l+1;
    y += h + 2;
  }
}

void
Font::draw_text(const std::string& text, const Vector& pos, 
    uint32_t drawing_effect, uint8_t alpha) const
{
  if(shadowsize > 0)
    draw_chars(shadow_chars, text, pos + Vector(shadowsize, shadowsize),
               drawing_effect, alpha);

  draw_chars(chars, text, pos, drawing_effect, alpha);
}

void
Font::draw_chars(Surface* pchars, const std::string& text, const Vector& pos,
                 uint32_t drawing_effect, uint8_t alpha) const
{
  SurfaceImpl* impl = pchars->impl;

  Vector p = pos;
  for(size_t i = 0; i < text.size(); ++i) {
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
