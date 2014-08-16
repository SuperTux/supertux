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

#include "video/gl/gl_lightmap.hpp"

#include <SDL_image.h>
#include <algorithm>
#include <assert.h>
#include <functional>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <physfs.h>
#include <sstream>

#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/obstackpp.hpp"
#include "video/drawing_context.hpp"
#include "video/drawing_request.hpp"
#include "video/font.hpp"
#include "video/gl/gl_painter.hpp"
#include "video/gl/gl_renderer.hpp"
#include "video/gl/gl_surface_data.hpp"
#include "video/gl/gl_texture.hpp"
#include "video/glutil.hpp"
#include "video/lightmap.hpp"
#include "video/renderer.hpp"
#include "video/surface.hpp"
#include "video/texture_manager.hpp"

inline int next_po2(int val)
{
  int result = 1;
  while(result < val)
    result *= 2;

  return result;
}

GLLightmap::GLLightmap() :
  lightmap(),
  lightmap_width(),
  lightmap_height(),
  lightmap_uv_right(),
  lightmap_uv_bottom()
{
  lightmap_width = SCREEN_WIDTH / LIGHTMAP_DIV;
  lightmap_height = SCREEN_HEIGHT / LIGHTMAP_DIV;
  unsigned int width = next_po2(lightmap_width);
  unsigned int height = next_po2(lightmap_height);

  lightmap.reset(new GLTexture(width, height));

  lightmap_uv_right = static_cast<float>(lightmap_width) / static_cast<float>(width);
  lightmap_uv_bottom = static_cast<float>(lightmap_height) / static_cast<float>(height);
  texture_manager->register_texture(lightmap.get());
}

GLLightmap::~GLLightmap()
{
}

void
GLLightmap::start_draw(const Color &ambient_color)
{

  glGetFloatv(GL_VIEWPORT, old_viewport); //save viewport
  glViewport(old_viewport[0], old_viewport[3] - lightmap_height + old_viewport[1], lightmap_width, lightmap_height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
#ifdef GL_VERSION_ES_CM_1_0
  glOrthof(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);
#else
  glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);
#endif
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor( ambient_color.red, ambient_color.green, ambient_color.blue, 1 );
  glClear(GL_COLOR_BUFFER_BIT);
}

void
GLLightmap::end_draw()
{
  glDisable(GL_BLEND);
  glBindTexture(GL_TEXTURE_2D, lightmap->get_handle());
  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, old_viewport[0], old_viewport[3]  - lightmap_height + old_viewport[1], lightmap_width, lightmap_height);

  glViewport(old_viewport[0], old_viewport[1], old_viewport[2], old_viewport[3]);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
#ifdef GL_VERSION_ES_CM_1_0
  glOrthof(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);
#else
  glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);
#endif
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_BLEND);

  glClearColor(0, 0, 0, 1 );
  glClear(GL_COLOR_BUFFER_BIT);
}

void
GLLightmap::do_draw()
{
  // multiple the lightmap with the framebuffer
  glBlendFunc(GL_DST_COLOR, GL_ZERO);

  glBindTexture(GL_TEXTURE_2D, lightmap->get_handle());

  float vertices[] = {
    0, 0,
    float(SCREEN_WIDTH), 0,
    float(SCREEN_WIDTH), float(SCREEN_HEIGHT),
    0, float(SCREEN_HEIGHT)
  };
  glVertexPointer(2, GL_FLOAT, 0, vertices);

  float uvs[] = {
    0,                 lightmap_uv_bottom,
    lightmap_uv_right, lightmap_uv_bottom,
    lightmap_uv_right, 0,
    0, 0
  };
  glTexCoordPointer(2, GL_FLOAT, 0, uvs);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void
GLLightmap::draw_surface(const DrawingRequest& request)
{
  GLPainter::draw_surface(request);
}

void
GLLightmap::draw_surface_part(const DrawingRequest& request)
{
  GLPainter::draw_surface_part(request);
}

void
GLLightmap::draw_gradient(const DrawingRequest& request)
{
  GLPainter::draw_gradient(request);
}

void
GLLightmap::draw_filled_rect(const DrawingRequest& request)
{
  GLPainter::draw_filled_rect(request);
}

void
GLLightmap::draw_inverse_ellipse(const DrawingRequest& request)
{
  GLPainter::draw_inverse_ellipse(request);
}

void
GLLightmap::get_light(const DrawingRequest& request) const
{
  const GetLightRequest* getlightrequest
    = (GetLightRequest*) request.request_data;

  float pixels[3];
  for( int i = 0; i<3; i++)
    pixels[i] = 0.0f; //set to black

  float posX = request.pos.x * lightmap_width / SCREEN_WIDTH + old_viewport[0];
  float posY = old_viewport[3] + old_viewport[1] - request.pos.y * lightmap_height / SCREEN_HEIGHT;
  glReadPixels((GLint) posX, (GLint) posY , 1, 1, GL_RGB, GL_FLOAT, pixels);
  *(getlightrequest->color_ptr) = Color( pixels[0], pixels[1], pixels[2]);
}

/* EOF */
