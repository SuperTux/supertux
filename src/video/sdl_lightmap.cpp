//  $Id: sdl_lightmap.cpp 5063 2007-05-27 11:32:00Z matzeb $
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
#include <cassert>
#include <iostream>
#include <SDL_image.h>
#include <sstream>
#include <iomanip>
#include <physfs.h>

#include "glutil.hpp"
#include "sdl_lightmap.hpp"
#include "sdl_texture.hpp"
#include "sdl_surface_data.hpp"
#include "drawing_context.hpp"
#include "drawing_request.hpp"
#include "renderer.hpp"
#include "surface.hpp"
#include "font.hpp"
#include "main.hpp"
#include "gameconfig.hpp"
#include "texture.hpp"
#include "texture_manager.hpp"
#include "obstack/obstackpp.hpp"

namespace SDL
{
  Lightmap::Lightmap()
  {
    screen = SDL_GetVideoSurface();

    width = screen->w;
    height = screen->h;

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

    red_channel = (Uint8 *)malloc(width * height * sizeof(Uint8));
    green_channel = (Uint8 *)malloc(width * height * sizeof(Uint8));
    blue_channel = (Uint8 *)malloc(width * height * sizeof(Uint8));
  }

  Lightmap::~Lightmap()
  {
    free(red_channel);
    free(green_channel);
    free(blue_channel);
  }

  void
  Lightmap::start_draw(const Color &ambient_color)
  {
    memset(red_channel, (Uint8) (ambient_color.red * 255), width * height * sizeof(Uint8));
    memset(green_channel, (Uint8) (ambient_color.green * 255), width * height * sizeof(Uint8));
    memset(blue_channel, (Uint8) (ambient_color.blue * 255), width * height * sizeof(Uint8));
  }

  void
  Lightmap::end_draw()
  {
  }

  void
  Lightmap::do_draw()
  {
    // FIXME: This is really slow
    int bpp = screen->format->BytesPerPixel;
    Uint8 *pixel = (Uint8 *) screen->pixels;
    int loc = 0;
    for(int y = 0;y < height;y++) {
      for(int x = 0;x < width;x++, pixel += bpp, loc++) {
        if(red_channel[loc] == 0xff && green_channel[loc] == 0xff && blue_channel[loc] == 0xff)
        {
          continue;
        }
        Uint32 mapped = 0;
        switch(bpp) {
          case 1:
            mapped = *pixel;
            break;
          case 2:
            mapped = *(Uint16 *)pixel;
            break;
          case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            mapped |= pixel[0] << 16;
            mapped |= pixel[1] << 8;
            mapped |= pixel[2] << 0;
#else
            mapped |= pixel[0] << 0;
            mapped |= pixel[1] << 8;
            mapped |= pixel[2] << 16;
#endif
            break;
          case 4:
            mapped = *(Uint32 *)pixel;
            break;
        }
        Uint8 red, green, blue, alpha;
        SDL_GetRGBA(mapped, screen->format, &red, &green, &blue, &alpha);
        red = (red * red_channel[loc]) >> 8;
        green = (green * green_channel[loc]) >> 8;
        blue = (blue * blue_channel[loc]) >> 8;
        mapped = SDL_MapRGBA(screen->format, red, green, blue, alpha);
        switch(bpp) {
          case 1:
            *pixel = mapped;
            break;
          case 2:
            *(Uint16 *)pixel = mapped;
            break;
          case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            pixel[0] = (mapped >> 16) & 0xff;
            pixel[1] = (mapped >> 8) & 0xff;
            pixel[2] = (mapped >> 0) & 0xff;
#else
            pixel[0] = (mapped >> 0) & 0xff;
            pixel[1] = (mapped >> 8) & 0xff;
            pixel[2] = (mapped >> 16) & 0xff;
#endif
            break;
          case 4:
            *(Uint32 *)pixel = mapped;
            break;
        }
      }
      pixel += screen->pitch - width * bpp;
    }
  }

  void Lightmap::light_blit(SDL_Surface *src, SDL_Rect *src_rect, int dstx, int dsty)
  {
    int bpp = src->format->BytesPerPixel;
      Uint8 *pixel = (Uint8 *) src->pixels + src_rect->y * src->pitch + src_rect->x * bpp;
    int loc = dsty * width + dstx;
    for(int y = 0;y < src_rect->h;y++) {
      for(int x = 0;x < src_rect->w;x++, pixel += bpp, loc++) {
        if(x + dstx < 0 || y + dsty < 0 || x + dstx >= width || y + dsty >= height)
        {
          continue;
        }
        if(red_channel[loc] == 0xff && green_channel[loc] == 0xff && blue_channel[loc] == 0xff)
        {
          continue;
        }

        Uint32 mapped = 0;
        switch(bpp) {
          case 1:
            mapped = *pixel;
            break;
          case 2:
            mapped = *(Uint16 *)pixel;
            break;
          case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            mapped |= pixel[0] << 16;
            mapped |= pixel[1] << 8;
            mapped |= pixel[2] << 0;
#else
            mapped |= pixel[0] << 0;
            mapped |= pixel[1] << 8;
            mapped |= pixel[2] << 16;
#endif
            break;
          case 4:
            mapped = *(Uint32 *)pixel;
            break;
        }
        Uint8 red, green, blue, alpha;
        SDL_GetRGBA(mapped, src->format, &red, &green, &blue, &alpha);

        if(red != 0)
        {
          int redsum = red_channel[loc] + (red * alpha >> 8);
          red_channel[loc] = redsum & ~0xff ? 0xff : redsum;
        }
        if(green != 0)
        {
          int greensum = green_channel[loc] + (green * alpha >> 8);
          green_channel[loc] = greensum & ~0xff ? 0xff : greensum;
        }
        if(blue != 0)
        {
          int bluesum = blue_channel[loc] + (blue * alpha >> 8);
          blue_channel[loc] = bluesum & ~0xff ? 0xff : bluesum;
        }
      }
      pixel += src->pitch - src_rect->w * bpp;
      loc += width - src_rect->w;
    }
  }

  void
  Lightmap::draw_surface(const DrawingRequest& request)
  {
    if((request.color.red == 0.0 && request.color.green == 0.0 && request.color.blue == 0.0) || request.color.alpha == 0.0 || request.alpha == 0.0)
    {
      return;
    }
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
    int dstx = (int) request.pos.x * numerator / denominator;
    int dsty = (int) request.pos.y * numerator / denominator;
    light_blit(transform, src_rect, dstx, dsty);
  }

  void
  Lightmap::draw_surface_part(const DrawingRequest& request)
  {
    const SurfacePartRequest* surfacepartrequest
      = (SurfacePartRequest*) request.request_data;

    const Surface* surface = surfacepartrequest->surface;
    SDL::Texture *sdltexture = dynamic_cast<SDL::Texture *>(surface->get_texture());

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

    SDL_Rect src_rect;
    src_rect.x = (ox + (int) surfacepartrequest->source.x) * numerator / denominator;
    src_rect.y = (oy + (int) surfacepartrequest->source.y) * numerator / denominator;
    src_rect.w = (int) surfacepartrequest->size.x * numerator / denominator;
    src_rect.h = (int) surfacepartrequest->size.y * numerator / denominator;
    int dstx = (int) request.pos.x * numerator / denominator;
    int dsty = (int) request.pos.y * numerator / denominator;
    light_blit(transform, &src_rect, dstx, dsty);
  }

  void
  Lightmap::draw_gradient(const DrawingRequest& request)
  {
    const GradientRequest* gradientrequest 
      = (GradientRequest*) request.request_data;
    const Color& top = gradientrequest->top;
    const Color& bottom = gradientrequest->bottom;

    for(int y = 0;y < height;++y)
    {
      Uint8 r = (Uint8)((((float)(top.red-bottom.red)/(0-height)) * y + top.red) * 255);
      Uint8 g = (Uint8)((((float)(top.green-bottom.green)/(0-height)) * y + top.green) * 255);
      Uint8 b = (Uint8)((((float)(top.blue-bottom.blue)/(0-height)) * y + top.blue) * 255);
      // FIXME
      //Uint8 a = (Uint8)((((float)(top.alpha-bottom.alpha)/(0-height)) * y + top.alpha) * 255);
      for(int x = 0;x < width;x++) {
        int loc = y * width + x;
        red_channel[loc] = std::min(red_channel[loc] + r, 255);
        green_channel[loc] = std::min(green_channel[loc] + g, 255);
        blue_channel[loc] = std::min(blue_channel[loc] + b, 255);
      }
    }
  }

  void
  Lightmap::draw_text(const DrawingRequest& /*request*/)
  {
    //const TextRequest* textrequest = (TextRequest*) request.request_data;

    //textrequest->font->draw(textrequest->text, request.pos,
    //    textrequest->alignment, request.drawing_effect, request.alpha);
  }

  void
  Lightmap::draw_filled_rect(const DrawingRequest& request)
  {
    const FillRectRequest* fillrectrequest
      = (FillRectRequest*) request.request_data;

    int rect_x = (int) (request.pos.x * width / SCREEN_WIDTH);
    int rect_y = (int) (request.pos.y * height / SCREEN_HEIGHT);
    int rect_w = (int) (fillrectrequest->size.x * width / SCREEN_WIDTH);
    int rect_h = (int) (fillrectrequest->size.y * height / SCREEN_HEIGHT);
    Uint8 red = (Uint8) (fillrectrequest->color.red * fillrectrequest->color.alpha * 255);
    Uint8 green = (Uint8) (fillrectrequest->color.green * fillrectrequest->color.alpha * 255);
    Uint8 blue = (Uint8) (fillrectrequest->color.blue * fillrectrequest->color.alpha * 255);
    if(red == 0 && green == 0 && blue == 0)
    {
      return;
    }
    for(int y = rect_y;y < rect_y + rect_h;y++) {
      for(int x = rect_x;x < rect_x + rect_w;x++) {
        int loc = y * width + x;
        red_channel[loc] = std::min(red_channel[loc] + red, 255);
        green_channel[loc] = std::min(green_channel[loc] + green, 255);
        blue_channel[loc] = std::min(blue_channel[loc] + blue, 255);
      }
    }
  }

  void
  Lightmap::get_light(const DrawingRequest& request) const
  {
    const GetLightRequest* getlightrequest 
      = (GetLightRequest*) request.request_data;

    int x = (int) (request.pos.x * width / SCREEN_WIDTH);
    int y = (int) (request.pos.y * height / SCREEN_HEIGHT);
    int loc = y * width + x;
    *(getlightrequest->color_ptr) = Color(((float)red_channel[loc])/255, ((float)green_channel[loc])/255, ((float)blue_channel[loc])/255);
  }
}
