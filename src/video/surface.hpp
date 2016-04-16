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
#include <memory>

#include "math/vector.hpp"
#include "math/rect.hpp"
#include "video/surface_ptr.hpp"
#include "video/texture_ptr.hpp"

class SurfaceData;

/** A rectangular image.  The class basically holds a reference to a
    texture with additional UV coordinates that specify a rectangular
    area on this texture */
class Surface
{
public:
  static SurfacePtr create(const std::string& file);
  static SurfacePtr create(const std::string& file, const Rect& rect);
  static SurfacePtr create(TexturePtr texture_);

private:
  TexturePtr texture;
  SurfaceData* surface_data;
  Rect rect;
  bool flipx;

private:
  Surface(const std::string& file);
  Surface(const std::string& file, const Rect& rect);
  Surface(TexturePtr texture);
  Surface(const Surface&);

public:
  ~Surface();

  SurfacePtr clone() const;

  /** flip the surface horizontally */
  void hflip();
  bool get_flipx() const;

  TexturePtr get_texture() const;
  SurfaceData* get_surface_data() const;
  int get_x() const;
  int get_y() const;
  int get_width() const;
  int get_height() const;
  Vector get_position() const;

  /** returns a vector containing width and height */
  Vector get_size() const;

private:
  Surface& operator=(const Surface&);
};

#endif

/* EOF */
