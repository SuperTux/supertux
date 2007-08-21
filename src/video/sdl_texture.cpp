//  $Id: sdl_texture.cpp 4063 2006-07-21 21:05:23Z anmaster $
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

#include "sdl_texture.hpp"
#include "color.hpp"
#include "gameconfig.hpp"
#include "main.hpp"

#include <assert.h>

#include <SDL.h>

namespace
{
#define BILINEAR

#ifdef NAIVE
  SDL_Surface *scale(SDL_Surface *src, int numerator, int denominator)
  {
    if(numerator == denominator)
    {
      src->refcount++;
      return src;
    }
    else
    {
      SDL_Surface *dst = SDL_CreateRGBSurface(src->flags, src->w * numerator / denominator, src->h * numerator / denominator, src->format->BitsPerPixel, src->format->Rmask,  src->format->Gmask, src->format->Bmask, src->format->Amask);
      int bpp = dst->format->BytesPerPixel;
      for(int y = 0;y < dst->h;y++) {
        for(int x = 0;x < dst->w;x++) {
          Uint8 *srcpixel = (Uint8 *) src->pixels + (y * denominator / numerator) * src->pitch + (x * denominator / numerator) * bpp;
          Uint8 *dstpixel = (Uint8 *) dst->pixels + y * dst->pitch + x * bpp;
          switch(bpp) {
            case 4:
              dstpixel[3] = srcpixel[3];
            case 3:
              dstpixel[2] = srcpixel[2];
            case 2:
              dstpixel[1] = srcpixel[1];
            case 1:
              dstpixel[0] = srcpixel[0];
          }
        }
      }
      return dst;
    }
  }
#endif

#ifdef BILINEAR
  void getpixel(SDL_Surface *src, int srcx, int srcy, Uint8 color[4])
  {
    int bpp = src->format->BytesPerPixel;
    if(srcx == src->w)
    {
      srcx--;
    }
    if(srcy == src->h)
    {
      srcy--;
    }
    Uint8 *srcpixel = (Uint8 *) src->pixels + srcy * src->pitch + srcx * bpp;
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
    SDL_GetRGBA(mapped, src->format, &color[0], &color[1], &color[2], &color[3]);
  }

  void merge(Uint8 color[4], Uint8 color0[4], Uint8 color1[4], int rem, int total)
  {
    color[0] = (color0[0] * (total - rem) + color1[0] * rem) / total;
    color[1] = (color0[1] * (total - rem) + color1[1] * rem) / total;
    color[2] = (color0[2] * (total - rem) + color1[2] * rem) / total;
    color[3] = (color0[3] * (total - rem) + color1[3] * rem) / total;
  }

  SDL_Surface *scale(SDL_Surface *src, int numerator, int denominator)
  {
    if(numerator == denominator)
    {
      src->refcount++;
      return src;
    }
    else
    {
      SDL_Surface *dst = SDL_CreateRGBSurface(src->flags, src->w * numerator / denominator, src->h * numerator / denominator, src->format->BitsPerPixel, src->format->Rmask,  src->format->Gmask, src->format->Bmask, src->format->Amask);
      int bpp = dst->format->BytesPerPixel;
      for(int y = 0;y < dst->h;y++) {
        for(int x = 0;x < dst->w;x++) {
          int srcx = x * denominator / numerator;
          int srcy = y * denominator / numerator;
          Uint8 color00[4], color01[4], color10[4], color11[4];
          getpixel(src, srcx, srcy, color00);
          getpixel(src, srcx + 1, srcy, color01);
          getpixel(src, srcx, srcy + 1, color10);
          getpixel(src, srcx + 1, srcy + 1, color11);
          Uint8 color0[4], color1[4], color[4];
          int remx = x * denominator % numerator;
          merge(color0, color00, color01, remx, numerator);
          merge(color1, color10, color11, remx, numerator);
          int remy = y * denominator % numerator;
          merge(color, color0, color1, remy, numerator);
          Uint8 *dstpixel = (Uint8 *) dst->pixels + y * dst->pitch + x * bpp;
          Uint32 mapped = SDL_MapRGBA(dst->format, color[0], color[1], color[2], color[3]);
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
      return dst;
    }
  }
#endif

  // FIXME: Horizontal and vertical line problem
#ifdef BRESENHAM
  void accumulate(SDL_Surface *src, int srcx, int srcy, int color[4], int weight)
  {
    if(srcx < 0 || srcy < 0 || weight == 0) {
      return;
    }
    int bpp = src->format->BytesPerPixel;
    Uint8 *srcpixel = (Uint8 *) src->pixels + srcy * src->pitch + srcx * bpp;
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
    Uint8 red, green, blue, alpha;
    SDL_GetRGBA(mapped, src->format, &red, &green, &blue, &alpha);
    color[0] += red * weight;
    color[1] += green * weight;
    color[2] += blue * weight;
    color[3] += alpha * weight;
  }

  void accumulate_line(SDL_Surface *src, int srcy, int line[][4], int linesize, int weight, int numerator, int denominator)
  {
    int intpart = denominator / numerator;
    int fractpart = denominator % numerator;
    for(int x = 0, xe = 0, srcx = 0;x < linesize;x++) {
      accumulate(src, srcx, srcy, line[x], (numerator - xe) * weight);
      srcx++;
      for(int i = 0;i < intpart - 1;i++) {
        accumulate(src, srcx, srcy, line[x], numerator * weight);
        srcx++;
      }
      xe += fractpart;
      if(xe >= numerator) {
        xe -= numerator;
        srcx++;
      }
      accumulate(src, srcx, srcy, line[x], xe * weight);
    }
  }

  SDL_Surface *scale(SDL_Surface *src, int numerator, int denominator)
  {
    if(numerator == denominator)
    {
      src->refcount++;
      return src;
    }
    else
    {
      SDL_Surface *dst = SDL_CreateRGBSurface(src->flags, src->w * numerator / denominator, src->h * numerator / denominator, src->format->BitsPerPixel, src->format->Rmask,  src->format->Gmask, src->format->Bmask, src->format->Amask);
      int bpp = dst->format->BytesPerPixel;
      int intpart = denominator / numerator;
      int fractpart = denominator % numerator;
      for(int y = 0, ye = 0, srcy = 0;y < dst->h;y++) {
        int line[dst->w][4];
        memset(line, 0, sizeof(int) * dst->w * 4);
        accumulate_line(src, srcy, line, dst->w, numerator - ye, numerator, denominator);
        srcy++;
        for(int i = 0;i < intpart - 1;i++) {
          accumulate_line(src, srcy, line, dst->w, numerator, numerator, denominator);
          srcy++;
        }
        ye += fractpart;
        if(ye >= numerator) {
          ye -= numerator;
          srcy++;
        }
        accumulate_line(src, srcy, line, dst->w, ye, numerator, denominator);
        for(int x = 0;x < dst->w;x++) {
          Uint8 *dstpixel = (Uint8 *) dst->pixels + y * dst->pitch + x * bpp;
          Uint32 mapped = SDL_MapRGBA(dst->format, line[x][0] / (denominator * denominator), line[x][1] / (denominator * denominator), line[x][2] / (denominator * denominator), line[x][3] / (denominator * denominator));
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
      return dst;
    }
  }
#endif

  SDL_Surface *horz_flip(SDL_Surface *src)
  {
    SDL_Surface *dst = SDL_CreateRGBSurface(src->flags, src->w, src->h, src->format->BitsPerPixel, src->format->Rmask,  src->format->Gmask, src->format->Bmask, src->format->Amask);
    int bpp = dst->format->BytesPerPixel;
    for(int y = 0;y < dst->h;y++) {
      for(int x = 0;x < dst->w;x++) {
        Uint8 *srcpixel = (Uint8 *) src->pixels + y * src->pitch + x * bpp;
        Uint8 *dstpixel = (Uint8 *) dst->pixels + y * dst->pitch + (dst->w - x - 1) * bpp;
        switch(bpp) {
          case 4:
            dstpixel[3] = srcpixel[3];
          case 3:
            dstpixel[2] = srcpixel[2];
          case 2:
            dstpixel[1] = srcpixel[1];
          case 1:
            dstpixel[0] = srcpixel[0];
        }
      }
    }
    return dst;
  }

  SDL_Surface *vert_flip(SDL_Surface *src)
  {
    SDL_Surface *dst = SDL_CreateRGBSurface(src->flags, src->w, src->h, src->format->BitsPerPixel, src->format->Rmask,  src->format->Gmask, src->format->Bmask, src->format->Amask);
    int bpp = dst->format->BytesPerPixel;
    for(int y = 0;y < dst->h;y++) {
      for(int x = 0;x < dst->w;x++) {
        Uint8 *srcpixel = (Uint8 *) src->pixels + y * src->pitch + x * bpp;
        Uint8 *dstpixel = (Uint8 *) dst->pixels + (dst->h - y - 1) * dst->pitch + x * bpp;
        switch(bpp) {
          case 4:
            dstpixel[3] = srcpixel[3];
          case 3:
            dstpixel[2] = srcpixel[2];
          case 2:
            dstpixel[1] = srcpixel[1];
          case 1:
            dstpixel[0] = srcpixel[0];
        }
      }
    }
    return dst;
  }

  SDL_Surface *colorize(SDL_Surface *src, const Color &color)
  {
    // FIXME: This is really slow
    assert(color.red != 1.0 || color.green != 1.0 || color.blue != 1.0);
    SDL_Surface *dst = SDL_CreateRGBSurface(src->flags, src->w, src->h, src->format->BitsPerPixel, src->format->Rmask,  src->format->Gmask, src->format->Bmask, src->format->Amask);
    int bpp = dst->format->BytesPerPixel;
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
        Uint8 red, green, blue, alpha;
        SDL_GetRGBA(mapped, src->format, &red, &green, &blue, &alpha);
        red = (Uint8) (red * color.red);
        green = (Uint8) (green * color.green);
        blue = (Uint8) (blue * color.blue);
        mapped = SDL_MapRGBA(dst->format, red, green, blue, alpha);
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
    return dst;
  }
}

namespace SDL
{
  Texture::Texture(SDL_Surface* image)
  {
    if(!(image->flags & SDL_SRCALPHA) || !image->format->Amask)
    {
      texture = SDL_DisplayFormat(image);
    }
    else
    {
      texture = SDL_DisplayFormatAlpha(image);
    }
    //width = texture->w;
    //height = texture->h;
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
    cache[NO_EFFECT][Color::WHITE] = scale(texture, numerator, denominator);
  }

  Texture::~Texture()
  {
    SDL_FreeSurface(texture);
  }

  SDL_Surface *Texture::get_transform(const Color &color, DrawingEffect effect)
  {
    if(cache[NO_EFFECT][color] == 0) {
      assert(cache[NO_EFFECT][Color::WHITE]);
      cache[NO_EFFECT][color] = colorize(cache[NO_EFFECT][Color::WHITE], color);
    }
    if(cache[effect][color] == 0) {
      assert(cache[NO_EFFECT][color]);
      switch(effect) {
        case NO_EFFECT:
          break;
        case HORIZONTAL_FLIP:
          cache[HORIZONTAL_FLIP][color] = horz_flip(cache[NO_EFFECT][color]);
          break;
        case VERTICAL_FLIP:
          cache[VERTICAL_FLIP][color] = vert_flip(cache[NO_EFFECT][color]);
          break;
        default:
          return 0;
      }
    }
    return cache[effect][color];
  }
}
