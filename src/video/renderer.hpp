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

#ifndef HEADER_SUPERTUX_VIDEO_RENDERER_HPP
#define HEADER_SUPERTUX_VIDEO_RENDERER_HPP

#include <memory>
#include <string>
#include <vector>

#include <SDL_video.h>
#include <assert.h>
#include <stdint.h>

#include "math/rect.hpp"
#include "math/vector.hpp"
#include "obstack/obstack.h"
#include "video/color.hpp"
#include "video/font.hpp"
#include "video/glutil.hpp"
#include "video/surface.hpp"

class Surface;
class Texture;
struct DrawingRequest;

class Renderer
{
public:
  Renderer();
  virtual ~Renderer();

  virtual void draw_surface(const DrawingRequest& request) = 0;
  virtual void draw_surface_part(const DrawingRequest& request) = 0;
  virtual void draw_gradient(const DrawingRequest& request) = 0;
  virtual void draw_filled_rect(const DrawingRequest& request)= 0;
  virtual void draw_inverse_ellipse(const DrawingRequest& request)= 0;
  virtual void do_take_screenshot() = 0;
  virtual void flip() = 0;
  virtual void resize(int w, int h) = 0;
  virtual void apply_config() = 0;

  static Renderer* instance() { assert(instance_); return instance_; }
  
protected:
  static Renderer* instance_;
};

#endif

/* EOF */
