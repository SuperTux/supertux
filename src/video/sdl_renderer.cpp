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
#include <config.h>

#include <functional>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <SDL_image.h>
#include <sstream>
#include <iomanip>
#include <physfs.h>

#include "video/glutil.hpp"
#include "video/sdl_renderer.hpp"
#include "video/sdl_texture.hpp"
#include "video/sdl_surface_data.hpp"
#include "video/drawing_context.hpp"
#include "video/drawing_request.hpp"
#include "video/surface.hpp"
#include "video/font.hpp"
#include "supertux/main.hpp"
#include "supertux/gameconfig.hpp"
#include "util/log.hpp"
#include "video/texture.hpp"
#include "video/texture_manager.hpp"
#include "obstack/obstackpp.hpp"

namespace
{
  SDL_Surface *apply_alpha(SDL_Surface *src, float alpha_factor)
  {
    // FIXME: This is really slow
    assert(src->format->Amask);
    int alpha = (int) (alpha_factor * 256);
    SDL_Surface *dst = SDL_CreateRGBSurface(src->flags, src->w, src->h, src->format->BitsPerPixel, src->format->Rmask,  src->format->Gmask, src->format->Bmask, src->format->Amask);
    int bpp = dst->format->BytesPerPixel;
    if(SDL_MUSTLOCK(src))
    {
      SDL_LockSurface(src);
    }
    if(SDL_MUSTLOCK(dst))
    {
      SDL_LockSurface(dst);
    }
    for(int y = 0;y < dst->h;y++) {
      for(int x = 0;x < dst->w;x++) {
        Uint8 *srcpixel = (Uint8 *) src->pixels + y * src->pitch + x * bpp;
        Uint8 *dstpixel = (Uint8 *) dst->pixels + y * dst->pitch + x * bpp;
        Uint32 mapped = 0;
        switch(bpp) {
          case 1:
            mapped = *srcpixel;
            break;
          case 2:
            mapped = *(Uint16 *)srcpixel;
            break;
          case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            mapped |= srcpixel[0] << 16;
            mapped |= srcpixel[1] << 8;
            mapped |= srcpixel[2] << 0;
#else
            mapped |= srcpixel[0] << 0;
            mapped |= srcpixel[1] << 8;
            mapped |= srcpixel[2] << 16;
#endif
            break;
          case 4:
            mapped = *(Uint32 *)srcpixel;
            break;
        }
        Uint8 r, g, b, a;
        SDL_GetRGBA(mapped, src->format, &r, &g, &b, &a);
        mapped = SDL_MapRGBA(dst->format, r, g, b, (a * alpha) >> 8);
        switch(bpp) {
          case 1:
            *dstpixel = mapped;
            break;
          case 2:
            *(Uint16 *)dstpixel = mapped;
            break;
          case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            dstpixel[0] = (mapped >> 16) & 0xff;
            dstpixel[1] = (mapped >> 8) & 0xff;
            dstpixel[2] = (mapped >> 0) & 0xff;
#else
            dstpixel[0] = (mapped >> 0) & 0xff;
            dstpixel[1] = (mapped >> 8) & 0xff;
            dstpixel[2] = (mapped >> 16) & 0xff;
#endif
            break;
          case 4:
            *(Uint32 *)dstpixel = mapped;
            break;
        }
      }
    }
    if(SDL_MUSTLOCK(dst))
    {
      SDL_UnlockSurface(dst);
    }
    if(SDL_MUSTLOCK(src))
    {
      SDL_UnlockSurface(src);
    }
    return dst;
  }
}

namespace SDL
{
  Renderer::Renderer()
  {
    ::Renderer::instance_ = this;

    const SDL_VideoInfo *info = SDL_GetVideoInfo();
    log_info << "Hardware surfaces are " << (info->hw_available ? "" : "not ") << "available." << std::endl;
    log_info << "Hardware to hardware blits are " << (info->blit_hw ? "" : "not ") << "accelerated." << std::endl;
    log_info << "Hardware to hardware blits with colorkey are " << (info->blit_hw_CC ? "" : "not ") << "accelerated." << std::endl;
    log_info << "Hardware to hardware blits with alpha are " << (info->blit_hw_A ? "" : "not ") << "accelerated." << std::endl;
    log_info << "Software to hardware blits are " << (info->blit_sw ? "" : "not ") << "accelerated." << std::endl;
    log_info << "Software to hardware blits with colorkey are " << (info->blit_sw_CC ? "" : "not ") << "accelerated." << std::endl;
    log_info << "Software to hardware blits with alpha are " << (info->blit_sw_A ? "" : "not ") << "accelerated." << std::endl;
    log_info << "Color fills are " << (info->blit_fill ? "" : "not ") << "accelerated." << std::endl;

    int flags = SDL_SWSURFACE | SDL_ANYFORMAT;
    if(config->use_fullscreen)
      flags |= SDL_FULLSCREEN;
    
    int width  = 800; //FIXME: config->screenwidth;
    int height = 600; //FIXME: config->screenheight;

    screen = SDL_SetVideoMode(width, height, 0, flags);
    if(screen == 0) {
      std::stringstream msg;
      msg << "Couldn't set video mode (" << width << "x" << height
          << "): " << SDL_GetError();
      throw std::runtime_error(msg.str());
    }

    numerator   = 1;
    denominator = 1;
    /* FIXME: 
    float xfactor = (float) config->screenwidth / SCREEN_WIDTH;
    float yfactor = (float) config->screenheight / SCREEN_HEIGHT;
    if(xfactor < yfactor)
    {
      numerator = config->screenwidth;
      denominator = SCREEN_WIDTH;
    }
    else
    {
      numerator = config->screenheight;
      denominator = SCREEN_HEIGHT;
    }
    */
    if(texture_manager == 0)
      texture_manager = new TextureManager();
  }

  Renderer::~Renderer()
  {
  }

  void
  Renderer::draw_surface(const DrawingRequest& request)
  {
    //FIXME: support parameters request.alpha, request.angle, request.blend
    const Surface* surface = (const Surface*) request.request_data;
    SDL::Texture *sdltexture = dynamic_cast<SDL::Texture *>(surface->get_texture());
    SDL::SurfaceData *surface_data = reinterpret_cast<SDL::SurfaceData *>(surface->get_surface_data());

    DrawingEffect effect = request.drawing_effect;
    if (surface->get_flipx()) effect = HORIZONTAL_FLIP;

    SDL_Surface *transform = sdltexture->get_transform(request.color, effect);

    // get and check SDL_Surface
    if (transform == 0) {
      std::cerr << "Warning: Tried to draw NULL surface, skipped draw" << std::endl;
      return;
    }

    SDL_Rect *src_rect = surface_data->get_src_rect(effect);
    SDL_Rect dst_rect;
    dst_rect.x = (int) request.pos.x * numerator / denominator;
    dst_rect.y = (int) request.pos.y * numerator / denominator;

    Uint8 alpha = 0;
    if(request.alpha != 1.0)
    {
      if(!transform->format->Amask)
      {
        if(transform->flags & SDL_SRCALPHA)
        {
          alpha = transform->format->alpha;
        }
        else
        {
          alpha = 255;
        }
        SDL_SetAlpha(transform, SDL_SRCALPHA, (Uint8) (request.alpha * alpha));
      }
      /*else
      {
        transform = apply_alpha(transform, request.alpha);
      }*/
    }

    SDL_BlitSurface(transform, src_rect, screen, &dst_rect);

    if(request.alpha != 1.0)
    {
      if(!transform->format->Amask)
      {
        if(alpha == 255)
        {
          SDL_SetAlpha(transform, SDL_RLEACCEL, 0);
        }
        else
        {
          SDL_SetAlpha(transform, SDL_SRCALPHA | SDL_RLEACCEL, alpha);
        }
      }
      /*else
      {
        SDL_FreeSurface(transform);
      }*/
    }
  }

  void
  Renderer::draw_surface_part(const DrawingRequest& request)
  {
    const SurfacePartRequest* surfacepartrequest
      = (SurfacePartRequest*) request.request_data;

    const Surface* surface = surfacepartrequest->surface;
    SDL::Texture *sdltexture = dynamic_cast<SDL::Texture *>(surface->get_texture());

    DrawingEffect effect = request.drawing_effect;
    if (surface->get_flipx()) effect = HORIZONTAL_FLIP;

    SDL_Surface *transform = sdltexture->get_transform(request.color, effect);

    // get and check SDL_Surface
    if (transform == 0) {
      std::cerr << "Warning: Tried to draw NULL surface, skipped draw" << std::endl;
      return;
    }

    int ox, oy;
    if (effect == HORIZONTAL_FLIP)
    {
      ox = sdltexture->get_texture_width() - surface->get_x() - (int) surfacepartrequest->size.x;
    }
    else
    {
      ox = surface->get_x();
    }
    if (effect == VERTICAL_FLIP)
    {
      oy = sdltexture->get_texture_height() - surface->get_y() - (int) surfacepartrequest->size.y;
    }
    else
    {
      oy = surface->get_y();
    }

    SDL_Rect src_rect;
    src_rect.x = (ox + (int) surfacepartrequest->source.x) * numerator / denominator;
    src_rect.y = (oy + (int) surfacepartrequest->source.y) * numerator / denominator;
    src_rect.w = (int) surfacepartrequest->size.x * numerator / denominator;
    src_rect.h = (int) surfacepartrequest->size.y * numerator / denominator;

    SDL_Rect dst_rect;
    dst_rect.x = (int) request.pos.x * numerator / denominator;
    dst_rect.y = (int) request.pos.y * numerator / denominator;

    Uint8 alpha = 0;
    if(request.alpha != 1.0)
    {
      if(!transform->format->Amask)
      {
        if(transform->flags & SDL_SRCALPHA)
        {
          alpha = transform->format->alpha;
        }
        else
        {
          alpha = 255;
        }
        SDL_SetAlpha(transform, SDL_SRCALPHA, (Uint8) (request.alpha * alpha));
      }
      /*else
      {
        transform = apply_alpha(transform, request.alpha);
      }*/
    }

    SDL_BlitSurface(transform, &src_rect, screen, &dst_rect);

    if(request.alpha != 1.0)
    {
      if(!transform->format->Amask)
      {
        if(alpha == 255)
        {
          SDL_SetAlpha(transform, SDL_RLEACCEL, 0);
        }
        else
        {
          SDL_SetAlpha(transform, SDL_SRCALPHA | SDL_RLEACCEL, alpha);
        }
      }
      /*else
      {
        SDL_FreeSurface(transform);
      }*/
    }
  }

  void
  Renderer::draw_gradient(const DrawingRequest& request)
  {
    const GradientRequest* gradientrequest 
      = (GradientRequest*) request.request_data;
    const Color& top = gradientrequest->top;
    const Color& bottom = gradientrequest->bottom;

    for(int y = 0;y < screen->h;++y)
    {
      Uint8 r = (Uint8)((((float)(top.red-bottom.red)/(0-screen->h)) * y + top.red) * 255);
      Uint8 g = (Uint8)((((float)(top.green-bottom.green)/(0-screen->h)) * y + top.green) * 255);
      Uint8 b = (Uint8)((((float)(top.blue-bottom.blue)/(0-screen->h)) * y + top.blue) * 255);
      Uint8 a = (Uint8)((((float)(top.alpha-bottom.alpha)/(0-screen->h)) * y + top.alpha) * 255);
      Uint32 color = SDL_MapRGB(screen->format, r, g, b);

      SDL_Rect rect;
      rect.x = 0;
      rect.y = y;
      rect.w = screen->w;
      rect.h = 1;

      if(a == SDL_ALPHA_OPAQUE) {
        SDL_FillRect(screen, &rect, color);
      } else if(a != SDL_ALPHA_TRANSPARENT) {
        SDL_Surface *temp = SDL_CreateRGBSurface(screen->flags, rect.w, rect.h, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);

        SDL_FillRect(temp, 0, color);
        SDL_SetAlpha(temp, SDL_SRCALPHA | SDL_RLEACCEL, a);
        SDL_BlitSurface(temp, 0, screen, &rect);
        SDL_FreeSurface(temp);
      }
    }
  }

  void
  Renderer::draw_filled_rect(const DrawingRequest& request)
  {
    const FillRectRequest* fillrectrequest
      = (FillRectRequest*) request.request_data;

    SDL_Rect rect;
    rect.x = (Sint16)request.pos.x * screen->w / SCREEN_WIDTH;
    rect.y = (Sint16)request.pos.y * screen->h / SCREEN_HEIGHT;
    rect.w = (Uint16)fillrectrequest->size.x * screen->w / SCREEN_WIDTH;
    rect.h = (Uint16)fillrectrequest->size.y * screen->h / SCREEN_HEIGHT;
    Uint8 r = static_cast<Uint8>(fillrectrequest->color.red * 255);
    Uint8 g = static_cast<Uint8>(fillrectrequest->color.green * 255);
    Uint8 b = static_cast<Uint8>(fillrectrequest->color.blue * 255);
    Uint8 a = static_cast<Uint8>(fillrectrequest->color.alpha * 255);
    Uint32 color = SDL_MapRGB(screen->format, r, g, b);
    if(a == SDL_ALPHA_OPAQUE) {
      SDL_FillRect(screen, &rect, color);
    } else if(a != SDL_ALPHA_TRANSPARENT) {
      SDL_Surface *temp = SDL_CreateRGBSurface(screen->flags, rect.w, rect.h, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);

      SDL_FillRect(temp, 0, color);
      SDL_SetAlpha(temp, SDL_SRCALPHA | SDL_RLEACCEL, a);
      SDL_BlitSurface(temp, 0, screen, &rect);
      SDL_FreeSurface(temp);
    }
  }

  void
  Renderer::draw_inverse_ellipse(const DrawingRequest&)
  {
  }

  void 
  Renderer::do_take_screenshot()
  {
    // [Christoph] TODO: Yes, this method also takes care of the actual disk I/O. Split it?

    SDL_Surface *screen = SDL_GetVideoSurface();

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
        SDL_SaveBMP(screen, fullFilename.c_str());
        log_debug << "Wrote screenshot to \"" << fullFilename << "\"" << std::endl;
        return;
      }
    }
    log_warning << "Did not save screenshot, because all files up to \"" << fullFilename << "\" already existed" << std::endl;
  }

  void
  Renderer::flip()
  {
    SDL_Flip(screen);
  }

  void
  Renderer::resize(int, int)
  {
    
  }
}
