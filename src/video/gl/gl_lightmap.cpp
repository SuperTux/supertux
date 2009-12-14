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
#include "video/gl/gl_surface_data.hpp"
#include "video/gl/gl_texture.hpp"
#include "video/glutil.hpp"
#include "video/lightmap.hpp"
#include "video/renderer.hpp"
#include "video/surface.hpp"
#include "video/texture_manager.hpp"

namespace {

inline void intern_draw(float left, float top, float right, float bottom,
                        float uv_left, float uv_top,
                        float uv_right, float uv_bottom,
                        float angle, float alpha,
                        const Color& color,
                        const Blend& blend,
                        DrawingEffect effect)
{
  if(effect & HORIZONTAL_FLIP)
    std::swap(uv_left, uv_right);
 
  if(effect & VERTICAL_FLIP) 
    std::swap(uv_top, uv_bottom);

  // unrotated blit
  glBlendFunc(blend.sfactor, blend.dfactor);
  glColor4f(color.red, color.green, color.blue, color.alpha * alpha);
 
  if (angle == 0.0f) {
    float vertices[] = {
      left, top,
      right, top,
      right, bottom,
      left, bottom,
    };
    glVertexPointer(2, GL_FLOAT, 0, vertices);

    float uvs[] = {
      uv_left, uv_top,
      uv_right, uv_top,
      uv_right, uv_bottom,
      uv_left, uv_bottom,
    };
    glTexCoordPointer(2, GL_FLOAT, 0, uvs);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  } else {
    // rotated blit
    float center_x = (left + right) / 2;
    float center_y = (top + bottom) / 2;

    float sa = sinf(angle/180.0f*M_PI);
    float ca = cosf(angle/180.0f*M_PI);

    left  -= center_x;
    right -= center_x;

    top    -= center_y;
    bottom -= center_y;

    float vertices[] = {
      left*ca - top*sa + center_x, left*sa + top*ca + center_y,
      right*ca - top*sa + center_x, right*sa + top*ca + center_y,
      right*ca - bottom*sa + center_x, right*sa + bottom*ca + center_y,
      left*ca - bottom*sa + center_x, left*sa + bottom*ca + center_y
    };
    glVertexPointer(2, GL_FLOAT, 0, vertices);

    float uvs[] = {
      uv_left, uv_top,
      uv_right, uv_top,
      uv_right, uv_bottom,
      uv_left, uv_bottom,
    };
    glTexCoordPointer(2, GL_FLOAT, 0, uvs);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }

  // FIXME: find a better way to restore the blend mode
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static inline int next_po2(int val)
{
  int result = 1;
  while(result < val)
    result *= 2;

  return result;
}

} // namespace

GLLightmap::GLLightmap() :
  screen(),
  lightmap(),
  lightmap_width(),
  lightmap_height(),
  lightmap_uv_right(),
  lightmap_uv_bottom()
{
  screen = SDL_GetVideoSurface();

  lightmap_width = screen->w / LIGHTMAP_DIV;
  lightmap_height = screen->h / LIGHTMAP_DIV;
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
  glViewport(0, screen->h - lightmap_height, lightmap_width, lightmap_height);
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
  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, screen->h - lightmap_height, lightmap_width, lightmap_height);

  glViewport(0, 0, screen->w, screen->h);
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
  //glClear(GL_COLOR_BUFFER_BIT);
}

void
GLLightmap::do_draw()
{
  // multiple the lightmap with the framebuffer
  glBlendFunc(GL_DST_COLOR, GL_ZERO);

  glBindTexture(GL_TEXTURE_2D, lightmap->get_handle());

  float vertices[] = {
    0, 0,
    SCREEN_WIDTH, 0,
    SCREEN_WIDTH, SCREEN_HEIGHT,
    0, SCREEN_HEIGHT
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
  const Surface* surface = (const Surface*) request.request_data;
  boost::shared_ptr<GLTexture> gltexture = boost::dynamic_pointer_cast<GLTexture>(surface->get_texture());
  GLSurfaceData *surface_data = reinterpret_cast<GLSurfaceData *>(surface->get_surface_data());

  glBindTexture(GL_TEXTURE_2D, gltexture->get_handle());
  intern_draw(request.pos.x, request.pos.y,
              request.pos.x + surface->get_width(),
              request.pos.y + surface->get_height(),
              surface_data->get_uv_left(),
              surface_data->get_uv_top(),
              surface_data->get_uv_right(),
              surface_data->get_uv_bottom(),
              request.angle,
              request.alpha,
              request.color,
              request.blend,
              request.drawing_effect);
}

void
GLLightmap::draw_surface_part(const DrawingRequest& request)
{
  const SurfacePartRequest* surfacepartrequest
    = (SurfacePartRequest*) request.request_data;
  const Surface* surface = surfacepartrequest->surface;
  boost::shared_ptr<GLTexture> gltexture = boost::dynamic_pointer_cast<GLTexture>(surface->get_texture());
  GLSurfaceData *surface_data = reinterpret_cast<GLSurfaceData *>(surface->get_surface_data());

  float uv_width = surface_data->get_uv_right() - surface_data->get_uv_left();
  float uv_height = surface_data->get_uv_bottom() - surface_data->get_uv_top();

  float uv_left = surface_data->get_uv_left() + (uv_width * surfacepartrequest->source.x) / surface->get_width();
  float uv_top = surface_data->get_uv_top() + (uv_height * surfacepartrequest->source.y) / surface->get_height();
  float uv_right = surface_data->get_uv_left() + (uv_width * (surfacepartrequest->source.x + surfacepartrequest->size.x)) / surface->get_width();
  float uv_bottom = surface_data->get_uv_top() + (uv_height * (surfacepartrequest->source.y + surfacepartrequest->size.y)) / surface->get_height();

  glBindTexture(GL_TEXTURE_2D, gltexture->get_handle());
  intern_draw(request.pos.x, request.pos.y,
              request.pos.x + surfacepartrequest->size.x,
              request.pos.y + surfacepartrequest->size.y,
              uv_left,
              uv_top,
              uv_right,
              uv_bottom,
              0.0,
              request.alpha,
              Color(1.0, 1.0, 1.0),
              Blend(),
              request.drawing_effect);
}

void
GLLightmap::draw_gradient(const DrawingRequest& request)
{
  const GradientRequest* gradientrequest 
    = (GradientRequest*) request.request_data;
  const Color& top = gradientrequest->top;
  const Color& bottom = gradientrequest->bottom;

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_COLOR_ARRAY);

  float vertices[] = {
    0, 0,
    SCREEN_WIDTH, 0,
    SCREEN_WIDTH, SCREEN_HEIGHT,
    0, SCREEN_HEIGHT
  };
  glVertexPointer(2, GL_FLOAT, 0, vertices);

  float colors[] = {
    top.red, top.green, top.blue, top.alpha,
    top.red, top.green, top.blue, top.alpha,
    bottom.red, bottom.green, bottom.blue, bottom.alpha,
    bottom.red, bottom.green, bottom.blue, bottom.alpha,
  };
  glColorPointer(4, GL_FLOAT, 0, colors);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glDisable(GL_COLOR_ARRAY);
  glEnable(GL_TEXTURE_COORD_ARRAY);

  glEnable(GL_TEXTURE_2D);
  glColor4f(1, 1, 1, 1);
}

void
GLLightmap::draw_filled_rect(const DrawingRequest& request)
{
  const FillRectRequest* fillrectrequest
    = (FillRectRequest*) request.request_data;

  float x = request.pos.x;
  float y = request.pos.y;
  float w = fillrectrequest->size.x;
  float h = fillrectrequest->size.y;

  glDisable(GL_TEXTURE_2D);
  glColor4f(fillrectrequest->color.red, fillrectrequest->color.green,
            fillrectrequest->color.blue, fillrectrequest->color.alpha);
  glDisable(GL_TEXTURE_COORD_ARRAY);

  float vertices[] = {
    x,   y,
    x+w, y,
    x+w, y+h,
    x,   y+h
  };
  glVertexPointer(2, GL_FLOAT, 0, vertices);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glEnable(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_TEXTURE_2D);
  glColor4f(1, 1, 1, 1);
}

void
GLLightmap::get_light(const DrawingRequest& request) const
{
  const GetLightRequest* getlightrequest 
    = (GetLightRequest*) request.request_data;

  float pixels[3];
  for( int i = 0; i<3; i++)
    pixels[i] = 0.0f; //set to black

  float posX = request.pos.x * lightmap_width / SCREEN_WIDTH;
  float posY = screen->h - request.pos.y * lightmap_height / SCREEN_HEIGHT;
  glReadPixels((GLint) posX, (GLint) posY , 1, 1, GL_RGB, GL_FLOAT, pixels);
  *(getlightrequest->color_ptr) = Color( pixels[0], pixels[1], pixels[2]);
}

/* EOF */
