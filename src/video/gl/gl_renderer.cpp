//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//	Updated by GiBy 2013 for SDL2 <giby_the_kid@yahoo.fr>
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

#include "video/gl/gl_renderer.hpp"

#include <iomanip>
#include <iostream>
#include <physfs.h>
#include "SDL.h"

#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "video/drawing_request.hpp"
#include "video/gl/gl_painter.hpp"
#include "video/gl/gl_surface_data.hpp"
#include "video/gl/gl_texture.hpp"
#include "video/util.hpp"

#define LIGHTMAP_DIV 5

#ifdef GL_VERSION_ES_CM_1_0
#  define glOrtho glOrthof
#endif

GLRenderer::GLRenderer() :
  m_window(),
  m_glcontext(),
  m_viewport(),
  m_desktop_size(0, 0),
  m_fullscreen_active(false)
{
  SDL_DisplayMode mode;
  SDL_GetCurrentDisplayMode(0, &mode);
  m_desktop_size = Size(mode.w, mode.h);

  if(texture_manager != 0)
    texture_manager->save_textures();

  if(g_config->try_vsync) {
    /* we want vsync for smooth scrolling */
    if (SDL_GL_SetSwapInterval(-1) != 0)
    {
      log_info << "no support for late swap tearing vsync: " << SDL_GetError() << std::endl;
      if (SDL_GL_SetSwapInterval(1))
      {
        log_info << "no support for vsync: " << SDL_GetError() << std::endl;
      }
    }
  }

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   5);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  5);

  apply_video_mode();

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

#ifndef GL_VERSION_ES_CM_1_0
  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
    std::ostringstream out;
    out << "GLRenderer: " << glewGetErrorString(err);
    throw std::runtime_error(out.str());
  }
  log_info << "Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  log_info << "GLEW_ARB_texture_non_power_of_two: " << static_cast<int>(GLEW_ARB_texture_non_power_of_two) << std::endl;
#endif
}

GLRenderer::~GLRenderer()
{
  SDL_GL_DeleteContext(m_glcontext);
  SDL_DestroyWindow(m_window);
}

void
GLRenderer::do_take_screenshot()
{
  // [Christoph] TODO: Yes, this method also takes care of the actual disk I/O. Split it?

  SDL_Surface *shot_surf;
  // create surface to hold screenshot
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  shot_surf = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 24, 0x00FF0000, 0x0000FF00, 0x000000FF, 0);
#else
  shot_surf = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
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
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
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
GLRenderer::flip()
{
  assert_gl("drawing");
  SDL_GL_SwapWindow(m_window);
}

void
GLRenderer::resize(int w, int h)
{
  g_config->window_size = Size(w, h);

  apply_config();
}

void
GLRenderer::apply_config()
{
  apply_video_mode();

  Size target_size = g_config->use_fullscreen ?
    ((g_config->fullscreen_size == Size(0, 0)) ? m_desktop_size : g_config->fullscreen_size) :
    g_config->window_size;

  float pixel_aspect_ratio = 1.0f;
  if (g_config->aspect_size != Size(0, 0))
  {
    pixel_aspect_ratio = calculate_pixel_aspect_ratio(m_desktop_size,
                                                      g_config->aspect_size);
  }
  else if (g_config->use_fullscreen)
  {
    pixel_aspect_ratio = calculate_pixel_aspect_ratio(m_desktop_size,
                                                      target_size);
  }

  Size max_size(1280, 800);
  Size min_size(640, 480);

  Vector scale;
  Size logical_size;
  calculate_viewport(min_size, max_size, target_size,
                     pixel_aspect_ratio,
                     g_config->magnification,
                     scale,
                     logical_size,
                     m_viewport);

  SCREEN_WIDTH = logical_size.width;
  SCREEN_HEIGHT = logical_size.height;

  if (m_viewport.x != 0 || m_viewport.y != 0)
  {
    // Clear both buffers so that we get a clean black border without junk
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(m_window);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(m_window);
  }

  glViewport(m_viewport.x, m_viewport.y, m_viewport.w, m_viewport.h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0, 0, 0);
  check_gl_error("Setting up view matrices");
}

void
GLRenderer::apply_video_mode()
{
  if (m_window)
  {
    if (!g_config->use_fullscreen)
    {
      SDL_SetWindowFullscreen(m_window, 0);
    }
    else
    {
      if (g_config->fullscreen_size.width == 0 &&
          g_config->fullscreen_size.height == 0)
      {
        if (SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)
        {
          log_warning << "failed to switch to desktop fullscreen mode: "
                      << SDL_GetError() << std::endl;
        }
        else
        {
          log_info << "switched to desktop fullscreen mode" << std::endl;
        }
      }
      else
      {
        SDL_DisplayMode mode;
        mode.format = SDL_PIXELFORMAT_RGB888;
        mode.w = g_config->fullscreen_size.width;
        mode.h = g_config->fullscreen_size.height;
        mode.refresh_rate = g_config->fullscreen_refresh_rate;
        mode.driverdata = 0;

        if (SDL_SetWindowDisplayMode(m_window, &mode) != 0)
        {
          log_warning << "failed to set display mode: "
                      << mode.w << "x" << mode.h << "@" << mode.refresh_rate << ": "
                      << SDL_GetError() << std::endl;
        }
        else
        {
          if (SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN) != 0)
          {
            log_warning << "failed to switch to fullscreen mode: "
                        << mode.w << "x" << mode.h << "@" << mode.refresh_rate << ": "
                        << SDL_GetError() << std::endl;
          }
          else
          {
            log_info << "switched to fullscreen mode: "
                     << mode.w << "x" << mode.h << "@" << mode.refresh_rate << std::endl;
          }
        }
      }
    }
  }
  else
  {
    int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    Size size;
    if (g_config->use_fullscreen)
    {
      if (g_config->fullscreen_size == Size(0, 0))
      {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        size = m_desktop_size;
      }
      else
      {
        flags |= SDL_WINDOW_FULLSCREEN;
        size.width  = g_config->fullscreen_size.width;
        size.height = g_config->fullscreen_size.height;
      }
    }
    else
    {
      size = g_config->window_size;
    }

    m_window = SDL_CreateWindow("SuperTux",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              size.width, size.height,
                              flags);
    if (!m_window)
    {
      std::ostringstream msg;
      msg << "Couldn't set video mode " << size.width << "x" << size.height << ": " << SDL_GetError();
      throw std::runtime_error(msg.str());
    }
    else
    {
      m_glcontext = SDL_GL_CreateContext(m_window);

      SCREEN_WIDTH = size.width;
      SCREEN_HEIGHT = size.height;

      m_fullscreen_active = g_config->use_fullscreen;
    }
  }
}

void
GLRenderer::start_draw()
{
}

void
GLRenderer::end_draw()
{
}

void
GLRenderer::draw_surface(const DrawingRequest& request)
{
  GLPainter::draw_surface(request);
}

void
GLRenderer::draw_surface_part(const DrawingRequest& request)
{
  GLPainter::draw_surface_part(request);
}

void
GLRenderer::draw_gradient(const DrawingRequest& request)
{
  GLPainter::draw_gradient(request);
}

void
GLRenderer::draw_filled_rect(const DrawingRequest& request)
{
  GLPainter::draw_filled_rect(request);
}

void
GLRenderer::draw_inverse_ellipse(const DrawingRequest& request)
{
  GLPainter::draw_inverse_ellipse(request);
}

Vector
GLRenderer::to_logical(int physical_x, int physical_y)
{
  return Vector(static_cast<float>(physical_x - m_viewport.x) * SCREEN_WIDTH / m_viewport.w,
                static_cast<float>(physical_y - m_viewport.y) * SCREEN_HEIGHT / m_viewport.h);
}

void
GLRenderer::set_gamma(float gamma)
{
  Uint16 ramp[256];
  SDL_CalculateGammaRamp(gamma, ramp);
  SDL_SetWindowGammaRamp(m_window, ramp, ramp, ramp);
}

/* EOF */
