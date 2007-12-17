//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                     Ingo Ruhnke <grumbel@gmx.de>
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

//#include "physfs/physfs_sdl.hpp"

#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "screen.hpp"
#include "font.hpp"
//#include "renderer.hpp"
#include <unison/video/Blittable.hpp>
#include "drawing_context.hpp"
#include "log.hpp"

namespace {
bool     has_multibyte_mark(unsigned char c);
uint32_t decode_utf8(const std::string& text, size_t& p);

struct UTF8Iterator
{
  const std::string&     text;
  std::string::size_type pos;
  uint32_t chr;

  UTF8Iterator(const std::string& text_)
    : text(text_),
      pos(0)
  {
    chr = decode_utf8(text, pos);
  }

  bool done() const
  {
    return pos > text.size();
  }

  UTF8Iterator& operator++() {
    try {
      chr = decode_utf8(text, pos);
    } catch (std::runtime_error) {
      log_debug << "Malformed utf-8 sequence beginning with " << *((uint32_t*)(text.c_str() + pos)) << " found " << std::endl;
      chr = 0;
      ++pos;
    }

    return *this;
  }

  uint32_t operator*() const {
    return chr;
  }
};

bool vline_empty(const Unison::Video::Surface &surface, int x, int start_y, int end_y, unsigned char threshold)
{
  for(int y = start_y; y < end_y; ++y)
  {
    if (surface.get_pixel(x, y).alpha > threshold)
    {
      return false;
    }
  }
  return true;
}
} // namespace

Font::Font(GlyphWidth glyph_width_,
           const std::string& filename,
           const std::string& shadowfile,
           int char_width, int char_height_,
           int shadowsize_)
  : glyph_width(glyph_width_),
    glyph_surface(filename), shadow_glyph_surface(shadowfile),
    char_height(char_height_),
    shadowsize(shadowsize_)
{
  //glyph_surface = new Surface(filename);
  //shadow_glyph_surface  = new Surface(shadowfile);

  first_char = 32;
  char_count = ((int) glyph_surface.get_size().y / char_height) * 16;

  if (glyph_width == FIXED)
    {
      for(uint32_t i = 0; i < char_count; ++i)
        {
          int x = (i % 16) * char_width;
          int y = (i / 16) * char_height;

          Glyph glyph;
          glyph.advance = char_width;
          glyph.offset  = Unison::Video::Point(0, 0);
          glyph.rect    = Unison::Video::Rect(x, y, char_width, char_height);

          glyphs.push_back(glyph);
          shadow_glyphs.push_back(glyph);
        }
    }
  else // glyph_width == VARIABLE
    {
      // Load the surface into RAM and scan the pixel data for characters
      Unison::Video::Surface surface(filename);
      for(uint32_t i = 0; i < char_count; ++i)
      {
        int x = (i % 16) * char_width;
        int y = (i / 16) * char_height;

        int left = x;
        while (left < x + char_width &&
               vline_empty(surface, left, y, y + char_height, 64))
          left += 1;

        int right = x + char_width - 1;
        while (right > left &&
               vline_empty(surface, right, y, y + char_height, 64))
          right -= 1;

        Glyph glyph;
        glyph.offset = Unison::Video::Point(0, 0);

        if (left <= right)
          glyph.rect = Unison::Video::Rect(left,  y, right - left + 1, char_height);
        else // glyph is completly transparent
          glyph.rect = Unison::Video::Rect(x,  y, char_width, char_height);

        glyph.advance = glyph.rect.size.x + 1; // FIXME: might be usefull to make spacing configurable

        glyphs.push_back(glyph);
        shadow_glyphs.push_back(glyph);
      }

      /*SDL_Surface* surface = IMG_Load_RW(get_physfs_SDLRWops(filename), 1);
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
                 vline_empty(surface, left, y, y + char_height, 64))
            left += 1;

          int right = x + char_width - 1;
          while (right > left &&
                 vline_empty(surface, right, y, y + char_height, 64))
            right -= 1;

          Glyph glyph;
          glyph.offset = Vector(0, 0);

          if (left <= right)
            glyph.rect = Rect(left,  y, right+1, y + char_height);
          else // glyph is completly transparent
            glyph.rect = Rect(x,  y, x + char_width, y + char_height);

          glyph.advance = glyph.rect.get_width() + 1; // FIXME: might be usefull to make spacing configurable

          glyphs.push_back(glyph);
          shadow_glyphs.push_back(glyph);
        } 
      SDL_UnlockSurface(surface);

      SDL_FreeSurface(surface);*/
    }
}

Font::~Font()
{
  //delete glyph_surface;
  //delete shadow_glyph_surface;
}

float
Font::get_text_width(const std::string& text) const
{
  float curr_width = 0;
  float last_width = 0;

  for(UTF8Iterator it(text); !it.done(); ++it)
    {
      if (*it == '\n')
        {
          last_width = std::max(last_width, curr_width);
          curr_width = 0;
        }
      else
        {
          int idx = chr2glyph(*it);
          curr_width += glyphs[idx].advance;
        }
    }

  return std::max(curr_width, last_width);
}

float
Font::get_text_height(const std::string& text) const
{
  std::string::size_type text_height = char_height;

  for(std::string::const_iterator it = text.begin(); it != text.end(); ++it)
    { // since UTF8 multibyte characters are decoded with values
      // outside the ASCII range there is no risk of overlapping and
      // thus we don't need to decode the utf-8 string
      if (*it == '\n')
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

std::string
Font::wrap_to_width(const std::string& s, float width, std::string* overflow)
{
  // if text is already smaller, return full text
  if (get_text_width(s) <= width) {
    if (overflow) *overflow = "";
    return s;
  }

  // if we can find a whitespace character to break at, return text up to this character
  for (int i = s.length()-1; i >= 0; i--) {
    std::string s2 = s.substr(0,i);
    if (s[i] != ' ') continue;
    if (get_text_width(s2) <= width) {
      if (overflow) *overflow = s.substr(i+1);
      return s.substr(0, i);
    }
  }
  
  // FIXME: hard-wrap at width, taking care of multibyte characters
  if (overflow) *overflow = "";
  return s;
}

void
Font::draw(Unison::Video::Blittable &dst, const std::string& text, const Vector& pos_,
           FontAlignment alignment, DrawingEffect drawing_effect,
           float alpha) const
{
  float x = pos_.x;
  float y = pos_.y;

  std::string::size_type last = 0;
  for(std::string::size_type i = 0;; ++i)
    {
      if (text[i] == '\n' || i == text.size())
        {
          std::string temp = text.substr(last, i - last);

          // calculate X positions based on the alignment type
          Vector pos = Vector(x, y);

          if(alignment == ALIGN_CENTER)
            pos.x -= get_text_width(temp) / 2;
          else if(alignment == ALIGN_RIGHT)
            pos.x -= get_text_width(temp);

          // Cast font position to integer to get a clean drawing result and
          // no bluring as we would get with subpixel positions
          pos.x = static_cast<int>(pos.x);

          draw_text(dst, temp, pos, drawing_effect, alpha);

          if (i == text.size())
            break;

          y += char_height + 2;
          last = i + 1;
        }
    }
}

void
Font::draw_text(Unison::Video::Blittable &dst, const std::string& text, const Vector& pos,
                DrawingEffect drawing_effect, float alpha) const
{
  if(shadowsize > 0)
    {
      // FIXME: shadow_glyph_surface and glyph_surface do currently
      // share the same glyph array, this is incorrect and should be
      // fixed, it is however hardly noticable
      draw_chars(dst, shadow_glyph_surface, text,
                 pos + Vector(shadowsize, shadowsize), drawing_effect, alpha);
    }

  draw_chars(dst, glyph_surface, text, pos, drawing_effect, alpha);
}

int
Font::chr2glyph(uint32_t chr) const
{
  int glyph_index = chr - first_char;

  // we don't have the control chars 0x80-0xa0 in the font
  if (chr >= 0x80) { // non-ascii character
    glyph_index -= 32;
    if(chr <= 0xa0) {
      log_debug << "Unsupported utf-8 character '" << chr << "' found" << std::endl;
      glyph_index = 0;
    }
  }

  if(glyph_index < 0 || glyph_index >= (int) char_count) {
    log_debug << "Unsupported utf-8 character found" << std::endl;
    glyph_index = 0;
  }

  return glyph_index;
}

void
Font::draw_chars(Unison::Video::Blittable &dst, const Unison::Video::Texture &pchars, const std::string& text,
                 const Vector& pos, DrawingEffect drawing_effect,
                 float alpha) const
{
  Unison::Video::Point p((int) pos.x, (int) pos.y);

  for(UTF8Iterator it(text); !it.done(); ++it)
    {
      int font_index = chr2glyph(*it);

      if(*it == '\n')
        {
          p.x = (int) pos.x;
          p.y += char_height + 2;
        }
      else if(*it == ' ')
        {
          p.x += (int) glyphs[font_index].advance;
        }
      else
        {
          const Glyph& glyph = glyphs[font_index];

          Unison::Video::RenderOptions options;
          options.alpha = (unsigned char) (alpha * 0xff);
          options.h_flip = (drawing_effect == HORIZONTAL_FLIP);
          options.v_flip = (drawing_effect == VERTICAL_FLIP);

          dst.blit(pchars, p + glyph.offset, glyph.rect, options);
          p.x += (int) glyphs[font_index].advance;

          /*DrawingRequest request;

          request.pos = p + glyph.offset;
          request.drawing_effect = drawing_effect;
          request.alpha = alpha;

          SurfacePartRequest surfacepartrequest;
          surfacepartrequest.size = glyph.rect.p2 - glyph.rect.p1;
          surfacepartrequest.source = glyph.rect.p1;
          surfacepartrequest.surface = pchars;

          request.request_data = &surfacepartrequest;
          renderer->draw_surface_part(request);

          p.x += glyphs[font_index].advance;*/
        }
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
 * gets unicode character at byte position @a p of UTF-8 encoded @a
 * text, then advances @a p to the next character.
 *
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
