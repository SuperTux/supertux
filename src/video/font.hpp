//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>,
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

#ifndef HEADER_SUPERTUX_VIDEO_FONT_HPP
#define HEADER_SUPERTUX_VIDEO_FONT_HPP

#include <stdint.h>
#include <string>

#include "math/rectf.hpp"
#include "math/vector.hpp"
#include "video/color.hpp"
#include "video/surface.hpp"
#include "video/texture.hpp"

class Renderer;

enum FontAlignment {
  ALIGN_LEFT,
  ALIGN_CENTER,
  ALIGN_RIGHT
};

class Font
{
public:
  /** Construct a fixed-width font
   *
   *  @param glyph_width  VARIABLE for proportional fonts, VARIABLE for monospace ones
   *  @param fontfile     file in format supertux-font
   *  @param sgadowsize   offset of shadow
   */
  Font(const std::string& fontfile, int font_size, int shadowsize = 2);
  ~Font();

  /** returns the width of a given text. (Note that I won't add a normal
   * get_width function here, as we might switch to variable width fonts in the
   * future.)
   * Supports breaklines.
   */
  float get_text_width(const std::string& text) const;

  /** returns the height of a given text. This function supports breaklines.
   * In case, you are positive that your text doesn't use break lines, you can
   * just use get_height().
   */
  float get_text_height(const std::string& text) const;

  /**
   * returns the height of the font.
   */
  float get_height() const;

  /**
   * returns the given string, truncated (preferably at whitespace) to be at most max_chars characters long
   */
  static std::string wrap_to_chars(const std::string& text, int max_chars, std::string* overflow);

  /**
   * returns the given string, truncated (preferably at whitespace) to be at most "width" pixels wide
   */
  std::string wrap_to_width(const std::string& text, float width, std::string* overflow);

   /**
    * returns the equivalent TrueTypeFont
    */
   TTF_Font* get_ttf_font() const;

   /**
    * Returns the shadow size for the current font
    */
  unsigned int get_shadow_size() const;

private:
  int shadowsize;
  int border;
  bool rtl;

  const std::string file_name;
  int fontsize;
  TTF_Font* ttf_font;

  Font(const Font&);
  Font operator=(const Font&);
};

#endif

/* EOF */
