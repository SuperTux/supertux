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
#ifndef SUPERTUX_LIGHTMAP_H
#define SUPERTUX_LIGHTMAP_H

#include <vector>
#include <string>
#include <memory>

#include <stdint.h>

#include <SDL_video.h>

#include "glutil.hpp"
#include "obstack/obstack.h"
#include "math/vector.hpp"
#include "math/rect.hpp"
#include "drawing_request.hpp"
#include "surface.hpp"
#include "font.hpp"
#include "color.hpp"

class Texture;
struct DrawingRequest;

class Lightmap
{
public:
  virtual ~Lightmap() {}

  virtual void start_draw(const Color &ambient_color) = 0;
  virtual void end_draw() = 0;
  virtual void do_draw() = 0;
  virtual void draw_surface(const DrawingRequest& request) = 0;
  virtual void draw_surface_part(const DrawingRequest& request) = 0;
  virtual void draw_gradient(const DrawingRequest& request) = 0;
  virtual void draw_filled_rect(const DrawingRequest& request) = 0;
  virtual void get_light(const DrawingRequest& request) const = 0;
};

#endif

