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
#include "SDL2/SDL_video.h"

#include "video/util.hpp"

SDLRenderer::SDLRenderer() :
  window(),
  renderer(),
  viewport(),
  desktop_size()
{
  Renderer::instance_ = this;

  SDL_DisplayMode mode;
  SDL_GetDesktopDisplayMode(0, &mode);
  desktop_size = Size(mode.w, mode.h);

  log_info << "creating SDLRenderer" << std::endl;
  int width  = g_config->window_size.width;
  int height = g_config->window_size.height;

  int flags = SDL_WINDOW_RESIZABLE;
  if(g_config->use_fullscreen)
  {
    flags |= SDL_WINDOW_FULLSCREEN;
    width  = g_config->fullscreen_size.width;
    height = g_config->fullscreen_size.height;
  }

  SCREEN_WIDTH = width;
  SCREEN_HEIGHT = height;

  viewport.x = 0;
  viewport.y = 0;
  viewport.w = width;
  viewport.h = height;

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

  int ret = SDL_CreateWindowAndRenderer(width, height, flags,
                                        &window, &renderer);

  if(ret != 0) {
    std::stringstream msg;
    msg << "Couldn't set video mode (" << width << "x" << height
        << "): " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }

  SDL_RendererInfo info;
  if (SDL_GetRendererInfo(renderer, &info) != 0)
  {
    log_warning << "Couldn't get RendererInfo: " << SDL_GetError() << std::endl;
  }
  else
  {
    log_info << "SDL_Renderer: " << info.name << std::endl;
    log_info << "SDL_RendererFlags: " << std::endl;
    if (info.flags & SDL_RENDERER_SOFTWARE) log_info << "  SDL_RENDERER_SOFTWARE" << std::endl;
    if (info.flags & SDL_RENDERER_ACCELERATED) log_info << "  SDL_RENDERER_ACCELERATED" << std::endl;
    if (info.flags & SDL_RENDERER_PRESENTVSYNC) log_info << "  SDL_RENDERER_PRESENTVSYNC" << std::endl;
    if (info.flags & SDL_RENDERER_TARGETTEXTURE) log_info << "  SDL_RENDERER_TARGETTEXTURE" << std::endl;
    log_info << "Texture Formats: " << std::endl;
    for(size_t i = 0; i < info.num_texture_formats; ++i)
    {
      log_info << "  " << SDL_GetPixelFormatName(info.texture_formats[i]) << std::endl;
    }
    log_info << "Max Texture Width: " << info.max_texture_width << std::endl;
    log_info << "Max Texture Height: " << info.max_texture_height << std::endl;
  }

  if(texture_manager == 0)
    texture_manager = new TextureManager();

  g_config->window_size = Size(width, height);
  apply_config();
}

SDLRenderer::~SDLRenderer()
{
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
}

void
SDLRenderer::draw_surface(const DrawingRequest& request)
{
  SDLPainter::draw_surface(renderer, request);
}

void
SDLRenderer::draw_surface_part(const DrawingRequest& request)
{
  SDLPainter::draw_surface_part(renderer, request);
}

void
SDLRenderer::draw_gradient(const DrawingRequest& request)
{
  SDLPainter::draw_gradient(renderer, request);
}

void
SDLRenderer::draw_filled_rect(const DrawingRequest& request)
{
  SDLPainter::draw_filled_rect(renderer, request);
}

void
SDLRenderer::draw_inverse_ellipse(const DrawingRequest& request)
{
  SDLPainter::draw_inverse_ellipse(renderer, request);
}

void
SDLRenderer::do_take_screenshot()
{
  // [Christoph] TODO: Yes, this method also takes care of the actual disk I/O. Split it?
  int width;
  int height;
  if (SDL_GetRendererOutputSize(renderer, &width, &height) != 0)
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
      int ret = SDL_RenderReadPixels(renderer, NULL,
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
            log_debug << "Wrote screenshot to \"" << fullFilename << "\"" << std::endl;
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
  SDL_RenderPresent(renderer);
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
    SDL_SetWindowFullscreen(window, 0);
  }
  else
  {
    SDL_DisplayMode mode;
    mode.format = SDL_PIXELFORMAT_RGB888;
    mode.w = g_config->fullscreen_size.width;
    mode.h = g_config->fullscreen_size.height;
    mode.refresh_rate = g_config->fullscreen_refresh_rate;
    mode.driverdata = 0;

    if (SDL_SetWindowDisplayMode(window, &mode) != 0)
    {
      log_warning << "failed to set display mode: "
                  << mode.w << "x" << mode.h << "@" << mode.refresh_rate << ": "
                  << SDL_GetError() << std::endl;
    }
    else
    {
      SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    }
  }

}

void
SDLRenderer::apply_viewport()
{
  // calculate the aspect ratio
  float target_aspect = static_cast<float>(desktop_size.width) / static_cast<float>(desktop_size.height);
  if (g_config->aspect_size != Size(0, 0))
  {
    target_aspect = float(g_config->aspect_size.width) / float(g_config->aspect_size.height);
  }

  float desktop_aspect = 4.0f / 3.0f; // random default fallback guess
  if (desktop_size.width != -1 && desktop_size.height != -1)
  {
    desktop_aspect = float(desktop_size.width) / float(desktop_size.height);
  }

  Size screen_size;

  // Get the screen width
  if (g_config->use_fullscreen)
  {
    screen_size = g_config->fullscreen_size;
    desktop_aspect = float(screen_size.width) / float(screen_size.height);
  }
  else
  {
    screen_size = g_config->window_size;
  }

  // calculate the viewport
  Size max_size(1280, 800);
  Size min_size(640, 480);

  // FIXME: don't do this, save window size
  Size window_size;
  SDL_GetWindowSize(window, &window_size.width, &window_size.height);

  Vector scale;
  Size logical_size;
  calculate_viewport(min_size, max_size, window_size,
                     target_aspect / desktop_aspect,
                     g_config->magnification,
                     scale, logical_size, viewport);

  SCREEN_WIDTH = logical_size.width;
  SCREEN_HEIGHT = logical_size.height;

  if (viewport.x != 0 || viewport.y != 0)
  {
    // Clear the screen to avoid garbage in unreachable areas after we
    // reset the coordinate system
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);
  }

  SDL_RenderSetScale(renderer, 1.0f, 1.0f);
  SDL_RenderSetViewport(renderer, &viewport);
  SDL_RenderSetScale(renderer, scale.x, scale.y);
}

void
SDLRenderer::apply_config()
{
  apply_video_mode();
  apply_viewport();
}

Vector
SDLRenderer::to_logical(int physical_x, int physical_y)
{
  return Vector(static_cast<float>(physical_x - viewport.x) * SCREEN_WIDTH / viewport.w,
                static_cast<float>(physical_y - viewport.y) * SCREEN_HEIGHT / viewport.h);
}

void
SDLRenderer::set_gamma(float gamma)
{
  Uint16 ramp[256];
  SDL_CalculateGammaRamp(gamma, ramp);
  SDL_SetWindowGammaRamp(window, ramp, ramp, ramp);
}

/* EOF */
