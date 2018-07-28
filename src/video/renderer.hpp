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

#include "math/rect.hpp"
#include "math/vector.hpp"
#include "video/color.hpp"

class Rect;
struct DrawingRequest;
struct SDL_Window;

class Renderer
{
public:
  virtual ~Renderer() {}

  virtual void start_draw() = 0;
  virtual void end_draw() = 0;

  virtual void draw_surface(const DrawingRequest& request) = 0;
  virtual void draw_surface_part(const DrawingRequest& request) = 0;
  virtual void draw_gradient(const DrawingRequest& request) = 0;
  virtual void draw_filled_rect(const DrawingRequest& request) = 0;
  virtual void draw_inverse_ellipse(const DrawingRequest& request) = 0;
  virtual void draw_line(const DrawingRequest& request) = 0;
  virtual void draw_triangle(const DrawingRequest& request) = 0;
  virtual void clear(const Color& color) = 0;

  virtual void set_clip_rect(const Rect& rect) = 0;
  virtual void clear_clip_rect() = 0;
};

#endif

/* EOF */
