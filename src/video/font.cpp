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

#include <config.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <SDL_image.h>
#include <physfs.h>

#include "physfs/physfs_sdl.hpp"
#include "supertux/screen.hpp"
#include "supertux/resources.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/utf8_iterator.hpp"
#include "video/drawing_context.hpp"
#include "video/drawing_request.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"

Font::Font(const std::string& filename,
           int font_size,
           int shadowsize_):
  shadowsize(shadowsize_),
  border(0),
  rtl(false),
  file_name(filename),
  fontsize(font_size),
  ttf_font()
{
  ttf_font = TTF_OpenFont(filename.c_str(), font_size);
  if(ttf_font == nullptr)
  {
    log_debug << "Couldn't open font!" << std::endl;
  }
}

Font::~Font()
{
  if(ttf_font != nullptr)
  {
    TTF_CloseFont(ttf_font);
    ttf_font = nullptr;
  }
}

float
Font::get_text_width(const std::string& text) const
{
  if(get_ttf_font() == nullptr)
    return 0;

  if(text.length() == 0)
    return 0;

  auto texture_manager = TextureManager::current();
  auto texture = texture_manager->get(get_ttf_font(), text);
  return texture->get_texture_width();
}

float
Font::get_text_height(const std::string& text) const
{
  if(text.length() == 0)
    return 0;

  auto texture_manager = TextureManager::current();
  auto texture = texture_manager->get(get_ttf_font(), text);
  std::string::size_type text_height = texture->get_texture_height();

  for(std::string::const_iterator it = text.begin(); it != text.end(); ++it)
  { // since UTF8 multibyte characters are decoded with values
    // outside the ASCII range there is no risk of overlapping and
    // thus we don't need to decode the utf-8 string
    if (*it == '\n')
    {
      text_height += texture->get_texture_height() + 2;
    }
  }

  return text_height;
}

float
Font::get_height() const
{
  // Adding a 2 pixel margin so that it looks better!
  return fontsize + 2;
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

TTF_Font*
Font::get_ttf_font() const
{
  if(ttf_font != nullptr)
    return ttf_font;

  return Resources::normal_font->get_ttf_font();
}

unsigned int
Font::get_shadow_size() const
{
  return shadowsize;
}

/* EOF */
