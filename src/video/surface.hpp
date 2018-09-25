//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_VIDEO_SURFACE_HPP
#define HEADER_SUPERTUX_VIDEO_SURFACE_HPP

#include <string>

#include "math/rect.hpp"
#include "math/vector.hpp"
#include "video/drawing_effect.hpp"
#include "video/surface_ptr.hpp"
#include "video/texture_ptr.hpp"

class SurfaceData;

/** A rectangular image.  The class basically holds a reference to a
    texture with additional UV coordinates that specify a rectangular
    area on this texture */
class Surface
{
public:
  static SurfacePtr from_file(const std::string& filename);
  static SurfacePtr from_file(const std::string& filename, const Rect& rect);
  static SurfacePtr from_texture(const TexturePtr& texture);

private:
  TexturePtr m_texture;
  Rect m_rect;
  DrawingEffect m_effect;

private:
  Surface(const TexturePtr& texture, const Rect& rect, DrawingEffect effect);
  Surface(const Surface&);

public:
  ~Surface();

  SurfacePtr clone(DrawingEffect effect = 0) const;
  DrawingEffect get_effect() const;

  TexturePtr get_texture() const;
  int get_x() const;
  int get_y() const;
  int get_width() const;
  int get_height() const;
  Vector get_position() const;
  Vector get_size() const;

private:
  Surface& operator=(const Surface&);
};

#endif

/* EOF */
