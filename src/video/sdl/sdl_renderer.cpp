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

SDLRenderer::SDLRenderer() :
  window(),
  renderer()
{
  Renderer::instance_ = this;

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

  PHYSICAL_SCREEN_WIDTH = width;
  PHYSICAL_SCREEN_HEIGHT = height;

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
  SCREEN_WIDTH  = w;
  SCREEN_HEIGHT = h;

  PHYSICAL_SCREEN_WIDTH = w;
  PHYSICAL_SCREEN_HEIGHT = h;
}

void
SDLRenderer::set_gamma(float gamma)
{
  Uint16 ramp[256];
  SDL_CalculateGammaRamp(gamma, ramp);
  SDL_SetWindowGammaRamp(window, ramp, ramp, ramp);
}

/* EOF */
