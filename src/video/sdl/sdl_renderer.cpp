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

#include "video/sdl/sdl_renderer.hpp"

#include "util/log.hpp"
#include "video/drawing_request.hpp"
#include "video/sdl/sdl_surface_data.hpp"
#include "video/sdl/sdl_texture.hpp"
#include "video/sdl/sdl_painter.hpp"

#include <iomanip>
#include <iostream>
#include <physfs.h>
#include <sstream>
#include <stdexcept>
#include <SDL_video.h>

#include "video/util.hpp"

SDLRenderer::SDLRenderer() :
  m_window(),
  m_renderer(),
  m_viewport(),
  m_desktop_size(0, 0),
  m_scale(1.0f, 1.0f)
{
  SDL_DisplayMode mode;
  if (SDL_GetDesktopDisplayMode(0, &mode) != 0)
  {
    log_warning << "Couldn't get desktop display mode: " << SDL_GetError() << std::endl;
  }
  else
  {
    m_desktop_size = Size(mode.w, mode.h);
  }

  log_info << "creating SDLRenderer" << std::endl;
  int width  = g_config->window_size.width;
  int height = g_config->window_size.height;

  int flags = SDL_WINDOW_RESIZABLE;
  if(g_config->use_fullscreen)
  {
    if (g_config->fullscreen_size == Size(0, 0))
    {
      flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
      width = g_config->window_size.width;
      height = g_config->window_size.height;
    }
    else
    {
      flags |= SDL_WINDOW_FULLSCREEN;
      width  = g_config->fullscreen_size.width;
      height = g_config->fullscreen_size.height;
    }
  }

  SCREEN_WIDTH = width;
  SCREEN_HEIGHT = height;

  m_viewport.x = 0;
  m_viewport.y = 0;
  m_viewport.w = width;
  m_viewport.h = height;

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

  int ret = SDL_CreateWindowAndRenderer(width, height, flags,
                                        &m_window, &m_renderer);

  if(ret != 0) {
    std::stringstream msg;
    msg << "Couldn't set video mode (" << width << "x" << height
        << "): " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }

  SDL_RendererInfo info;
  if (SDL_GetRendererInfo(m_renderer, &info) != 0)
  {
    log_warning << "Couldn't get RendererInfo: " << SDL_GetError() << std::endl;
  }
  else
  {
    log_info << "SDL_Renderer: " << info.name << std::endl;
    log_info << "SDL_RendererFlags: " << std::endl;
    if (info.flags & SDL_RENDERER_SOFTWARE) { log_info << "  SDL_RENDERER_SOFTWARE" << std::endl; }
    if (info.flags & SDL_RENDERER_ACCELERATED) { log_info << "  SDL_RENDERER_ACCELERATED" << std::endl; }
    if (info.flags & SDL_RENDERER_PRESENTVSYNC) { log_info << "  SDL_RENDERER_PRESENTVSYNC" << std::endl; }
    if (info.flags & SDL_RENDERER_TARGETTEXTURE) { log_info << "  SDL_RENDERER_TARGETTEXTURE" << std::endl; }
    log_info << "Texture Formats: " << std::endl;
    for(size_t i = 0; i < info.num_texture_formats; ++i)
    {
      log_info << "  " << SDL_GetPixelFormatName(info.texture_formats[i]) << std::endl;
    }
    log_info << "Max Texture Width: " << info.max_texture_width << std::endl;
    log_info << "Max Texture Height: " << info.max_texture_height << std::endl;
  }

  g_config->window_size = Size(width, height);
  apply_config();
}

SDLRenderer::~SDLRenderer()
{
  SDL_DestroyRenderer(m_renderer);
  SDL_DestroyWindow(m_window);
}

void
SDLRenderer::start_draw()
{
  SDL_RenderSetScale(m_renderer, m_scale.x, m_scale.y);
}

void
SDLRenderer::end_draw()
{
}

void
SDLRenderer::draw_surface(const DrawingRequest& request)
{
  SDLPainter::draw_surface(m_renderer, request);
}

void
SDLRenderer::draw_surface_part(const DrawingRequest& request)
{
  SDLPainter::draw_surface_part(m_renderer, request);
}

void
SDLRenderer::draw_gradient(const DrawingRequest& request)
{
  SDLPainter::draw_gradient(m_renderer, request);
}

void
SDLRenderer::draw_filled_rect(const DrawingRequest& request)
{
  SDLPainter::draw_filled_rect(m_renderer, request);
}

void
SDLRenderer::draw_inverse_ellipse(const DrawingRequest& request)
{
  SDLPainter::draw_inverse_ellipse(m_renderer, request);
}

void
SDLRenderer::draw_text(const DrawingRequest& request)
{
  SDLPainter::draw_text(m_renderer, request);
}

void
SDLRenderer::draw_line(const DrawingRequest& request)
{
  SDLPainter::draw_line(m_renderer, request);
}

void
SDLRenderer::draw_triangle(const DrawingRequest& request)
{
  SDLPainter::draw_triangle(m_renderer, request);
}

void
SDLRenderer::do_take_screenshot()
{
  // [Christoph] TODO: Yes, this method also takes care of the actual disk I/O. Split it?
  int width;
  int height;
  if (SDL_GetRendererOutputSize(m_renderer, &width, &height) != 0)
  {
    log_warning << "SDL_GetRenderOutputSize failed: " << SDL_GetError() << std::endl;
  }
  else
  {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    Uint32 rmask = 0xff000000;
    Uint32 gmask = 0x00ff0000;
    Uint32 bmask = 0x0000ff00;
    Uint32 amask = 0x000000ff;
#else
    Uint32 rmask = 0x000000ff;
    Uint32 gmask = 0x0000ff00;
    Uint32 bmask = 0x00ff0000;
    Uint32 amask = 0xff000000;
#endif
    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32,
                                                rmask, gmask, bmask, amask);
    if (!surface)
    {
      log_warning << "SDL_CreateRGBSurface failed: " << SDL_GetError() << std::endl;
    }
    else
    {
      int ret = SDL_RenderReadPixels(m_renderer, NULL,
                                     SDL_PIXELFORMAT_ABGR8888,
                                     surface->pixels,
                                     surface->pitch);
      if (ret != 0)
      {
        log_warning << "SDL_RenderReadPixels failed: " << SDL_GetError() << std::endl;
      }
      else
      {
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
            SDL_SaveBMP(surface, fullFilename.c_str());
            log_info << "Wrote screenshot to \"" << fullFilename << "\"" << std::endl;
            return;
          }
        }
        log_warning << "Did not save screenshot, because all files up to \"" << fullFilename << "\" already existed" << std::endl;
      }
    }
  }
}

void
SDLRenderer::flip()
{
  SDL_RenderPresent(m_renderer);
}

void
SDLRenderer::resize(int w , int h)
{
  g_config->window_size = Size(w, h);

  apply_config();
}

void
SDLRenderer::apply_video_mode()
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

void
SDLRenderer::apply_viewport()
{
  Size target_size = (g_config->use_fullscreen && g_config->fullscreen_size != Size(0, 0)) ?
    g_config->fullscreen_size :
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

  // calculate the viewport
  Size max_size(3840, 2160);
  Size min_size(640, 480);

  Size logical_size;
  calculate_viewport(min_size, max_size,
                     target_size,
                     pixel_aspect_ratio,
                     g_config->magnification,
                     m_scale, logical_size, m_viewport);

  SCREEN_WIDTH = logical_size.width;
  SCREEN_HEIGHT = logical_size.height;

  if (m_viewport.x != 0 || m_viewport.y != 0)
  {
    // Clear the screen to avoid garbage in unreachable areas after we
    // reset the coordinate system
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    SDL_RenderClear(m_renderer);
    SDL_RenderPresent(m_renderer);
    SDL_RenderClear(m_renderer);
  }

  // SetViewport() works in scaled screen coordinates, so we have to
  // reset it to 1.0, 1.0 to get meaningful results
  SDL_RenderSetScale(m_renderer, 1.0f, 1.0f);
  SDL_RenderSetViewport(m_renderer, &m_viewport);
  SDL_RenderSetScale(m_renderer, m_scale.x, m_scale.y);
}

void
SDLRenderer::apply_config()
{
  apply_video_mode();
  apply_viewport();
}

Vector
SDLRenderer::to_logical(int physical_x, int physical_y) const
{
  return Vector(static_cast<float>(physical_x - m_viewport.x) * SCREEN_WIDTH / m_viewport.w,
                static_cast<float>(physical_y - m_viewport.y) * SCREEN_HEIGHT / m_viewport.h);
}

void
SDLRenderer::set_gamma(float gamma)
{
  Uint16 ramp[256];
  SDL_CalculateGammaRamp(gamma, ramp);
  SDL_SetWindowGammaRamp(m_window, ramp, ramp, ramp);
}

/* EOF */
