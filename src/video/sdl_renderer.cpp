//  $Id: sdl_renderer.cpp 5063 2007-05-27 11:32:00Z matzeb $
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

#include <functional>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <SDL_image.h>
#include <sstream>
#include <iomanip>
#include <physfs.h>

#include "glutil.hpp"
#include "sdl_renderer.hpp"
#include "sdl_texture.hpp"
#include "drawing_context.hpp"
#include "drawing_request.hpp"
#include "surface.hpp"
#include "font.hpp"
#include "main.hpp"
#include "gameconfig.hpp"
#include "texture.hpp"
#include "texture_manager.hpp"
#include "obstack/obstackpp.hpp"

namespace SDL
{
  Renderer::Renderer()
  {
    int flags = SDL_SWSURFACE;
    if(config->use_fullscreen)
      flags |= SDL_FULLSCREEN;
    int width = config->screenwidth;
    int height = config->screenheight;
    int bpp = 0;

    screen = SDL_SetVideoMode(width, height, bpp, flags);
    if(screen == 0) {
      std::stringstream msg;
      msg << "Couldn't set video mode (" << width << "x" << height
          << "-" << bpp << "bpp): " << SDL_GetError();
      throw std::runtime_error(msg.str());
    }

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
    SDL::Texture *sdltexture = dynamic_cast<Texture *>(surface->get_texture());
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
      ox = sdltexture->get_texture_width() - surface->get_x() - surface->get_width();
    }
    else
    {
      ox = surface->get_x();
    }
    if (effect == VERTICAL_FLIP)
    {
      oy = sdltexture->get_texture_height() - surface->get_y() - surface->get_height();
    }
    else
    {
      oy = surface->get_y();
    }

    int numerator, denominator;
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

    SDL_Rect srcRect;
    srcRect.x = ox * numerator / denominator;
    srcRect.y = oy * numerator / denominator;
    srcRect.w = surface->get_width() * numerator / denominator;
    srcRect.h = surface->get_height() * numerator / denominator;

    SDL_Rect dstRect;
    dstRect.x = (int) request.pos.x * numerator / denominator;
    dstRect.y = (int) request.pos.y * numerator / denominator;

    SDL_BlitSurface(transform, &srcRect, screen, &dstRect);
  }

  void
  Renderer::draw_surface_part(const DrawingRequest& request)
  {
    const SurfacePartRequest* surfacepartrequest
      = (SurfacePartRequest*) request.request_data;

    const Surface* surface = surfacepartrequest->surface;
    SDL::Texture *sdltexture = dynamic_cast<Texture *>(surface->get_texture());
    DrawingEffect effect = request.drawing_effect;
    if (surface->get_flipx()) effect = HORIZONTAL_FLIP;

    SDL_Surface *transform = sdltexture->get_transform(Color(1.0, 1.0, 1.0), effect);

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

    int numerator, denominator;
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

    SDL_Rect srcRect;
    srcRect.x = (ox + (int) surfacepartrequest->source.x) * numerator / denominator;
    srcRect.y = (oy + (int) surfacepartrequest->source.y) * numerator / denominator;
    srcRect.w = (int) surfacepartrequest->size.x * numerator / denominator;
    srcRect.h = (int) surfacepartrequest->size.y * numerator / denominator;

    SDL_Rect dstRect;
    dstRect.x = (int) request.pos.x * numerator / denominator;
    dstRect.y = (int) request.pos.y * numerator / denominator;

    SDL_BlitSurface(transform, &srcRect, screen, &dstRect);
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
        SDL_SetAlpha(temp, SDL_SRCALPHA, a);
        SDL_BlitSurface(temp, 0, screen, &rect);
        SDL_FreeSurface(temp);
      }
    }
  }

  void
  Renderer::draw_text(const DrawingRequest& request)
  {
    const TextRequest* textrequest = (TextRequest*) request.request_data;

    textrequest->font->draw(this, textrequest->text, request.pos,
        textrequest->alignment, request.drawing_effect, request.alpha);
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
      SDL_SetAlpha(temp, SDL_SRCALPHA, a);
      SDL_BlitSurface(temp, 0, screen, &rect);
      SDL_FreeSurface(temp);
    }
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
}
