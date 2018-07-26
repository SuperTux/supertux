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

#ifndef HEADER_SUPERTUX_VIDEO_GL_RENDERER_HPP
#define HEADER_SUPERTUX_VIDEO_GL_RENDERER_HPP

#include <SDL.h>

#include "math/vector.hpp"
#include "video/renderer.hpp"

class GLVideoSystem;
struct DrawingRequest;

class GLRenderer : public Renderer
{
private:

public:
  GLRenderer(GLVideoSystem& video_system);
  ~GLRenderer();

  virtual void start_draw() override;
  virtual void end_draw() override;

  virtual void draw_surface(const DrawingRequest& request) override;
  virtual void draw_surface_part(const DrawingRequest& request) override;
  virtual void draw_gradient(const DrawingRequest& request) override;
  virtual void draw_filled_rect(const DrawingRequest& request) override;
  virtual void draw_inverse_ellipse(const DrawingRequest& request) override;
  virtual void draw_line(const DrawingRequest& request) override;
  virtual void draw_triangle(const DrawingRequest& request) override;
  virtual void clear(const Color& color) override;

  Vector to_logical(int physical_x, int physical_y) const override;

  virtual void set_clip_rect(const Rect& rect) override;
  virtual void clear_clip_rect() override;

  void set_viewport(const SDL_Rect& viewport, const Vector& scale);
  SDL_Rect get_viewport() const { return m_viewport; }

private:
  GLVideoSystem& m_video_system;
  SDL_Rect m_viewport;
  Vector m_scale;

private:
  GLRenderer(const GLRenderer&) = delete;
  GLRenderer& operator=(const GLRenderer&) = delete;
};

#endif

/* EOF */
