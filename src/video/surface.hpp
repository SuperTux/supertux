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

#include <config.h>

#include <string>
#include <SDL.h>
#include "math/vector.hpp"
#include "texture.hpp"

/**
 * A rectangular image.
 * The class basically holds a reference to a texture with additional UV
 * coordinates that specify a rectangular area on this texture
 */
class Surface
{
private:
  Texture* texture;
  int x;
  int y;
  int w;
  int h;
  bool flipx;

public:
  Surface(const std::string& file) :
    texture(texture_manager->get(file)),
    x(0), y(0), w(0), h(0),
    flipx(false)
  {
    texture->ref();
    w = texture->get_image_width();
    h = texture->get_image_height();
  }

  Surface(const std::string& file, int x, int y, int w, int h) :
    texture(texture_manager->get(file)),
    x(x), y(y), w(w), h(h),
    flipx(false)
  {
    texture->ref();
  }

  Surface(const Surface& other) :
    texture(other.texture),
    x(other.x), y(other.y),
    w(other.w), h(other.h),
    flipx(false)
  {
    texture->ref();
  }

  ~Surface()
  {
    texture->unref();
  }

  /** flip the surface horizontally */
  void hflip()
  {
    flipx = !flipx;
  }

  bool get_flipx() const
  {
    return flipx;
  }

  const Surface& operator= (const Surface& other)
  {
    other.texture->ref();
    texture->unref();
    texture = other.texture;
    x = other.x;
    y = other.y;
    w = other.w;
    h = other.h;
    return *this;
  }

  Texture *get_texture() const
  {
    return texture;
  }

  int get_x() const
  {
    return x;
  }

  int get_y() const
  {
    return y;
  }

  int get_width() const
  {
    return w;
  }

  int get_height() const
  {
    return h;
  }

  Vector get_position() const
  { return Vector(get_x(), get_y()); }

  /**
   * returns a vector containing width and height
   */
  Vector get_size() const
  { return Vector(get_width(), get_height()); }

  float get_uv_left() const
  {
    return (float) (x + (flipx ? w : 0)) / texture->get_texture_width();
  }

  float get_uv_top() const
  {
    return (float) y / texture->get_texture_height();
  }

  float get_uv_right() const
  {
    return (float) (x + (flipx ? 0 : w)) / texture->get_texture_width();
  }

  float get_uv_bottom() const
  {
    return (float) (y + h) / texture->get_texture_height();
  }

  //void draw_part(float src_x, float src_y, float dst_x, float dst_y,
  //               float width, float height, float alpha,
  //               DrawingEffect effect) const;
};

#endif
