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

#include <SDL_image.h>
#include "physfs/physfs_sdl.hpp"

#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "screen.hpp"
#include "font.hpp"
#include "drawing_context.hpp"
#include "log.hpp"

namespace {
bool     has_multibyte_mark(unsigned char c);
uint32_t decode_utf8(const std::string& text, size_t& p);

bool vline_empty(SDL_Surface* surface, int x, int start_y, int end_y, Uint8 threshold)
{
  Uint8* pixels = (Uint8*)surface->pixels;

  for(int y = start_y; y < end_y; ++y)
    {
      const Uint8& p = pixels[surface->pitch*y + x*surface->format->BytesPerPixel + 3];
      if (p > threshold)
        {
          return false;
        }
    }
  return true;
}
} // namespace

Font::Font(const std::string& file, const std::string& shadowfile,
           int w, int h, int shadowsize)
  : glyph_surface(0), shadow_chars(0),
    char_width(w), char_height(h), 
    shadowsize(shadowsize)
{
  glyph_surface = new Surface(file);
  shadow_chars  = new Surface(shadowfile);

  first_char = 32;
  char_count = ((int) glyph_surface->get_height() / char_height) * 16;

  for(uint32_t i = 0; i < char_count; ++i)
    {
      float x = (i % 16) * char_width;
      float y = (i / 16) * char_height;
      glyphs.push_back(Rect(x, y,
                            x + char_width, y + char_height));
    }
}

Font::Font(const std::string& filename, int char_width_, int char_height_)
  : glyph_surface(0), shadow_chars(0), 
    char_width(char_width_), char_height(char_height_), 
    shadowsize(0)
{
  glyph_surface = new Surface(filename);

  first_char = 32;
  char_count = ((int) glyph_surface->get_height() / char_height) * 16;

  // Load the surface into RAM and scan the pixel data for characters
  SDL_Surface* surface = IMG_Load_RW(get_physfs_SDLRWops(filename), 1);
  if(surface == NULL) {
    std::ostringstream msg;
    msg << "Couldn't load image '" << filename << "' :" << SDL_GetError();
    throw std::runtime_error(msg.str());
  }

  SDL_LockSurface(surface);

  for(uint32_t i = 0; i < char_count; ++i)
    {
      int x = (i % 16) * char_width;
      int y = (i / 16) * char_height;

      int left = x;
      while (left < x + char_width &&
             vline_empty(surface, left, y, y + char_height, 0))
        left += 1;

      int right = x + char_width - 1;
      while (right > left && 
             vline_empty(surface, right, y, y + char_height, 0))
        right -= 1;

      if (left <= right)
        glyphs.push_back(Rect(left,  y,
                              right+1, y + char_height));
      else // glyph is completly transparent
        glyphs.push_back(Rect(x,  y,
                              x + char_width, y + char_height));

    }
  
  SDL_UnlockSurface(surface);

  SDL_FreeSurface(surface);
}

Font::~Font()
{
  delete glyph_surface;
  delete shadow_chars;
}

float
Font::get_text_width(const std::string& text) const
{
  float curr_width = 0;
  float last_width = 0;

 // FIXME: add UTF8 decode here
  for(std::string::size_type i = 0; i < text.size(); ++i)
    {
      uint32_t chr = text[i];
      if (chr == '\n')
        {
          last_width = std::max(last_width, curr_width);
          curr_width = 0;
        }
      else
        {
          curr_width += glyphs[static_cast<unsigned char>(text[i])].get_width() + 1;
        }
    }

  return std::max(curr_width, last_width);
}

float
Font::get_text_height(const std::string& text) const
{
  std::string::size_type text_height = char_height;
  
  for(std::string::size_type i = 0; i < text.size(); ++i)
    {
      if (i == '\n')
        text_height += char_height + 2;
    }

  return text_height;
}

float
Font::get_height() const
{
  return char_height; 
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
    y += char_height + 2;
  }
}

void
Font::draw_text(const std::string& text, const Vector& pos,
                DrawingEffect drawing_effect, float alpha) const
{
  if(shadowsize > 0)
    draw_chars(shadow_chars, text, pos + Vector(shadowsize, shadowsize),
               drawing_effect, alpha);

  draw_chars(glyph_surface, text, pos, drawing_effect, alpha);
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
      c = decode_utf8(text, i); // FIXME: this seems wrong, since when incrementing i by 
    }
    catch (std::runtime_error) {
     log_debug << "Malformed utf-8 sequence beginning with " << *((uint32_t*)(text.c_str() + i)) << " found " << std::endl;
     c = 0;
     i++;
    }

    int font_index = c - first_char;

    // a non-printable character?
    if(c == '\n') {
      p.x = pos.x;
      p.y += char_height + 2;
      continue;
    }
    if(c == ' ') {
      p.x += glyphs[font_index].get_width();
      continue;
    }

    // we don't have the control chars 0x80-0xa0 in the font
    if (c >= 0x80) {
      font_index -= 32;
      if(c <= 0xa0) {
        log_debug << "Unsupported utf-8 character '" << c << "' found" << std::endl;
        font_index = 0;
      }
    }

    if(font_index < 0 || font_index >= (int) char_count) {
      log_debug << "Unsupported utf-8 character found" << std::endl;
      font_index = 0;
    }

    const Rect& glyph = glyphs[font_index];
    pchars->draw_part(glyph.get_left(), glyph.get_top(),
                      p.x, p.y,
                      glyph.get_width(), glyph.get_height(),
                      alpha, drawing_effect);
    p.x += glyphs[font_index].get_width();
  }
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

} // namespace
