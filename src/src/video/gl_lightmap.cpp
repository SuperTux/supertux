//  $Id: gl_lightmap.cpp 5063 2007-05-27 11:32:00Z matzeb $
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include <config.h>

#ifdef HAVE_OPENGL

#include <functional>
#include <algorithm>
#include <cassert>
#include <math.h>
#include <iostream>
#include <SDL_image.h>
#include <sstream>
#include <iomanip>
#include <physfs.h>

#include "glutil.hpp"
#include "gl_lightmap.hpp"
#include "gl_surface_data.hpp"
#include "drawing_context.hpp"
#include "drawing_request.hpp"
#include "renderer.hpp"
#include "surface.hpp"
#include "font.hpp"
#include "main.hpp"
#include "gameconfig.hpp"
#include "gl_texture.hpp"
#include "texture_manager.hpp"
#include "obstack/obstackpp.hpp"

namespace
{
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
    if(effect & VERTICAL_FLIP) {
      std::swap(uv_top, uv_bottom);
    }

    float center_x = (left + right) / 2;
    float center_y = (top + bottom) / 2;

    float sa = sinf(angle/180.0f*M_PI);
    float ca = cosf(angle/180.0f*M_PI);

    left  -= center_x;
    right -= center_x;

    top    -= center_y;
    bottom -= center_y;

    glBlendFunc(blend.sfactor, blend.dfactor);
    glColor4f(color.red, color.green, color.blue, color.alpha * alpha);
    glBegin(GL_QUADS);
    glTexCoord2f(uv_left, uv_top);
    glVertex2f(left*ca - top*sa + center_x,
               left*sa + top*ca + center_y);

    glTexCoord2f(uv_right, uv_top);
    glVertex2f(right*ca - top*sa + center_x,
               right*sa + top*ca + center_y);

    glTexCoord2f(uv_right, uv_bottom);
    glVertex2f(right*ca - bottom*sa + center_x,
               right*sa + bottom*ca + center_y);

    glTexCoord2f(uv_left, uv_bottom);
    glVertex2f(left*ca - bottom*sa + center_x,
               left*sa + bottom*ca + center_y);
    glEnd();

    // FIXME: find a better way to restore the blend mode
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
}

namespace GL
{
  static inline int next_po2(int val)
  {
    int result = 1;
    while(result < val)
      result *= 2;

    return result;
  }

  Lightmap::Lightmap()
  {
    screen = SDL_GetVideoSurface();

    lightmap_width = screen->w / LIGHTMAP_DIV;
    lightmap_height = screen->h / LIGHTMAP_DIV;
    unsigned int width = next_po2(lightmap_width);
    unsigned int height = next_po2(lightmap_height);

    lightmap = new Texture(width, height);

    lightmap_uv_right = static_cast<float>(lightmap_width) / static_cast<float>(width);
    lightmap_uv_bottom = static_cast<float>(lightmap_height) / static_cast<float>(height);
    texture_manager->register_texture(lightmap);
  }

  Lightmap::~Lightmap()
  {
    texture_manager->remove_texture(lightmap);
    delete lightmap;
  }

  void
  Lightmap::start_draw(const Color &ambient_color)
  {
    glViewport(0, screen->h - lightmap_height, lightmap_width, lightmap_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor( ambient_color.red, ambient_color.green, ambient_color.blue, 1 );
    glClear(GL_COLOR_BUFFER_BIT);
  }

  void
  Lightmap::end_draw()
  {
    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, lightmap->get_handle());
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, screen->h - lightmap_height, lightmap_width, lightmap_height);

    glViewport(0, 0, screen->w, screen->h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);
    //glClear(GL_COLOR_BUFFER_BIT);
  }

  void
  Lightmap::do_draw()
  {
    const Texture* texture = lightmap;

    // multiple the lightmap with the framebuffer
    glBlendFunc(GL_DST_COLOR, GL_ZERO);

    glBindTexture(GL_TEXTURE_2D, texture->get_handle());
    glBegin(GL_QUADS);

    glTexCoord2f(0, lightmap_uv_bottom);
    glVertex2f(0, 0);

    glTexCoord2f(lightmap_uv_right, lightmap_uv_bottom);
    glVertex2f(SCREEN_WIDTH, 0);

    glTexCoord2f(lightmap_uv_right, 0);
    glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);

    glTexCoord2f(0, 0);
    glVertex2f(0, SCREEN_HEIGHT);

    glEnd();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  void
  Lightmap::draw_surface(const DrawingRequest& request)
  {
    const Surface* surface = (const Surface*) request.request_data;
    GL::Texture *gltexture = dynamic_cast<GL::Texture *>(surface->get_texture());
    GL::SurfaceData *surface_data = reinterpret_cast<GL::SurfaceData *>(surface->get_surface_data());

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
  Lightmap::draw_surface_part(const DrawingRequest& request)
  {
    const SurfacePartRequest* surfacepartrequest
      = (SurfacePartRequest*) request.request_data;
    const Surface *surface = surfacepartrequest->surface;
    GL::Texture *gltexture = dynamic_cast<GL::Texture *>(surface->get_texture());
    GL::SurfaceData *surface_data = reinterpret_cast<GL::SurfaceData *>(surface->get_surface_data());

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
  Lightmap::draw_gradient(const DrawingRequest& request)
  {
    const GradientRequest* gradientrequest 
      = (GradientRequest*) request.request_data;
    const Color& top = gradientrequest->top;
    const Color& bottom = gradientrequest->bottom;

    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glColor4f(top.red, top.green, top.blue, top.alpha);
    glVertex2f(0, 0);
    glVertex2f(SCREEN_WIDTH, 0);
    glColor4f(bottom.red, bottom.green, bottom.blue, bottom.alpha);
    glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);
    glVertex2f(0, SCREEN_HEIGHT);
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glColor4f(1, 1, 1, 1);
  }

  void
  Lightmap::draw_filled_rect(const DrawingRequest& request)
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

    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x+w, y);
    glVertex2f(x+w, y+h);
    glVertex2f(x, y+h);
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glColor4f(1, 1, 1, 1);
  }

  void
  Lightmap::get_light(const DrawingRequest& request) const
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
    //printf("get_light %f/%f =>%f/%f r%f g%f b%f\n", request.pos.x, request.pos.y, posX, posY, pixels[0], pixels[1], pixels[2]);
  }
}

#endif
