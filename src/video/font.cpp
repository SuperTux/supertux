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

#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "screen.hpp"
#include "font.hpp"
#include "drawing_context.hpp"
#include "log.hpp"

Font::Font(const std::string& file, const std::string& shadowfile,
           int w, int h, int shadowsize)
    : chars(0), shadow_chars(0), w(w), h(h), shadowsize(shadowsize)
{
  chars = new Surface(file);
  shadow_chars = new Surface(shadowfile);
 
  first_char = 32;
  char_count = ((int) chars->get_height() / h) * 16;
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

  for (unsigned int i = 0; i < text.size(); i++)
    if ((unsigned char) text[i] > 0xC2 && (unsigned char) text[i] < 0xC6)
      hl--;  // control characters are a WASTE.

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
           DrawingEffect drawing_effect, float alpha) const
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

    if(l > sizeof(temp)-1)
      l = sizeof(temp)-1;
    
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
                DrawingEffect drawing_effect, float alpha) const
{
  if(shadowsize > 0)
    draw_chars(shadow_chars, text, pos + Vector(shadowsize, shadowsize),
               drawing_effect, alpha);

  draw_chars(chars, text, pos, drawing_effect, alpha);
}

/** decoding of a byte stream to a single unicode character.
 * This should be correct for well formed utf-8 sequences but doesn't check for
 * all forms of illegal sequences.
 * (see unicode standard section 3.10 table 3-5 and 3-6 for details)
 */
uint32_t decode_utf8(const std::string& text, size_t& p)
{
  // 1 byte sequence
  uint32_t c = (unsigned char) text[p++];
  if(c <= 0x7F) {
    return c;
  }
  
  // 2 byte sequence
  if(p >= text.size())
    throw std::runtime_error("Malformed utf-8 sequence");
  uint32_t c2 = (unsigned char) text[p++];
  if(c <= 0xDF) {
    if(c < 0xC2)
      throw std::runtime_error("Malformed utf-8 sequence");
    return (c & 0x1F) << 6 | (c2 & 0x3F);
  }
  
  // 3 byte sequence
  if(p >= text.size())
    throw std::runtime_error("Malformed utf-8 sequence");
  uint32_t c3 = (unsigned char) text[p++];
  if(c <= 0xEF) {
    return (c & 0x0F) << 12 | (c2 & 0x3F) << 6 | (c3 & 0x3F);
  }
  
  // 4 byte sequence
  if(p >= text.size())
    throw std::runtime_error("Malformed utf-8 sequence");
  uint32_t c4 = (unsigned char) text[p++];
  if(c <= 0xF4) {
    return (c & 0x07) << 18 | (c2 & 0x3F) << 12 | (c3 & 0x3F) << 6 
      | (c4 & 0x3F);
  }

  throw std::runtime_error("Malformed utf-8 sequence");
}

void
Font::draw_chars(Surface* pchars, const std::string& text, const Vector& pos,
                 DrawingEffect drawing_effect, float alpha) const
{
  Vector p = pos;
  size_t i = 0;
  while(i < text.size()) {
    uint32_t c = decode_utf8(text, i);
    ssize_t font_index;

    // a non-printable character?
    if(c == '\n') {                                      
      p.x = pos.x;
      p.y += h + 2;
      continue;
    }
    if(c == ' ') {
      p.x += w;
      continue;
    }

    font_index = c - first_char;
    // we don't have the control chars 0x80-0xa0 in the font
    if(c >= 0x80) {
      font_index -= 32;
      if(c <= 0xa0) {
        log_debug << "Unsupported utf-8 character '" << c << "' found" << std::endl;
        font_index = 0;
      }
    }
        
    if(font_index < 0 || font_index >= (ssize_t) char_count) {
      log_debug << "Unsupported utf-8 character found" << std::endl;
      font_index = 0;
    }                   

    int source_x = (font_index % 16) * w;
    int source_y = (font_index / 16) * h;
    pchars->draw_part(source_x, source_y, p.x, p.y, w, h, alpha,
                      drawing_effect);
    p.x += w;
  }
}
