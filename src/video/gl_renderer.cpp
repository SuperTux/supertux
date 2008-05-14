//  $Id: gl_renderer.cpp 5063 2007-05-27 11:32:00Z matzeb $
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
#include "gl_renderer.hpp"
#include "gl_texture.hpp"
#include "gl_surface_data.hpp"
#include "drawing_context.hpp"
#include "drawing_request.hpp"
#include "surface.hpp"
#include "font.hpp"
#include "main.hpp"
#include "gameconfig.hpp"
#include "texture.hpp"
#include "texture_manager.hpp"
#include "obstack/obstackpp.hpp"
#define LIGHTMAP_DIV 5

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
 
  if(effect & VERTICAL_FLIP) 
    std::swap(uv_top, uv_bottom);
 
  if (angle == 0.0f)
    { // unrotated blit
      glBlendFunc(blend.sfactor, blend.dfactor);
      glColor4f(color.red, color.green, color.blue, color.alpha * alpha);
      glBegin(GL_QUADS);
      glTexCoord2f(uv_left, uv_top);
      glVertex2f(left, top);

      glTexCoord2f(uv_right, uv_top);
      glVertex2f(right, top);

      glTexCoord2f(uv_right, uv_bottom);
      glVertex2f(right, bottom);

      glTexCoord2f(uv_left, uv_bottom);
      glVertex2f(left, bottom);
      glEnd();
    }
  else
    { // rotated blit
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
    }

  // FIXME: find a better way to restore the blend mode
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
}

namespace GL {

Renderer::Renderer()
  : desktop_width(-1),
    desktop_height(-1)
{
  ::Renderer::instance_ = this;

#if SDL_MAJOR_VERSION > 1 || SDL_MINOR_VERSION > 2 || (SDL_MINOR_VERSION == 2 && SDL_PATCHLEVEL >= 10)
  // unfortunately only newer SDLs have these infos.
  // This must be called before SDL_SetVideoMode() or it will return
  // the window size instead of the desktop size.
  const SDL_VideoInfo *info = SDL_GetVideoInfo();
  if (info)
    {
      desktop_width  = info->current_w;
      desktop_height = info->current_h;     
    }
#endif

  if(texture_manager != 0)
    texture_manager->save_textures();

  if(config->try_vsync) {
    /* we want vsync for smooth scrolling */
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
  }

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // Hu? 16bit rendering?
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   5);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  5);

  int flags = SDL_OPENGL;
  int width;
  int height;
  if(config->use_fullscreen)
    {
      flags |= SDL_FULLSCREEN;
      width  = config->fullscreen_width;
      height = config->fullscreen_height;
    }
  else
    {
      flags |= SDL_RESIZABLE;
      width  = config->window_width;
      height = config->window_height;
    }
  int bpp = 0;

  SDL_Surface *screen = SDL_SetVideoMode(width, height, bpp, flags);
  if(screen == 0) {
    std::stringstream msg;
    msg << "Couldn't set video mode (" << width << "x" << height
        << "-" << bpp << "bpp): " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }

  // setup opengl state and transform
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport(0, 0, screen->w, screen->h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // logical resolution here not real monitor resolution
  glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0, 0, 0);

  check_gl_error("Setting up view matrices");


  if(texture_manager == 0)
    texture_manager = new TextureManager();
  else
    texture_manager->reload_textures();
}

Renderer::~Renderer()
{
}

void
Renderer::draw_surface(const DrawingRequest& request)
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
Renderer::draw_surface_part(const DrawingRequest& request)
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
Renderer::draw_gradient(const DrawingRequest& request)
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
Renderer::draw_filled_rect(const DrawingRequest& request)
{
  const FillRectRequest* fillrectrequest
    = (FillRectRequest*) request.request_data;

  if (fillrectrequest->radius != 0.0f)
    {
      // draw round rect
      // Keep radius in the limits, so that we get a circle instead of
      // just graphic junk
      float radius = std::min(fillrectrequest->radius,
                              std::min(fillrectrequest->size.x/2,
                                       fillrectrequest->size.y/2));

      // inner rectangle
      Rect irect(request.pos.x    + radius,
                 request.pos.y    + radius,
                 request.pos.x + fillrectrequest->size.x - radius,
                 request.pos.y + fillrectrequest->size.y - radius);

      glDisable(GL_TEXTURE_2D);
      glColor4f(fillrectrequest->color.red, fillrectrequest->color.green,
                fillrectrequest->color.blue, fillrectrequest->color.alpha);


      int n = 8;
      glBegin(GL_QUAD_STRIP);
      for(int i = 0; i <= n; ++i)
        {
          float x = sinf(i * (M_PI/2) / n) * radius;
          float y = cosf(i * (M_PI/2) / n) * radius;

          glVertex2f(irect.get_left()  - x, irect.get_top() - y);
          glVertex2f(irect.get_right() + x, irect.get_top() - y);
        }
      for(int i = 0; i <= n; ++i)
        {
          float x = cosf(i * (M_PI/2) / n) * radius;
          float y = sinf(i * (M_PI/2) / n) * radius;

          glVertex2f(irect.get_left()  - x, irect.get_bottom() + y);
          glVertex2f(irect.get_right() + x, irect.get_bottom() + y);
        }
      glEnd();
      glEnable(GL_TEXTURE_2D);
      glColor4f(1, 1, 1, 1);
    }
  else
    {
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
}

void
Renderer::draw_inverse_ellipse(const DrawingRequest& request)
{
  const InverseEllipseRequest* ellipse = (InverseEllipseRequest*)request.request_data;

  glDisable(GL_TEXTURE_2D);
  glColor4f(ellipse->color.red,  ellipse->color.green,
            ellipse->color.blue, ellipse->color.alpha);
    
  float x = request.pos.x;
  float y = request.pos.y;
  float w = ellipse->size.x/2.0f;
  float h = ellipse->size.y/2.0f;

  glBegin(GL_TRIANGLES);
    
  // Bottom
  glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);
  glVertex2f(0, SCREEN_HEIGHT);
  glVertex2f(x, y+h);

  // Top
  glVertex2f(SCREEN_WIDTH, 0);
  glVertex2f(0, 0);
  glVertex2f(x, y-h);

  // Left
  glVertex2f(SCREEN_WIDTH, 0);
  glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);
  glVertex2f(x+w, y);

  // Right
  glVertex2f(0, 0);
  glVertex2f(0, SCREEN_HEIGHT);
  glVertex2f(x-w, y);

  glEnd();
        
  int slices = 16;
  for(int i = 0; i < slices; ++i)
    {
      float ex1 = sinf(M_PI/2 / slices * i) * w;
      float ey1 = cosf(M_PI/2 / slices * i) * h;

      float ex2 = sinf(M_PI/2 / slices * (i+1)) * w;
      float ey2 = cosf(M_PI/2 / slices * (i+1)) * h;

      glBegin(GL_TRIANGLES);

      // Bottom/Right
      glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);
      glVertex2f(x + ex1, y + ey1);
      glVertex2f(x + ex2, y + ey2);

      // Top/Left
      glVertex2f(0, 0);
      glVertex2f(x - ex1, y - ey1);
      glVertex2f(x - ex2, y - ey2);

      // Top/Right
      glVertex2f(SCREEN_WIDTH, 0);
      glVertex2f(x + ex1, y - ey1);
      glVertex2f(x + ex2, y - ey2);

      // Bottom/Left
      glVertex2f(0, SCREEN_HEIGHT);
      glVertex2f(x - ex1, y + ey1);
      glVertex2f(x - ex2, y + ey2);

      glEnd();
    }
  glEnable(GL_TEXTURE_2D);
  glColor4f(1, 1, 1, 1);    
}

void 
Renderer::do_take_screenshot()
{
  // [Christoph] TODO: Yes, this method also takes care of the actual disk I/O. Split it?

  SDL_Surface *shot_surf;
  // create surface to hold screenshot
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  shot_surf = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, 24, 0x00FF0000, 0x0000FF00, 0x000000FF, 0);
#else
  shot_surf = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
#endif
  if (!shot_surf) {
    log_warning << "Could not create RGB Surface to contain screenshot" << std::endl;
    return;
  }

  // read pixels into array
  char* pixels = new char[3 * SCREEN_WIDTH * SCREEN_HEIGHT];
  if (!pixels) {
    log_warning << "Could not allocate memory to store screenshot" << std::endl;
    SDL_FreeSurface(shot_surf);
    return;
  }
  glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels);

  // copy array line-by-line
  for (int i = 0; i < SCREEN_HEIGHT; i++) {
    char* src = pixels + (3 * SCREEN_WIDTH * (SCREEN_HEIGHT - i - 1));
    if(SDL_MUSTLOCK(shot_surf))
      {
        SDL_LockSurface(shot_surf);
      }
    char* dst = ((char*)shot_surf->pixels) + i * shot_surf->pitch;
    memcpy(dst, src, 3 * SCREEN_WIDTH);
    if(SDL_MUSTLOCK(shot_surf))
      {
        SDL_UnlockSurface(shot_surf);
      }
  }

  // free array
  delete[](pixels);

  // save screenshot
  static const std::string writeDir = PHYSFS_getWriteDir();
  static const std::string dirSep = PHYSFS_getDirSeparator();
  static const std::string baseName = "screenshot";
  static const std::string fileExt = ".bmp";
  std::string fullFilename;
  for (int num = 0; num < 1000; num++) {
    std::ostringstream oss;
    oss << baseName;
    oss << std::setw(3) << std::setfill('0') << num;
    oss << fileExt;
    std::string fileName = oss.str();
    fullFilename = writeDir + dirSep + fileName;
    if (!PHYSFS_exists(fileName.c_str())) {
      SDL_SaveBMP(shot_surf, fullFilename.c_str());
      log_debug << "Wrote screenshot to \"" << fullFilename << "\"" << std::endl;
      SDL_FreeSurface(shot_surf);
      return;
    }
  }
  log_warning << "Did not save screenshot, because all files up to \"" << fullFilename << "\" already existed" << std::endl;
  SDL_FreeSurface(shot_surf);
}

void
Renderer::flip()
{
  assert_gl("drawing");
  SDL_GL_SwapBuffers();
}

void
Renderer::resize(int w, int h)
{
  SDL_SetVideoMode(w, h, 0, SDL_OPENGL | SDL_RESIZABLE);

  config->window_width  = w;
  config->window_height = h;

  apply_config();
}

void
Renderer::apply_config()
{    
  std::cout << "Applying Config: " << config->aspect_width << ":" << config->aspect_height << std::endl;

  int w,h;
  float target_aspect  = float(config->aspect_width) / float(config->aspect_height);
  float desktop_aspect = 4.0f / 3.0f; // random default fallback guess
  
  if (desktop_width != -1 && desktop_height != -1)
    {
      desktop_aspect = float(desktop_width) / float(desktop_height);
    }

  if (config->use_fullscreen)
    {
      w = config->fullscreen_width;
      h = config->fullscreen_height;
      desktop_aspect = float(w) / float(h);
    }
  else
    {
      w = config->window_width;        
      h = config->window_height;
    }

  std::cout 
    << " -> " << w << "x" << h << " -> " << target_aspect << " " << desktop_aspect << std::endl;

  if (target_aspect > 1.0f)
    {
      SCREEN_WIDTH  = w * (target_aspect / desktop_aspect);
      SCREEN_HEIGHT = h;
    }
  else
    {
      SCREEN_WIDTH  = w;
      SCREEN_HEIGHT = h  * (target_aspect / desktop_aspect);
    }

  // Limit must take aspect into account
  //SCREEN_WIDTH  = std::min(w, 1024);
  //SCREEN_HEIGHT = std::min(h, 768);

  // Position the Viewport in the center of the window
  glViewport(std::max(0, (config->window_width  - w)/2),
             std::max(0, (config->window_height - h)/2),
             w, h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);    
}

} // namespace GL

#endif
