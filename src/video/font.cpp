//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                     Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <SDL_image.h>
#include <physfs.h>

#include "physfs/physfs_sdl.hpp"

#include "util/file_system.hpp"

#include "lisp/list_iterator.hpp"
#include "lisp/parser.hpp"
#include "supertux/screen.hpp"
#include "util/log.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"

namespace {
bool     has_multibyte_mark(unsigned char c);
uint32_t decode_utf8(const std::string& text, size_t& p);
std::string encode_utf8(uint32_t code);

struct UTF8Iterator
{
  const std::string&     text;
  std::string::size_type pos;
  uint32_t chr;

  UTF8Iterator(const std::string& text_) :
    text(text_),
    pos(0),
    chr()
  {
    try {
      chr = decode_utf8(text, pos);
    } catch (std::exception) {
      log_debug << "Malformed utf-8 sequence beginning with " << *((uint32_t*)(text.c_str() + pos)) << " found " << std::endl;
      chr = 0;
    }
  }

  bool done() const
  {
    return pos > text.size();
  }

  UTF8Iterator& operator++() {
    try {
      chr = decode_utf8(text, pos);
    } catch (std::exception) {
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

Font::Font(GlyphWidth glyph_width_,
           const std::string& filename,
           int shadowsize_) :
  glyph_width(glyph_width_),
  glyph_surfaces(),
  shadow_surfaces(),
  char_height(),
  shadowsize(shadowsize_),
  glyphs(65536)
{
  for(unsigned int i=0; i<65536;i++) glyphs[i].surface_idx = -1;

  const std::string fontdir = FileSystem::dirname(filename);
  const std::string fontname = FileSystem::basename(filename);

  // scan for prefix-filename in addons search path
  char **rc = PHYSFS_enumerateFiles(fontdir.c_str());
  for (char **i = rc; *i != NULL; i++) {
    std::string filename(*i);
    if( filename.rfind(fontname) != std::string::npos ) {
      loadFontFile(fontdir + filename);
    }
  }
  PHYSFS_freeList(rc);
}

void 
Font::loadFontFile(const std::string &filename)
{
  lisp::Parser parser;
  log_debug << "Loading font: " << filename << std::endl;
  const lisp::Lisp* root = parser.parse(filename);
  const lisp::Lisp* config_l = root->get_lisp("supertux-font");

  if(!config_l) {
    std::ostringstream msg;
    msg << "Font file:" << filename << ": is not a supertux-font file";
    throw std::runtime_error(msg.str());
  }

  int def_char_width=0;

  if( !config_l->get("glyph-width",def_char_width) ) {
    log_warning << "Font:"<< filename << ": misses default glyph-width" << std::endl;
  }
  
  if( !config_l->get("glyph-height",char_height) ) {
    std::ostringstream msg;
    msg << "Font:" << filename << ": misses glyph-height";
    throw std::runtime_error(msg.str());
  }

  lisp::ListIterator iter(config_l);
  while(iter.next()) {
    const std::string& token = iter.item();
    if( token == "surface" ) {
      const lisp::Lisp * glyphs_val = iter.lisp();
      int local_char_width;
      bool monospaced;
      GlyphWidth local_glyph_width;
      std::string glyph_image;
      std::string shadow_image;
      std::vector<std::string> chars;
      if( ! glyphs_val->get("glyph-width", local_char_width) ) {
        local_char_width = def_char_width;
      }
      if( ! glyphs_val->get("monospace", monospaced ) ) {
        local_glyph_width = glyph_width;
      }
      else {
        if( monospaced ) local_glyph_width = FIXED;
        else local_glyph_width = VARIABLE;
      }
      if( ! glyphs_val->get("glyphs", glyph_image) ) {
        std::ostringstream msg;
        msg << "Font:" << filename << ": missing glyphs image";
        throw std::runtime_error(msg.str());
      }
      if( ! glyphs_val->get("shadows", shadow_image) ) {
        std::ostringstream msg;
        msg << "Font:" << filename << ": missing shadows image";
        throw std::runtime_error(msg.str());
      }
      if( ! glyphs_val->get("chars", chars) || chars.size() == 0) {
        std::ostringstream msg;
        msg << "Font:" << filename << ": missing chars definition";
        throw std::runtime_error(msg.str());
      }

      if( local_char_width==0 ) {
        std::ostringstream msg;
        msg << "Font:" << filename << ": misses glyph-width for some surface";
        throw std::runtime_error(msg.str());
      }

      loadFontSurface(glyph_image, shadow_image, chars,
                      local_glyph_width, local_char_width);
    }
  }
}

void 
Font::loadFontSurface(
  const std::string &glyphimage,
  const std::string &shadowimage,
  const std::vector<std::string> &chars,
  GlyphWidth glyph_width,
  int char_width
  )
{
  Surface glyph_surface("images/engine/fonts/" + glyphimage);
  Surface shadow_surface("images/engine/fonts/" + shadowimage);

  int surface_idx = glyph_surfaces.size();
  glyph_surfaces.push_back(glyph_surface);
  shadow_surfaces.push_back(shadow_surface);

  int row=0, col=0;
  int wrap = glyph_surface.get_width() / char_width;
 
  SDL_Surface *surface = NULL;
  
  if( glyph_width == VARIABLE ) {
    //this does not work:
    // surface = ((SDL::Texture *)glyph_surface.get_texture())->get_texture();
    surface = IMG_Load_RW(get_physfs_SDLRWops("images/engine/fonts/"+glyphimage), 1);
    if(surface == NULL) {
      std::ostringstream msg;
      msg << "Couldn't load image '" << glyphimage << "' :" << SDL_GetError();
      throw std::runtime_error(msg.str());
    }
    SDL_LockSurface(surface);
  }

  for( unsigned int i = 0; i < chars.size(); i++) {
    for(UTF8Iterator chr(chars[i]); !chr.done(); ++chr) {
      int y = row * char_height;
      int x = col * char_width;
      if( ++col == wrap ) { col=0; row++; }
      if( *chr == 0x0020 && glyphs[0x20].surface_idx != -1) continue;
        
      Glyph glyph;
      glyph.surface_idx   = surface_idx;
      
      if( glyph_width == FIXED ) {
        glyph.rect    = Rect(x, y, x + char_width, y + char_height);
        glyph.offset  = Vector(0, 0);
        glyph.advance = char_width;
      }
      else {
        int left = x;
        while (left < x + char_width && vline_empty(surface, left, y, y + char_height, 64))
          left += 1;
        int right = x + char_width - 1;
        while (right > left && vline_empty(surface, right, y, y + char_height, 64))
          right -= 1;
          
        if (left <= right)
          glyph.rect = Rect(left,  y, right+1, y + char_height);
        else // glyph is completely transparent
          glyph.rect = Rect(x,  y, x + char_width, y + char_height);
        
        glyph.offset  = Vector(0, 0);
        glyph.advance = glyph.rect.get_width() + 1; // FIXME: might be useful to make spacing configurable
      }

      glyphs[*chr] = glyph;
    }
    if( col>0 && col <= wrap ) { 
      col = 0;
      row++;
    }
  }
  
  if( surface != NULL ) {
    SDL_UnlockSurface(surface);
    SDL_FreeSurface(surface);
  }
}

Font::~Font()
{
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
      if( glyphs.at(*it).surface_idx != -1 )
        curr_width += glyphs[*it].advance;
      else 
        curr_width += glyphs[0x20].advance;
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
Font::wrap_to_width(const std::string& s_, float width, std::string* overflow)
{
  std::string s = s_;

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
Font::draw(Renderer *renderer, const std::string& text, const Vector& pos_,
           FontAlignment alignment, DrawingEffect drawing_effect, Color color,
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
      // no blurring as we would get with subpixel positions
      pos.x = static_cast<int>(pos.x);

      draw_text(renderer, temp, pos, drawing_effect, color, alpha);

      if (i == text.size())
        break;

      y += char_height + 2;
      last = i + 1;
    }
  }
}

void
Font::draw_text(Renderer *renderer, const std::string& text, const Vector& pos,
                DrawingEffect drawing_effect, Color color, float alpha) const
{
  if(shadowsize > 0)
    draw_chars(renderer, false, text, 
               pos + Vector(shadowsize, shadowsize), drawing_effect, Color(1,1,1), alpha);

  draw_chars(renderer, true, text, pos, drawing_effect, color, alpha);
}

void
Font::draw_chars(Renderer *renderer, bool notshadow, const std::string& text,
                 const Vector& pos, DrawingEffect drawing_effect, Color color,
                 float alpha) const
{
  Vector p = pos;

  for(UTF8Iterator it(text); !it.done(); ++it)
  {
    if(*it == '\n')
    {
      p.x = pos.x;
      p.y += char_height + 2;
    }
    else if(*it == ' ')
    {
      p.x += glyphs[0x20].advance;
    }
    else
    {
      Glyph glyph;
      if( glyphs.at(*it).surface_idx != -1 )
        glyph = glyphs[*it];
      else 
        glyph = glyphs[0x20];

      DrawingRequest request;

      request.pos = p + glyph.offset;
      request.drawing_effect = drawing_effect;
      request.color = color;
      request.alpha = alpha;

      SurfacePartRequest surfacepartrequest;
      surfacepartrequest.size = glyph.rect.p2 - glyph.rect.p1;
      surfacepartrequest.source = glyph.rect.p1;
      surfacepartrequest.surface = notshadow ? &(glyph_surfaces[glyph.surface_idx]) : &(shadow_surfaces[glyph.surface_idx]);

      request.request_data = &surfacepartrequest;
      renderer->draw_surface_part(request);

      p.x += glyph.advance;
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

/* EOF */
