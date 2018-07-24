//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                     Ingo Ruhnke <grumbel@gmail.com>
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

#include "video/font.hpp"

#include <SDL_image.h>
#include <algorithm>
#include <physfs.h>

#include "physfs/physfs_sdl.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/utf8_iterator.hpp"
#include "video/drawing_request.hpp"
#include "video/renderer.hpp"
#include "video/surface.hpp"

namespace {

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
  border(0),
  rtl(false),
  glyphs(65536)
{
  for(unsigned int i=0; i<65536;i++) glyphs[i].surface_idx = -1;

  const std::string fontdir = FileSystem::dirname(filename);
  const std::string fontname = FileSystem::basename(filename);

  // scan for prefix-filename in addons search path
  char **rc = PHYSFS_enumerateFiles(fontdir.c_str());
  for (char **i = rc; *i != NULL; i++) {
    std::string filename_(*i);
    if( filename_.rfind(fontname) != std::string::npos ) {
      try {
        loadFontFile(fontdir + filename_);
      }
      catch(const std::exception& e)
      {
        log_fatal << "Couldn't load font file: " << e.what() << std::endl;
      }
    }
  }
  PHYSFS_freeList(rc);
}

void
Font::loadFontFile(const std::string &filename)
{
  // FIXME: Workaround for a crash on MSYS2 when starting with --debug
  log_debug_ << "Loading font: " << filename << std::endl;
  auto doc = ReaderDocument::parse(filename);
  auto root = doc.get_root();
  if (root.get_name() != "supertux-font") {
    std::ostringstream msg;
    msg << "Font file:" << filename << ": is not a supertux-font file";
    throw std::runtime_error(msg.str());
  }

  auto config_l = root.get_mapping();

  int def_char_width=0;

  if( !config_l.get("glyph-width",def_char_width) ) {
    log_warning << "Font:"<< filename << ": misses default glyph-width" << std::endl;
  }

  if( !config_l.get("glyph-height",char_height) ) {
    std::ostringstream msg;
    msg << "Font:" << filename << ": misses glyph-height";
    throw std::runtime_error(msg.str());
  }

  config_l.get("glyph-border", border);
  config_l.get("rtl", rtl);

  auto iter = config_l.get_iter();
  while(iter.next()) {
    const std::string& token = iter.get_key();
    if( token == "surface" ) {
      auto glyphs_val = iter.as_mapping();
      int local_char_width;
      bool monospaced;
      GlyphWidth local_glyph_width;
      std::string glyph_image;
      std::string shadow_image;
      std::vector<std::string> chars;
      if( ! glyphs_val.get("glyph-width", local_char_width) ) {
        local_char_width = def_char_width;
      }
      if( ! glyphs_val.get("monospace", monospaced ) ) {
        local_glyph_width = glyph_width;
      }
      else {
        if( monospaced ) local_glyph_width = FIXED;
        else local_glyph_width = VARIABLE;
      }
      if( ! glyphs_val.get("glyphs", glyph_image) ) {
        std::ostringstream msg;
        msg << "Font:" << filename << ": missing glyphs image";
        throw std::runtime_error(msg.str());
      }
      if( ! glyphs_val.get("shadows", shadow_image) ) {
        std::ostringstream msg;
        msg << "Font:" << filename << ": missing shadows image";
        throw std::runtime_error(msg.str());
      }
      if( ! glyphs_val.get("chars", chars) || chars.size() == 0) {
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
  GlyphWidth glyph_width_,
  int char_width
  )
{
  SurfacePtr glyph_surface  = Surface::create("images/engine/fonts/" + glyphimage);
  SurfacePtr shadow_surface = Surface::create("images/engine/fonts/" + shadowimage);

  int surface_idx = glyph_surfaces.size();
  glyph_surfaces.push_back(glyph_surface);
  shadow_surfaces.push_back(shadow_surface);

  int row=0, col=0;
  int wrap = glyph_surface->get_width() / char_width;

  SDL_Surface *surface = NULL;

  if( glyph_width_ == VARIABLE ) {
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
      int y = row * (char_height + 2*border) + border;
      int x = col * (char_width + 2*border) + border;
      if( ++col == wrap ) { col=0; row++; }
      if( *chr == 0x0020 && glyphs[0x20].surface_idx != -1) continue;

      Glyph glyph;
      glyph.surface_idx   = surface_idx;

      if( glyph_width_ == FIXED || (*chr <= 255 && isdigit(*chr)) )
      {
        glyph.rect    = Rectf(x, y, x + char_width, y + char_height);
        glyph.offset  = Vector(0, 0);
        glyph.advance = char_width;
      }
      else
      {
        if (y + char_height > surface->h)
        {
          log_warning << "error: font definition contains more letter then the images: " << glyphimage << std::endl;
          goto abort;
        }

        int left = x;
        while (left < x + char_width && vline_empty(surface, left, y, y + char_height, 64))
          left += 1;
        int right = x + char_width - 1;
        while (right > left && vline_empty(surface, right, y, y + char_height, 64))
          right -= 1;

        if (left <= right)
        {
          glyph.offset  = Vector(x-left, 0);
          glyph.advance = right - left + 1 + 1; // FIXME: might be useful to make spacing configurable
        }
        else
        { // glyph is completly transparent
          glyph.offset  = Vector(0, 0);
          glyph.advance = char_width + 1; // FIXME: might be useful to make spacing configurable
        }

        glyph.rect = Rectf(x,  y, x + char_width, y + char_height);
      }

      glyphs[*chr] = glyph;
    }
    if( col>0 && col <= wrap ) {
      col = 0;
      row++;
    }
  }
abort:

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
    draw_chars(renderer, false, rtl ? std::string(text.rbegin(), text.rend()) : text,
               pos + Vector(shadowsize, shadowsize), drawing_effect, Color(1,1,1), alpha);

  draw_chars(renderer, true, rtl ? std::string(text.rbegin(), text.rend()) : text, pos, drawing_effect, color, alpha);
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
      surfacepartrequest.srcrect = glyph.rect;
      surfacepartrequest.dstsize = glyph.rect.get_size();
      surfacepartrequest.surface = notshadow ? glyph_surfaces[glyph.surface_idx].get() : shadow_surfaces[glyph.surface_idx].get();

      request.request_data = &surfacepartrequest;
      renderer->draw_surface_part(request);

      p.x += glyph.advance;
    }
  }
}

/* EOF */
