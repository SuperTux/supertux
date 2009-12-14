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

#include <iostream>

#include "video/sdl/sdl_lightmap.hpp"
#include "video/sdl/sdl_surface_data.hpp"
#include "video/sdl/sdl_texture.hpp"

SDLLightmap::SDLLightmap() :
  screen(),
  red_channel(),
  blue_channel(),
  green_channel(),
  width(),
  height(),
  numerator(),
  denominator(),
  LIGHTMAP_DIV()
{
  screen = SDL_GetVideoSurface();

  //float xfactor = 1.0f; // FIXME: (float) config->screenwidth / SCREEN_WIDTH;
  //float yfactor = 1.0f; // FIXME: (float) config->screenheight / SCREEN_HEIGHT;

  numerator = 1;
  denominator = 1;

  /* FIXME:
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

  LIGHTMAP_DIV = 8 * numerator / denominator;

  width = screen->w / LIGHTMAP_DIV;
  height = screen->h / LIGHTMAP_DIV;

  red_channel = (Uint8 *)malloc(width * height * sizeof(Uint8));
  green_channel = (Uint8 *)malloc(width * height * sizeof(Uint8));
  blue_channel = (Uint8 *)malloc(width * height * sizeof(Uint8));
}

SDLLightmap::~SDLLightmap()
{
  free(red_channel);
  free(green_channel);
  free(blue_channel);
}

void
SDLLightmap::start_draw(const Color &ambient_color)
{
  memset(red_channel, (Uint8) (ambient_color.red * 255), width * height * sizeof(Uint8));
  memset(green_channel, (Uint8) (ambient_color.green * 255), width * height * sizeof(Uint8));
  memset(blue_channel, (Uint8) (ambient_color.blue * 255), width * height * sizeof(Uint8));
}

void
SDLLightmap::end_draw()
{
}

//#define BILINEAR

#ifdef BILINEAR
namespace {

void merge(Uint8 color[3], Uint8 color0[3], Uint8 color1[3], int rem, int total)
{
  color[0] = (color0[0] * (total - rem) + color1[0] * rem) / total;
  color[1] = (color0[1] * (total - rem) + color1[1] * rem) / total;
  color[2] = (color0[2] * (total - rem) + color1[2] * rem) / total;
}

} // namespace
#endif

void
SDLLightmap::do_draw()
{
  // FIXME: This is really slow
  if(LIGHTMAP_DIV == 1)
  {
    int bpp = screen->format->BytesPerPixel;
    if(SDL_MUSTLOCK(screen))
    {
      SDL_LockSurface(screen);
    }
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
    if(SDL_MUSTLOCK(screen))
    {
      SDL_UnlockSurface(screen);
    }
  }
  else
  {
    int bpp = screen->format->BytesPerPixel;
    if(SDL_MUSTLOCK(screen))
    {
      SDL_LockSurface(screen);
    }
    Uint8 *div_pixel = (Uint8 *) screen->pixels;
    int loc = 0;
    for(int y = 0;y < height;y++) {
      for(int x = 0;x < width;x++, div_pixel += bpp * LIGHTMAP_DIV, loc++) {
        if(red_channel[loc] == 0xff && green_channel[loc] == 0xff && blue_channel[loc] == 0xff)
        {
          continue;
        }
        Uint8 *pixel = div_pixel;
        for(int div_y = 0;div_y < LIGHTMAP_DIV;div_y++) {
          for(int div_x = 0;div_x < LIGHTMAP_DIV;pixel += bpp, div_x++) {
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

#ifdef BILINEAR
            int xinc = (x + 1 != width ? 1 : 0);
            int yinc = (y + 1 != height ? width : 0);
            Uint8 color00[3], color01[3], color10[3], color11[3];
            {
              color00[0] = red_channel[loc];
              color00[1] = green_channel[loc];
              color00[2] = blue_channel[loc];
            }
            {
              color01[0] = red_channel[loc + xinc];
              color01[1] = green_channel[loc + xinc];
              color01[2] = blue_channel[loc + xinc];
            }
            {
              color10[0] = red_channel[loc + yinc];
              color10[1] = green_channel[loc + yinc];
              color10[2] = blue_channel[loc + yinc];
            }
            {
              color11[0] = red_channel[loc + yinc + xinc];
              color11[1] = green_channel[loc + yinc + xinc];
              color11[2] = blue_channel[loc + yinc + xinc];
            }
            Uint8 color0[3], color1[3], color[3];
            merge(color0, color00, color01, div_x, LIGHTMAP_DIV);
            merge(color1, color10, color11, div_x, LIGHTMAP_DIV);
            merge(color, color0, color1, div_y, LIGHTMAP_DIV);
            red = (red * color[0]) >> 8;
            green = (green * color[1]) >> 8;
            blue = (blue * color[2]) >> 8;
#else
            red = (red * red_channel[loc]) >> 8;
            green = (green * green_channel[loc]) >> 8;
            blue = (blue * blue_channel[loc]) >> 8;
#endif

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
          pixel += screen->pitch - LIGHTMAP_DIV * bpp;
        }
      }
      div_pixel += (screen->pitch - width * bpp) * LIGHTMAP_DIV;
    }
    if(SDL_MUSTLOCK(screen))
    {
      SDL_UnlockSurface(screen);
    }
  }
}

void
SDLLightmap::light_blit(SDL_Surface *src, SDL_Rect *src_rect, int dstx, int dsty)
{
  dstx /= LIGHTMAP_DIV;
  dsty /= LIGHTMAP_DIV;
  int srcx = src_rect->x / LIGHTMAP_DIV;
  int srcy = src_rect->y / LIGHTMAP_DIV;
  int blit_width = src_rect->w / LIGHTMAP_DIV;
  int blit_height = src_rect->h / LIGHTMAP_DIV;
  int bpp = src->format->BytesPerPixel;
  if(SDL_MUSTLOCK(src))
  {
    SDL_LockSurface(src);
  }
  Uint8 *pixel = (Uint8 *) src->pixels + srcy * src->pitch + srcx * bpp;
  int loc = dsty * width + dstx;
  for(int y = 0;y < blit_height;y++) {
    for(int x = 0;x < blit_width;x++, pixel += bpp * LIGHTMAP_DIV, loc++) {
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
    pixel += (src->pitch - blit_width * bpp) * LIGHTMAP_DIV;
    loc += width - blit_width;
  }
  if(SDL_MUSTLOCK(src))
  {
    SDL_UnlockSurface(src);
  }
}

/*void Lightmap::light_blit(SDL_Surface *src, SDL_Rect *src_rect, int dstx, int dsty)
  {
  int bpp = src->format->BytesPerPixel;
  if(SDL_MUSTLOCK(src))
  {
  SDL_LockSurface(src);
  }
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
  if(SDL_MUSTLOCK(src))
  {
  SDL_UnlockSurface(src);
  }
  }*/

void
SDLLightmap::draw_surface(const DrawingRequest& request)
{
  if((request.color.red == 0.0 && request.color.green == 0.0 && request.color.blue == 0.0) || request.color.alpha == 0.0 || request.alpha == 0.0)
  {
    return;
  }
  //FIXME: support parameters request.alpha, request.angle, request.blend
 
  const Surface* surface = (const Surface*) request.request_data;
  boost::shared_ptr<SDLTexture> sdltexture = boost::dynamic_pointer_cast<SDLTexture>(surface->get_texture());
  SDLSurfaceData *surface_data = reinterpret_cast<SDLSurfaceData *>(surface->get_surface_data());

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
SDLLightmap::draw_surface_part(const DrawingRequest& request)
{
  const SurfacePartRequest* surfacepartrequest
    = (SurfacePartRequest*) request.request_data;

  const Surface* surface = surfacepartrequest->surface;
  boost::shared_ptr<SDLTexture> sdltexture = boost::dynamic_pointer_cast<SDLTexture>(surface->get_texture());

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
SDLLightmap::draw_gradient(const DrawingRequest& request)
{
  const GradientRequest* gradientrequest 
    = (GradientRequest*) request.request_data;
  const Color& top = gradientrequest->top;
  const Color& bottom = gradientrequest->bottom;

  int loc = 0;
  for(int y = 0;y < height;++y)
  {
    Uint8 red = (Uint8)((((float)(top.red-bottom.red)/(0-height)) * y + top.red) * 255);
    Uint8 green = (Uint8)((((float)(top.green-bottom.green)/(0-height)) * y + top.green) * 255);
    Uint8 blue = (Uint8)((((float)(top.blue-bottom.blue)/(0-height)) * y + top.blue) * 255);
    Uint8 alpha = (Uint8)((((float)(top.alpha-bottom.alpha)/(0-height)) * y + top.alpha) * 255);
    for(int x = 0;x < width;x++, loc++) {
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
  }
}

void
SDLLightmap::draw_filled_rect(const DrawingRequest& request)
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
      if(red != 0)
      {
        int redsum = red_channel[loc] + red;
        red_channel[loc] = redsum & ~0xff ? 0xff : redsum;
      }
      if(green != 0)
      {
        int greensum = green_channel[loc] + green;
        green_channel[loc] = greensum & ~0xff ? 0xff : greensum;
      }
      if(blue != 0)
      {
        int bluesum = blue_channel[loc] + blue;
        blue_channel[loc] = bluesum & ~0xff ? 0xff : bluesum;
      }
    }
  }
}

void
SDLLightmap::get_light(const DrawingRequest& request) const
{
  const GetLightRequest* getlightrequest 
    = (GetLightRequest*) request.request_data;

  int x = (int) (request.pos.x * width / SCREEN_WIDTH);
  int y = (int) (request.pos.y * height / SCREEN_HEIGHT);
  int loc = y * width + x;
  *(getlightrequest->color_ptr) = Color(((float)red_channel[loc])/255, ((float)green_channel[loc])/255, ((float)blue_channel[loc])/255);
}

/* EOF */
