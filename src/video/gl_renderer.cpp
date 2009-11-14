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

#ifdef GL_VERSION_ES_CM_1_0
#  define glOrtho glOrthof
#endif

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

#ifdef SDL_GL_SWAP_CONTROL
  if(config->try_vsync) {
    /* we want vsync for smooth scrolling */
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
  }
#endif

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // FIXME: Hu? 16bit rendering?
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
//      flags |= SDL_RESIZABLE;
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
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Init the projection matrix, viewport and stuff
  apply_config();
  
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
              request.color,
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
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

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

  glDisableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glEnable(GL_TEXTURE_2D);
  glColor4f(1, 1, 1, 1);
}

void
Renderer::draw_filled_rect(const DrawingRequest& request)
{
  const FillRectRequest* fillrectrequest
    = (FillRectRequest*) request.request_data;

  glDisable(GL_TEXTURE_2D);
  glColor4f(fillrectrequest->color.red, fillrectrequest->color.green,
            fillrectrequest->color.blue, fillrectrequest->color.alpha);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  
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


      int n = 8;
      int p = 0;
      float vertices[(n+1) * 4 * 2];

      for(int i = 0; i <= n; ++i)
        {
          float x = sinf(i * (M_PI/2) / n) * radius;
          float y = cosf(i * (M_PI/2) / n) * radius;

          vertices[p++] = irect.get_left() - x;
          vertices[p++] = irect.get_top()  - y;

          vertices[p++] = irect.get_right() + x;
          vertices[p++] = irect.get_top()   - y;
        }

      for(int i = 0; i <= n; ++i)
        {
          float x = cosf(i * (M_PI/2) / n) * radius;
          float y = sinf(i * (M_PI/2) / n) * radius;

          vertices[p++] = irect.get_left()   - x;
          vertices[p++] = irect.get_bottom() + y;

          vertices[p++] = irect.get_right()  + x;
          vertices[p++] = irect.get_bottom() + y;
        }

      glVertexPointer(2, GL_FLOAT, 0, vertices);
      glDrawArrays(GL_TRIANGLE_STRIP, 0,  sizeof(vertices)/sizeof(float)/2);
    }
  else
    {
      float x = request.pos.x;
      float y = request.pos.y;
      float w = fillrectrequest->size.x;
      float h = fillrectrequest->size.y;

      float vertices[] = {
        x,   y,
        x+w, y,
        x+w, y+h,
        x,   y+h
      };
      glVertexPointer(2, GL_FLOAT, 0, vertices);

      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_TEXTURE_2D);
  glColor4f(1, 1, 1, 1);
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

  static const int slices = 16;
  static const int points = (slices+1) * 12;

  float vertices[points * 2];
  int   p = 0;

  // Bottom
  vertices[p++] = SCREEN_WIDTH; vertices[p++] = SCREEN_HEIGHT;
  vertices[p++] = 0;            vertices[p++] = SCREEN_HEIGHT;
  vertices[p++] = x;            vertices[p++] = y+h;

  // Top
  vertices[p++] = SCREEN_WIDTH; vertices[p++] = 0;
  vertices[p++] = 0;            vertices[p++] = 0;
  vertices[p++] = x;            vertices[p++] = y-h;

  // Left
  vertices[p++] = SCREEN_WIDTH; vertices[p++] = 0;
  vertices[p++] = SCREEN_WIDTH; vertices[p++] = SCREEN_HEIGHT;
  vertices[p++] = x+w;          vertices[p++] = y;

  // Right
  vertices[p++] = 0;            vertices[p++] = 0;
  vertices[p++] = 0;            vertices[p++] = SCREEN_HEIGHT;
  vertices[p++] = x-w;          vertices[p++] = y;

  for(int i = 0; i < slices; ++i)
    {
      float ex1 = sinf(M_PI/2 / slices * i) * w;
      float ey1 = cosf(M_PI/2 / slices * i) * h;

      float ex2 = sinf(M_PI/2 / slices * (i+1)) * w;
      float ey2 = cosf(M_PI/2 / slices * (i+1)) * h;

      // Bottom/Right
      vertices[p++] = SCREEN_WIDTH; vertices[p++] = SCREEN_HEIGHT;
      vertices[p++] = x + ex1;      vertices[p++] = y + ey1;
      vertices[p++] = x + ex2;      vertices[p++] = y + ey2;

      // Top/Left
      vertices[p++] = 0;            vertices[p++] = 0;
      vertices[p++] = x - ex1;      vertices[p++] = y - ey1;
      vertices[p++] = x - ex2;      vertices[p++] = y - ey2;

      // Top/Right
      vertices[p++] = SCREEN_WIDTH; vertices[p++] = 0;
      vertices[p++] = x + ex1;      vertices[p++] = y - ey1;
      vertices[p++] = x + ex2;      vertices[p++] = y - ey2;

      // Bottom/Left
      vertices[p++] = 0;            vertices[p++] = SCREEN_HEIGHT;
      vertices[p++] = x - ex1;      vertices[p++] = y + ey1;
      vertices[p++] = x - ex2;      vertices[p++] = y + ey2;
    }

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, vertices);

  glDrawArrays(GL_TRIANGLES, 0, points);

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

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
  // This causes the screen to go black, which is annoying, but seems
  // unavoidable with SDL at the moment
  SDL_SetVideoMode(w, h, 0, SDL_OPENGL /*| SDL_RESIZABLE*/);

  config->window_width  = w;
  config->window_height = h;

  apply_config();
}

void
Renderer::apply_config()
{    
  if (0)
    {
      std::cout << "Applying Config:" 
                << "\n  Desktop: " << desktop_width << "x" << desktop_height
                << "\n  Window:  " << config->window_width << "x" << config->window_height
                << "\n  FullRes: " << config->fullscreen_width << "x" << config->fullscreen_height
                << "\n  Aspect:  " << config->aspect_width << ":" << config->aspect_height
                << "\n  Magnif:  " << config->magnification
                << std::endl;
    }

  int w,h;
  float target_aspect = float(desktop_width) / desktop_height;
  
  if (config->aspect_width != 0 && config->aspect_height != 0)
    target_aspect = float(config->aspect_width) / float(config->aspect_height);

  float desktop_aspect = 4.0f / 3.0f; // random default fallback guess
  
  if (desktop_width != -1 && desktop_height != -1)
    {
      desktop_aspect = float(desktop_width) / float(desktop_height);
    }

  // Get the screen width
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

  if (target_aspect > 1.0f)
    {
      SCREEN_WIDTH  = static_cast<int>(w * (target_aspect / desktop_aspect));
      SCREEN_HEIGHT = static_cast<int>(h);
    }
  else
    {
      SCREEN_WIDTH  = static_cast<int>(w);
      SCREEN_HEIGHT = static_cast<int>(h  * (target_aspect / desktop_aspect));
    }

  int max_width  = 1600; // FIXME: Maybe 1920 is ok too
  int max_height = 1200;

  if (config->magnification == 0.0f) // Magic value that means 'minfill'
    {
      // This scales SCREEN_WIDTH/SCREEN_HEIGHT so that they never excede
      // max_width/max_height
      if (SCREEN_WIDTH > max_width || SCREEN_HEIGHT > max_height)
        {
          float scale1  = float(max_width)/SCREEN_WIDTH;
          float scale2  = float(max_height)/SCREEN_HEIGHT;
          float scale   = (scale1 < scale2) ? scale1 : scale2;
          SCREEN_WIDTH  = static_cast<int>(SCREEN_WIDTH  * scale);
          SCREEN_HEIGHT = static_cast<int>(SCREEN_HEIGHT * scale);
        }

      glViewport(0, 0, w, h);
    }
  else
    {
      SCREEN_WIDTH  = static_cast<int>(SCREEN_WIDTH  / config->magnification);
      SCREEN_HEIGHT = static_cast<int>(SCREEN_HEIGHT / config->magnification);

      // This works by adding black borders around the screen to limit
      // SCREEN_WIDTH/SCREEN_HEIGHT to max_width/max_height
      int nw = w;
      int nh = h;

      if (SCREEN_WIDTH > max_width)
        {
          nw = static_cast<int>((float) nw * float(max_width)/SCREEN_WIDTH);
          SCREEN_WIDTH = static_cast<int>(max_width);
        }

      if (SCREEN_HEIGHT > max_height)
        {
          nh = static_cast<int>((float) nh * float(max_height)/SCREEN_HEIGHT);
          SCREEN_HEIGHT = static_cast<int>(max_height);
        }

      // Clear both buffers so that we get a clean black border without junk
      glClear(GL_COLOR_BUFFER_BIT);
      SDL_GL_SwapBuffers();
      glClear(GL_COLOR_BUFFER_BIT);
      SDL_GL_SwapBuffers();

      if (0)
        std::cout << (w-nw)/2 << " "
                  << (h-nh)/2 << " "
                  << nw << "x" << nh << std::endl;

      glViewport(std::max(0, (w-nw)/2), 
                 std::max(0, (h-nh)/2), 
                 std::min(nw, w),
                 std::min(nh, h));
    }

  if (0)
    std::cout << "  -> " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << std::endl;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0, 0, 0);
  check_gl_error("Setting up view matrices");
}

} // namespace GL

#endif
