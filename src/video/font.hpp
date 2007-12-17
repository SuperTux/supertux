//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>,
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

#ifndef SUPERTUX_FONT_H
#define SUPERTUX_FONT_H

#include <string>
#include <stdint.h>

#include "video/surface.hpp"
#include "math/vector.hpp"
#include "math/rect.hpp"

class Unison::Video::Blittable;

enum FontAlignment {
  ALIGN_LEFT,
  ALIGN_CENTER,
  ALIGN_RIGHT
};

class Font
{
public:
  enum GlyphWidth {
    FIXED,
    VARIABLE
  };

  /** Construct a fixed-width font
   *
   *  @param glyph_width  VARIABLE for proportional fonts, VARIABLE for monospace ones
   *  @param filename     image file containing the characters
   *  @param shadowfile   image file containing the characters shadows
   *  @param char_width   width of a character
   *  @param char_height  height of a character
   */
  Font(GlyphWidth glyph_width,
       const std::string& filename, const std::string& shadowfile,
       int char_width, int char_height, int shadowsize = 2);
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
   * returns the given string, truncated (preferrably at whitespace) to be at most max_chars characters long
   */
  static std::string wrap_to_chars(const std::string& text, int max_chars, std::string* overflow);

  /**
   * returns the given string, truncated (preferrably at whitespace) to be at most "width" pixels wide
   */
  std::string wrap_to_width(const std::string& text, float width, std::string* overflow);

  /** Draws the given text to the screen. Also needs the position.
   * Type of alignment, drawing effect and alpha are optional. */
  void draw(Unison::Video::Blittable &dst, const std::string& text, const Vector& pos,
            FontAlignment allignment = ALIGN_LEFT,
            DrawingEffect drawing_effect = NO_EFFECT,
            float alpha = 1.0f) const;

private:
  friend class DrawingContext;

  void draw_text(Unison::Video::Blittable &dst, const std::string& text, const Vector& pos,
                 DrawingEffect drawing_effect = NO_EFFECT,
                 float alpha = 1.0f) const;

  void draw_chars(Unison::Video::Blittable &dst, const Unison::Video::Texture &pchars, const std::string& text,
                  const Vector& position, DrawingEffect drawing_effect,
                  float alpha) const;

  /** Convert a Unicode character code to the index of its glyph */
  int chr2glyph(uint32_t chr) const;

  GlyphWidth glyph_width;
  Unison::Video::Texture   glyph_surface;
  Unison::Video::Texture   shadow_glyph_surface;
  int char_height;
  int shadowsize;

  /// the number of the first character that is represented in the font
  uint32_t first_char;
  /// the number of the last character that is represented in the font
  uint32_t char_count;

  struct Glyph {
    /** How many pixels should the cursor advance after printing the
        glyph */
    float advance;

    /** Offset that is used when drawing the glyph */
    Unison::Video::Point offset;

    /** Position of the glyph inside the surface */
    Unison::Video::Rect rect;
  };

  /** Location of the characters inside the surface */
  std::vector<Glyph> glyphs;
  std::vector<Glyph> shadow_glyphs;
};

#endif
