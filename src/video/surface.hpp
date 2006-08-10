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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#ifndef __SURFACE_HPP__
#define __SURFACE_HPP__

#include <string>
#include "math/vector.hpp"

class Color;
class Blend;
class ImageTexture;

/// bitset for drawing effects
enum DrawingEffect {
  /** Don't apply anything */
  NO_EFFECT       = 0x0000,
  /** Draw the Surface upside down */
  VERTICAL_FLIP     = 0x0001,
  /** Draw the Surface from left to down */
  HORIZONTAL_FLIP   = 0x0002,
};

/**
 * A rectangular image.
 * The class basically holds a reference to a texture with additional UV
 * coordinates that specify a rectangular area on this texture
 */
class Surface
{
private:
  friend class DrawingContext;
  friend class Font;
  ImageTexture* texture;

  float uv_left;
  float uv_top;
  float uv_right;
  float uv_bottom;

  void draw(float x, float y, float alpha, float angle, const Color& color, const Blend& blend, DrawingEffect effect) const;
  void draw(float x, float y, float alpha, DrawingEffect effect) const;
  void draw_part(float src_x, float src_y, float dst_x, float dst_y,
                 float width, float height,
                 float alpha, DrawingEffect effect) const;

  float width;
  float height;
public:
  Surface(const std::string& file);
  Surface(const std::string& file, int x, int y, int w, int h);
  Surface(const Surface& other);
  ~Surface();

  /** flip the surface horizontally */
  void hflip();

  const Surface& operator= (const Surface& other);

  float get_width() const
  {
    return width;
  }

  float get_height() const
  {
    return height;
  }

  /**
   * returns a vector containing width and height
   */
  Vector get_size() const
  { return Vector(get_width(), get_height()); }

};

#endif
