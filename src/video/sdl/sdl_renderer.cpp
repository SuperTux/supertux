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
  renderer(),
  desktop_size()
{
  Renderer::instance_ = this;

  SDL_DisplayMode mode;
  SDL_GetCurrentDisplayMode(0, &mode);
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

  PHYSICAL_SCREEN_WIDTH = w;
  PHYSICAL_SCREEN_HEIGHT = h;

  apply_config();
}

void
SDLRenderer::apply_config()
{
  if (false)
  {
    log_info << "Applying Config:" 
             << "\n  Desktop: " << desktop_size.width << "x" << desktop_size.height
             << "\n  Window:  " << g_config->window_size
             << "\n  FullRes: " << g_config->fullscreen_size
             << "\n  Aspect:  " << g_config->aspect_size
             << "\n  Magnif:  " << g_config->magnification
             << std::endl;
  }

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

  //apply_video_mode(screen_size, g_config->use_fullscreen);

  if (target_aspect > 1.0f)
  {
    SCREEN_WIDTH  = static_cast<int>(screen_size.width * (target_aspect / desktop_aspect));
    SCREEN_HEIGHT = static_cast<int>(screen_size.height);
  }
  else
  {
    SCREEN_WIDTH  = static_cast<int>(screen_size.width);
    SCREEN_HEIGHT = static_cast<int>(screen_size.height  * (target_aspect / desktop_aspect));
  }

  Size max_size(1280, 800);
  Size min_size(640, 480);

  if (g_config->magnification == 0.0f) // Magic value that means 'minfill'
  {
    // This scales SCREEN_WIDTH/SCREEN_HEIGHT so that they never excede
    // max_size.width/max_size.height resp. min_size.width/min_size.height
    if (SCREEN_WIDTH > max_size.width || SCREEN_HEIGHT > max_size.height)
    {
      float scale1  = float(max_size.width)/SCREEN_WIDTH;
      float scale2  = float(max_size.height)/SCREEN_HEIGHT;
      float scale   = (scale1 < scale2) ? scale1 : scale2;
      SCREEN_WIDTH  = static_cast<int>(SCREEN_WIDTH  * scale);
      SCREEN_HEIGHT = static_cast<int>(SCREEN_HEIGHT * scale);
    } 
    else if (SCREEN_WIDTH < min_size.width || SCREEN_HEIGHT < min_size.height)
    {
      float scale1  = float(min_size.width)/SCREEN_WIDTH;
      float scale2  = float(min_size.height)/SCREEN_HEIGHT;
      float scale   = (scale1 < scale2) ? scale1 : scale2;
      SCREEN_WIDTH  = static_cast<int>(SCREEN_WIDTH  * scale);
      SCREEN_HEIGHT = static_cast<int>(SCREEN_HEIGHT * scale);
    }
  }
  else
  {
    SCREEN_WIDTH  = static_cast<int>(SCREEN_WIDTH  / g_config->magnification);
    SCREEN_HEIGHT = static_cast<int>(SCREEN_HEIGHT / g_config->magnification);

    // This works by adding black borders around the screen to limit
    // SCREEN_WIDTH/SCREEN_HEIGHT to max_size.width/max_size.height
    Size new_size = screen_size;

    if (SCREEN_WIDTH > max_size.width)
    {
      new_size.width = static_cast<int>((float) new_size.width * float(max_size.width)/SCREEN_WIDTH);
      SCREEN_WIDTH = static_cast<int>(max_size.width);
    }

    if (SCREEN_HEIGHT > max_size.height)
    {
      new_size.height = static_cast<int>((float) new_size.height * float(max_size.height)/SCREEN_HEIGHT);
      SCREEN_HEIGHT = static_cast<int>(max_size.height);
    }
  }
  SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
}

Vector
SDLRenderer::to_logical(int physical_x, int physical_y, bool foobar)
{
  if (foobar)
  {
    // SDL translates coordinates automatically, except for SDL_GetMouseState(), thus foobar
    return Vector(physical_x * float(SCREEN_WIDTH) / (PHYSICAL_SCREEN_WIDTH),
                  physical_y * float(SCREEN_HEIGHT) / (PHYSICAL_SCREEN_HEIGHT));
  }
  else
  {
    // SDL is doing the translation internally, so we have nothing to do
    return Vector(physical_x, physical_y);
  }
}

void
SDLRenderer::set_gamma(float gamma)
{
  Uint16 ramp[256];
  SDL_CalculateGammaRamp(gamma, ramp);
  SDL_SetWindowGammaRamp(window, ramp, ramp, ramp);
}

/* EOF */
