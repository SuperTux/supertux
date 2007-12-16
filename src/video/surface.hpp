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
#include "file_system.hpp"
#include <unison/video/Texture.hpp>

/// bitset for drawing effects
enum DrawingEffect {
  /** Don't apply anything */
  NO_EFFECT,
  /** Draw the Surface upside down */
  VERTICAL_FLIP,
  /** Draw the Surface from left to down */
  HORIZONTAL_FLIP,
  NUM_EFFECTS
};

/**
 * A rectangular image.
 * The class basically holds a reference to a texture with additional UV
 * coordinates that specify a rectangular area on this texture
 */
class Surface
{
private:
  Unison::Video::TextureSection texture;
  bool flipx;

public:
  Surface(const std::string& file) :
    texture(FileSystem::normalize(file)),
    flipx(false)
  {
  }

  Surface(const std::string& file, int x, int y, int w, int h) :
    texture(FileSystem::normalize(file), Unison::Video::Rect(x, y, w, h)),
    flipx(false)
  {
  }

  Surface(const Surface& other) :
    texture(other.texture),
    flipx(false)
  {
  }

  ~Surface()
  {
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
    texture = other.texture;
    return *this;
  }

  Unison::Video::TextureSection get_texture() const
  {
    return texture;
  }

  int get_x() const
  {
    return texture.clip_rect.pos.x;
  }

  int get_y() const
  {
    return texture.clip_rect.pos.y;
  }

  int get_width() const
  {
    return texture.clip_rect.size.x ? texture.clip_rect.size.x : texture.image.get_size().x;
  }

  int get_height() const
  {
    return texture.clip_rect.size.y ? texture.clip_rect.size.y : texture.image.get_size().y;
  }

  Vector get_position() const
  { return Vector(get_x(), get_y()); }

  /**
   * returns a vector containing width and height
   */
  Vector get_size() const
  { return Vector(get_width(), get_height()); }
};

#endif
