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

  int flags = 0;
  if(g_config->use_fullscreen)
  {
    flags |= SDL_WINDOW_FULLSCREEN;
    width  = g_config->fullscreen_size.width;
    height = g_config->fullscreen_size.height;
  }

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

  SDL_SetWindowTitle(window, "SuperTux");
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
  //FIXME: support parameters request.angle, request.blend
  const Surface* surface = (const Surface*) request.request_data;
  boost::shared_ptr<SDLTexture> sdltexture = boost::dynamic_pointer_cast<SDLTexture>(surface->get_texture());

  SDL_Rect dst_rect;
  dst_rect.x = request.pos.x;
  dst_rect.y = request.pos.y;
  dst_rect.w = sdltexture->get_image_width();
  dst_rect.h = sdltexture->get_image_height();

  Uint8 r = static_cast<Uint8>(request.color.red * 255);
  Uint8 g = static_cast<Uint8>(request.color.green * 255);
  Uint8 b = static_cast<Uint8>(request.color.blue * 255);
  Uint8 a = static_cast<Uint8>(request.color.alpha * request.alpha * 255);
  SDL_SetTextureColorMod(sdltexture->get_texture(), r, g, b);
  SDL_SetTextureAlphaMod(sdltexture->get_texture(), a);

  if (surface->get_flipx())
  {
    SDL_RenderCopyEx(renderer, sdltexture->get_texture(), NULL, &dst_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
  }
  else
  {
    switch(request.drawing_effect)
    {
      case VERTICAL_FLIP:
        SDL_RenderCopyEx(renderer, sdltexture->get_texture(), NULL, &dst_rect, 0, NULL, SDL_FLIP_VERTICAL);
        break;

      case HORIZONTAL_FLIP:
        SDL_RenderCopyEx(renderer, sdltexture->get_texture(), NULL, &dst_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        break;

      default:
      case NO_EFFECT:
        SDL_RenderCopy(renderer, sdltexture->get_texture(), NULL, &dst_rect);
        break;
    }
  }
}

void
SDLRenderer::draw_surface_part(const DrawingRequest& request)
{
  //FIXME: support parameters request.angle, request.blend
  const SurfacePartRequest* surface = (const SurfacePartRequest*) request.request_data;
  const SurfacePartRequest* surfacepartrequest = (SurfacePartRequest*) request.request_data;

  boost::shared_ptr<SDLTexture> sdltexture = boost::dynamic_pointer_cast<SDLTexture>(surface->surface->get_texture());

  SDL_Rect src_rect;
  src_rect.x = surfacepartrequest->source.x;
  src_rect.y = surfacepartrequest->source.y;
  src_rect.w = surfacepartrequest->size.x;
  src_rect.h = surfacepartrequest->size.y;

  SDL_Rect dst_rect;
  dst_rect.x = request.pos.x;
  dst_rect.y = request.pos.y;
  dst_rect.w = surfacepartrequest->size.x;
  dst_rect.h = surfacepartrequest->size.y;

  Uint8 r = static_cast<Uint8>(request.color.red * 255);
  Uint8 g = static_cast<Uint8>(request.color.green * 255);
  Uint8 b = static_cast<Uint8>(request.color.blue * 255);
  Uint8 a = static_cast<Uint8>(request.color.alpha * request.alpha * 255);
  SDL_SetTextureColorMod(sdltexture->get_texture(), r, g, b);
  SDL_SetTextureAlphaMod(sdltexture->get_texture(), a);

  if (surface->surface->get_flipx())
  {
    SDL_RenderCopyEx(renderer, sdltexture->get_texture(), &src_rect, &dst_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
  }
  else
  {
    switch(request.drawing_effect)
    {
      case VERTICAL_FLIP:
        SDL_RenderCopyEx(renderer, sdltexture->get_texture(), &src_rect, &dst_rect, 0, NULL, SDL_FLIP_VERTICAL);
        break;

      case HORIZONTAL_FLIP:
        SDL_RenderCopyEx(renderer, sdltexture->get_texture(), &src_rect, &dst_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
        break;

      default:
      case NO_EFFECT:
        SDL_RenderCopy(renderer, sdltexture->get_texture(), &src_rect, &dst_rect);
        break;
    }
  }
}

void
SDLRenderer::draw_gradient(const DrawingRequest& request)
{
  const GradientRequest* gradientrequest 
    = (GradientRequest*) request.request_data;
  const Color& top = gradientrequest->top;
  const Color& bottom = gradientrequest->bottom;

  int w;
  int h;
  SDL_GetWindowSize(window, &w, &h);

  // calculate the maximum number of steps needed for the gradient
  int n = static_cast<int>(std::max(std::max(fabsf(top.red - bottom.red),
                                             fabsf(top.green - bottom.green)),
                                    std::max(fabsf(top.blue - bottom.blue),
                                             fabsf(top.alpha - bottom.alpha))) * 255);
  for(int i = 0; i < n; ++i)
  {
    SDL_Rect rect;
    rect.x = 0;
    rect.y = h * i / n;
    rect.w = w;
    rect.h = (h * (i+1) / n) - rect.y;

    float p = static_cast<float>(i+1) / static_cast<float>(n);
    Uint8 r = static_cast<Uint8>(((1.0f - p) * top.red + p * bottom.red)  * 255);
    Uint8 g = static_cast<Uint8>(((1.0f - p) * top.green + p * bottom.green) * 255);
    Uint8 b = static_cast<Uint8>(((1.0f - p) * top.blue + p * bottom.blue) * 255);
    Uint8 a = static_cast<Uint8>(((1.0f - p) * top.alpha + p * bottom.alpha) * 255);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRect(renderer, &rect);
  }
}

void
SDLRenderer::draw_filled_rect(const DrawingRequest& request)
{
  const FillRectRequest* fillrectrequest
    = (FillRectRequest*) request.request_data;

  SDL_Rect rect;
  rect.x = request.pos.x;
  rect.y = request.pos.y;
  rect.w = fillrectrequest->size.x;
  rect.h = fillrectrequest->size.y;

  Uint8 r = static_cast<Uint8>(fillrectrequest->color.red * 255);
  Uint8 g = static_cast<Uint8>(fillrectrequest->color.green * 255);
  Uint8 b = static_cast<Uint8>(fillrectrequest->color.blue * 255);
  Uint8 a = static_cast<Uint8>(fillrectrequest->color.alpha * 255);

  int radius = std::min(std::min(rect.h / 2, rect.w / 2),
                        static_cast<int>(fillrectrequest->radius));

  if (radius)
  {
    int slices = radius;

    // rounded top and bottom parts
    std::vector<SDL_Rect> rects;
    rects.reserve(2*slices + 1);
    for(int i = 0; i < slices; ++i)
    {
      float p = (static_cast<float>(i) + 0.5f) / static_cast<float>(slices);
      int xoff = radius - static_cast<int>(sqrtf(1.0f - p*p) * radius);

      SDL_Rect tmp;
      tmp.x = rect.x + xoff;
      tmp.y = rect.y + (radius - i);
      tmp.w = rect.w - 2*(xoff);
      tmp.h = 1;
      rects.push_back(tmp);

      SDL_Rect tmp2;
      tmp2.x = rect.x + xoff;
      tmp2.y = rect.y + rect.h - radius + i;
      tmp2.w = rect.w - 2*xoff;
      tmp2.h = 1;

      if (tmp2.y != tmp.y)
      {
        rects.push_back(tmp2);
      }
    }

    if (2*radius < rect.h)
    {
      // center rectangle
      SDL_Rect tmp;
      tmp.x = rect.x;
      tmp.y = rect.y + radius + 1;
      tmp.w = rect.w;
      tmp.h = rect.h - 2*radius - 1;
      rects.push_back(tmp);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRects(renderer, &*rects.begin(), rects.size());
  }
  else
  {
    if((rect.w != 0) && (rect.h != 0))
    {
      SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
      SDL_SetRenderDrawColor(renderer, r, g, b, a);
      SDL_RenderFillRect(renderer, &rect);
    }
  }
}

void
SDLRenderer::draw_inverse_ellipse(const DrawingRequest& request)
{
  const InverseEllipseRequest* ellipse = (InverseEllipseRequest*)request.request_data;

  int window_w;
  int window_h;
  SDL_GetWindowSize(window, &window_w, &window_h);

  float x = request.pos.x;
  float w = ellipse->size.x;
  float h = ellipse->size.y;

  int top = request.pos.y - (h / 2);

  const int max_slices = 256;
  SDL_Rect rects[2*max_slices+2];
  int slices = std::min(static_cast<int>(ellipse->size.y), max_slices);
  for(int i = 0; i < slices; ++i)
  {
    float p = ((static_cast<float>(i) + 0.5f) / static_cast<float>(slices)) * 2.0f - 1.0f; 
    int xoff = static_cast<int>(sqrtf(1.0f - p*p) * w / 2);

    SDL_Rect& left  = rects[2*i+0];
    SDL_Rect& right = rects[2*i+1];

    left.x = 0;
    left.y = top + (i * h / slices);
    left.w = x - xoff;
    left.h = (top + ((i+1) * h / slices)) - left.y;

    right.x = x + xoff;
    right.y = left.y;
    right.w = window_w - right.x;
    right.h = left.h;
  }

  SDL_Rect& top_rect = rects[2*slices+0];
  SDL_Rect& bottom_rect = rects[2*slices+1];

  top_rect.x = 0;
  top_rect.y = 0;
  top_rect.w = window_w;
  top_rect.h = top;

  bottom_rect.x = 0;
  bottom_rect.y = top + h;
  bottom_rect.w = window_w;
  bottom_rect.h = window_h - bottom_rect.y;

  Uint8 r = static_cast<Uint8>(ellipse->color.red * 255);
  Uint8 g = static_cast<Uint8>(ellipse->color.green * 255);
  Uint8 b = static_cast<Uint8>(ellipse->color.blue * 255);
  Uint8 a = static_cast<Uint8>(ellipse->color.alpha * 255);

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, r, g, b, a);
  SDL_RenderFillRects(renderer, rects, 2*slices+2);
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
SDLRenderer::resize(int, int)
{
    
}

void
SDLRenderer::set_gamma(float gamma)
{
  Uint16 ramp[256];
  SDL_CalculateGammaRamp(gamma, ramp);
  SDL_SetWindowGammaRamp(window, ramp, ramp, ramp);
}

/* EOF */
