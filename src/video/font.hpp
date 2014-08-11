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
  enum GlyphWidth {
    FIXED,
    VARIABLE
  };

public:
  /** Construct a fixed-width font
   *
   *  @param glyph_width  VARIABLE for proportional fonts, VARIABLE for monospace ones
   *  @param fontfile     file in format supertux-font
   *  @param sgadowsize   offset of shadow
   */
  Font(GlyphWidth glyph_width, const std::string& fontfile, int shadowsize = 2);
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

  /** Draws the given text to the screen. Also needs the position.
   * Type of alignment, drawing effect and alpha are optional. */
  void draw(Renderer *renderer, const std::string& text, const Vector& pos,
            FontAlignment alignment = ALIGN_LEFT,
            DrawingEffect drawing_effect = NO_EFFECT,
            Color color = Color(1.0,1.0,1.0),
            float alpha = 1.0f) const;

private:
  friend class DrawingContext;

  void draw_text(Renderer *renderer, const std::string& text, const Vector& pos,
                 DrawingEffect drawing_effect = NO_EFFECT,
                 Color color = Color(1.0,1.0,1.0),
                 float alpha = 1.0f) const;

  void draw_chars(Renderer *renderer, bool nonshadow, const std::string& text,
                  const Vector& position, DrawingEffect drawing_effect, Color color,
                  float alpha) const;

  void loadFontFile(const std::string &filename);
  void loadFontSurface(const std::string &glyphimage,
                       const std::string &shadowimage,
                       const std::vector<std::string> &chars,
                       GlyphWidth glyph_width,
                       int char_width);
private:
  struct Glyph {
    /** How many pixels should the cursor advance after printing the
        glyph */
    float advance;

    /** Offset that is used when drawing the glyph */
    Vector offset;

    /** index of containing surface */
    int surface_idx;

    /** Position of the glyph inside the surface */
    Rectf rect;

    Glyph() :
      advance(),
      offset(),
      surface_idx(),
      rect()
    {}
  };

private:
  GlyphWidth glyph_width;

  std::vector<SurfacePtr>  glyph_surfaces;
  std::vector<SurfacePtr>  shadow_surfaces;
  int char_height;
  int shadowsize;
  int border;

  /** 65536 of glyphs */
  std::vector<Glyph> glyphs;
};

#endif

/* EOF */
