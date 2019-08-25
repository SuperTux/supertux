//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>,
//                     Tobias Markus <tobbi.bugs@googlemail.com>
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

#include "video/ttf_font.hpp"

#include <iostream>
#include <sstream>

#include "util/line_iterator.hpp"
#include "physfs/physfs_sdl.hpp"
#include "video/canvas.hpp"
#include "video/surface.hpp"
#include "video/ttf_surface_manager.hpp"

TTFFont::TTFFont(const std::string& filename, int font_size, float line_spacing, int shadow_size, int border) :
  m_font(),
  m_filename(filename),
  m_font_size(font_size),
  m_line_spacing(line_spacing),
  m_shadow_size(shadow_size),
  m_border(border)
{
  m_font = TTF_OpenFontRW(get_physfs_SDLRWops(m_filename), 1, font_size);
  if (!m_font)
  {
    std::ostringstream msg;
    msg << "Couldn't load TTFFont: " << m_filename << ": " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }
}

TTFFont::~TTFFont()
{
  TTF_CloseFont(m_font);
}

float
TTFFont::get_text_width(const std::string& text) const
{
  if (text.empty())
    return 0.0f;

  float max_width = 0.0f;

  LineIterator iter(text);
  while (iter.next())
  {
    const std::string& line = iter.get();

    // Since create_surface() takes a surface from the cache instead of
    // generating it from scratch it should be faster than doing a whole
    // layout.
    if ((false))
    {
      int w = 0;
      int h = 0;
      int ret = TTF_SizeUTF8(m_font, line.c_str(), &w, &h);
      if (ret < 0)
      {
        std::cerr << "TTFFont::get_text_width(): " << TTF_GetError() << std::endl;
      }
      max_width = std::max(max_width, static_cast<float>(w));
    }
    else
    {
      TTFSurfacePtr surface = TTFSurfaceManager::current()->create_surface(*this, line);
      max_width = std::max(max_width, static_cast<float>(surface->get_width()));
    }
  }

  return max_width;
}

float
TTFFont::get_text_height(const std::string& text) const
{
  if (text.empty())
    return 0.0f;

  float total_height = get_height();
  // since UTF8 multibyte characters are decoded with values
  // outside the ASCII range there is no risk of overlapping and
  // thus we don't need to decode the utf-8 string
  for (auto c : text)
  {
    if (c == '\n')
    {
      total_height += get_height();
    }
  }

  return total_height;
}

void
TTFFont::draw_text(Canvas& canvas, const std::string& text,
                   const Vector& pos, FontAlignment alignment, int layer, const Color& color)

{
  float last_y = pos.y - (static_cast<float>(TTF_FontHeight(m_font)) - get_height()) / 2.0f;

  LineIterator iter(text);
  while (iter.next())
  {
    const std::string& line = iter.get();

    if (!line.empty())
    {
      TTFSurfacePtr ttf_surface = TTFSurfaceManager::current()->create_surface(*this, line);

      Vector new_pos(pos.x, last_y);

      if (alignment == ALIGN_CENTER)
      {
        new_pos.x -= static_cast<float>(ttf_surface->get_width()) / 2.0f;
      }
      else if (alignment == ALIGN_RIGHT)
      {
        new_pos.x -= static_cast<float>(ttf_surface->get_width());
      }

      // draw text
      canvas.draw_surface(ttf_surface->get_surface(), new_pos.floor(), 0.0f, color, Blend(), layer);
    }

    last_y += get_height();
  }
}

std::string
TTFFont::wrap_to_width(const std::string& text, float width, std::string* overflow)
{
  std::string s = text;

  // if text is already smaller, return full text
  if (get_text_width(s) <= width) {
    if (overflow) *overflow = "";
    return s;
  }

  // if we can find a whitespace character to break at, return text up to this character
  for (int i = static_cast<int>(s.length()) - 1; i >= 0; i--) {
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

/* EOF */
