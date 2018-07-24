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

#ifndef HEADER_SUPERTUX_VIDEO_GL_LIGHTMAP_HPP
#define HEADER_SUPERTUX_VIDEO_GL_LIGHTMAP_HPP

#include <memory>

#include "video/lightmap.hpp"
#include "video/glutil.hpp"

class GLTexture;
class Texture;
struct DrawingRequest;

class GLLightmap : public Lightmap
{
public:
  GLLightmap();

  void start_draw(const Color &ambient_color) override;
  void end_draw() override;

  void draw_surface(const DrawingRequest& request) override;
  void draw_surface_part(const DrawingRequest& request) override;
  void draw_gradient(const DrawingRequest& request) override;
  void draw_filled_rect(const DrawingRequest& request) override;
  void draw_inverse_ellipse(const DrawingRequest& request) override;
  void draw_line(const DrawingRequest& request) override;
  void draw_triangle(const DrawingRequest& request) override;

  void get_light(const DrawingRequest& request) const override;
  void do_draw() override;

private:
  static const int s_LIGHTMAP_DIV = 5;

  std::shared_ptr<GLTexture> m_lightmap;
  int m_lightmap_width;
  int m_lightmap_height;
  float m_lightmap_uv_right;
  float m_lightmap_uv_bottom;
  GLint m_old_viewport[4]; //holds vieport before redefining in start_draw - returned from glGet

private:
  GLLightmap(const GLLightmap&);
  GLLightmap& operator=(const GLLightmap&);
};

#endif

/* EOF */
