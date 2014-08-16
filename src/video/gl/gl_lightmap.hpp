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

#include "video/lightmap.hpp"

struct DrawingRequest;

class Texture;

class GLLightmap : public Lightmap
{
public:
  GLLightmap();
  ~GLLightmap();

  void start_draw(const Color &ambient_color);
  void end_draw();
  void do_draw();
  void draw_surface(const DrawingRequest& request);
  void draw_surface_part(const DrawingRequest& request);
  void draw_gradient(const DrawingRequest& request);
  void draw_filled_rect(const DrawingRequest& request);
  void draw_inverse_ellipse(const DrawingRequest& request);
  void get_light(const DrawingRequest& request) const;

private:
  static const int LIGHTMAP_DIV = 5;

  boost::shared_ptr<GLTexture> lightmap;
  int lightmap_width;
  int lightmap_height;
  float lightmap_uv_right;
  float lightmap_uv_bottom;
  GLfloat old_viewport[4]; //holds vieport before redefining in start_draw - returned from glGet

private:
  GLLightmap(const GLLightmap&);
  GLLightmap& operator=(const GLLightmap&);
};

#endif

/* EOF */
