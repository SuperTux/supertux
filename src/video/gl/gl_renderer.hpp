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

#include "math/size.hpp"
#include "video/drawing_request.hpp"
#include "video/renderer.hpp"

#include "SDL.h"
#include <math.h>

class GLRenderer : public Renderer
{
private:
  SDL_Window* m_window;
  SDL_GLContext m_glcontext;
  SDL_Rect m_viewport;
  Size m_desktop_size;
  bool m_fullscreen_active;

public:
  GLRenderer();
  ~GLRenderer();

  void start_draw() override;
  void end_draw() override;
  void draw_surface(const DrawingRequest& request) override;
  void draw_surface_part(const DrawingRequest& request) override;
  void draw_gradient(const DrawingRequest& request) override;
  void draw_filled_rect(const DrawingRequest& request) override;
  void draw_inverse_ellipse(const DrawingRequest& request) override;
  void draw_text(const DrawingRequest& request) override;
  void draw_line(const DrawingRequest& request) override;
  void draw_triangle(const DrawingRequest& request) override;
  void do_take_screenshot() override;
  void flip() override;
  void resize(int w, int h) override;
  void apply_config() override;
  Vector to_logical(int physical_x, int physical_y) const override;
  void set_gamma(float gamma) override;

  SDL_Window* get_window() const override { return m_window; }

private:
  void apply_video_mode();

private:
  GLRenderer(const GLRenderer&) = delete;
  GLRenderer& operator=(const GLRenderer&) = delete;
};

#endif

/* EOF */
