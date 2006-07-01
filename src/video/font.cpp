//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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

std::string
Font::wrap_to_width(const std::string& s, int max_width, std::string* overflow) const
{
  return wrap_to_chars(s, max_width / w, overflow);
}

std::string
Font::wrap_to_chars(const std::string& s, int line_length, std::string* overflow)
{
  // if text is already smaller, return full text
  if ((int)s.length() <= line_length) {
    if (overflow) *overflow = "";
    return s;
  }

  // if we can find a whitespace character to break at, return text up to this character
  int i = line_length;
  while ((i > 0) && (s[i] != ' ')) i--;
  if (i > 0) {
    if (overflow) *overflow = s.substr(i+1);
    return s.substr(0, i);
  }

  // FIXME: wrap at line_length, taking care of multibyte characters
  if (overflow) *overflow = "";
  return s;
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

namespace {

/**
 * returns true if this byte matches a bitmask of 10xx.xxxx, i.e. it is the 2nd, 3rd or 4th byte of a multibyte utf8 string
 */
bool has_multibyte_mark(unsigned char c) {
  return ((c & 0300) == 0200);
}

/** 
 * gets unicode character at byte position @a p of UTF-8 encoded @a text, then advances @a p to the next character.
 * @throws std::runtime_error if decoding fails.
 * See unicode standard section 3.10 table 3-5 and 3-6 for details.
 */
uint32_t decode_utf8(const std::string& text, size_t& p)
{
  uint32_t c1 = (unsigned char) text[p+0];

  if (has_multibyte_mark(c1)) std::runtime_error("Malformed utf-8 sequence");

  if ((c1 & 0200) == 0000) {
    // 0xxx.xxxx: 1 byte sequence
    p+=1;
    return c1;
  } 
  else if ((c1 & 0340) == 0300) {
    // 110x.xxxx: 2 byte sequence
    if(p+1 >= text.size()) throw std::range_error("Malformed utf-8 sequence");
    uint32_t c2 = (unsigned char) text[p+1];
    if (!has_multibyte_mark(c2)) throw std::runtime_error("Malformed utf-8 sequence");
    p+=2;
    return (c1 & 0037) << 6 | (c2 & 0077);
  }
  else if ((c1 & 0360) == 0340) {
    // 1110.xxxx: 3 byte sequence
    if(p+2 >= text.size()) throw std::range_error("Malformed utf-8 sequence");
    uint32_t c2 = (unsigned char) text[p+1];
    uint32_t c3 = (unsigned char) text[p+2];
    if (!has_multibyte_mark(c2)) throw std::runtime_error("Malformed utf-8 sequence");
    if (!has_multibyte_mark(c3)) throw std::runtime_error("Malformed utf-8 sequence");
    p+=3;
    return (c1 & 0017) << 12 | (c2 & 0077) << 6 | (c3 & 0077);
  }
  else if ((c1 & 0370) == 0360) {
    // 1111.0xxx: 4 byte sequence
    if(p+3 >= text.size()) throw std::range_error("Malformed utf-8 sequence");
    uint32_t c2 = (unsigned char) text[p+1];
    uint32_t c3 = (unsigned char) text[p+2];
    uint32_t c4 = (unsigned char) text[p+4];
    if (!has_multibyte_mark(c2)) throw std::runtime_error("Malformed utf-8 sequence");
    if (!has_multibyte_mark(c3)) throw std::runtime_error("Malformed utf-8 sequence");
    if (!has_multibyte_mark(c4)) throw std::runtime_error("Malformed utf-8 sequence");
    p+=4;
    return (c1 & 0007) << 18 | (c2 & 0077) << 12 | (c3 & 0077) << 6 | (c4 & 0077);
  }
  throw std::runtime_error("Malformed utf-8 sequence");
}

}

void
Font::draw_chars(Surface* pchars, const std::string& text, const Vector& pos,
                 DrawingEffect drawing_effect, float alpha) const
{
  Vector p = pos;
  size_t i = 0;
  while(i < text.size()) {
    uint32_t c;
    try {
     c = decode_utf8(text, i);
    } 
    catch (std::runtime_error) {
     log_debug << "Malformed utf-8 sequence beginning with " << *((uint32_t*)(text.c_str() + i)) << " found " << std::endl;
     c = 0;
     i++;
    }
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
