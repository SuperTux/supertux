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

#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "video/color.hpp"
#include "video/sdl/sdl_texture.hpp"

#include <assert.h>

#include <SDL.h>

namespace {
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
    if(SDL_MUSTLOCK(dst))
    {
      SDL_UnlockSurface(dst);
    }
    if(SDL_MUSTLOCK(src))
    {
      SDL_UnlockSurface(src);
    }
    if(!src->format->Amask)
    {
      if(src->flags & SDL_SRCALPHA)
      {
        SDL_SetAlpha(dst, SDL_SRCALPHA | SDL_RLEACCEL, src->format->alpha);
      }
      if(src->flags & SDL_SRCCOLORKEY)
      {
        SDL_SetColorKey(dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, src->format->colorkey);
      }
    }
    return dst;
  }
} // namespace
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
    if(SDL_MUSTLOCK(dst))
    {
      SDL_UnlockSurface(dst);
    }
    if(SDL_MUSTLOCK(src))
    {
      SDL_UnlockSurface(src);
    }
    if(!src->format->Amask)
    {
      if(src->flags & SDL_SRCALPHA)
      {
        SDL_SetAlpha(dst, SDL_SRCALPHA | SDL_RLEACCEL, src->format->alpha);
      }
      if(src->flags & SDL_SRCCOLORKEY)
      {
        SDL_SetColorKey(dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, src->format->colorkey);
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
  if(SDL_MUSTLOCK(dst))
  {
    SDL_UnlockSurface(dst);
  }
  if(SDL_MUSTLOCK(src))
  {
    SDL_UnlockSurface(src);
  }
  if(!src->format->Amask)
  {
    if(src->flags & SDL_SRCALPHA)
    {
      SDL_SetAlpha(dst, SDL_SRCALPHA | SDL_RLEACCEL, src->format->alpha);
    }
    if(src->flags & SDL_SRCCOLORKEY)
    {
      SDL_SetColorKey(dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, src->format->colorkey);
    }
  }
  return dst;
}

SDL_Surface *vert_flip(SDL_Surface *src)
{
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
  if(SDL_MUSTLOCK(dst))
  {
    SDL_UnlockSurface(dst);
  }
  if(SDL_MUSTLOCK(src))
  {
    SDL_UnlockSurface(src);
  }
  if(!src->format->Amask)
  {
    if(src->flags & SDL_SRCALPHA)
    {
      SDL_SetAlpha(dst, SDL_SRCALPHA | SDL_RLEACCEL, src->format->alpha);
    }
    if(src->flags & SDL_SRCCOLORKEY)
    {
      SDL_SetColorKey(dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, src->format->colorkey);
    }
  }
  return dst;
}

SDL_Surface *colorize(SDL_Surface *src, const Color &color)
{
  // FIXME: This is really slow
  assert(color.red != 1.0 || color.green != 1.0 || color.blue != 1.0);
  int red = (int) (color.red * 256);
  int green = (int) (color.green * 256);
  int blue = (int) (color.blue * 256);
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
      if(src->format->Amask || !(src->flags & SDL_SRCCOLORKEY) || mapped != src->format->colorkey)
      {
        Uint8 r, g, b, a;
        SDL_GetRGBA(mapped, src->format, &r, &g, &b, &a);
        mapped = SDL_MapRGBA(dst->format, (r * red) >> 8, (g * green) >> 8, (b * blue) >> 8, a);
      }
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
  if(!src->format->Amask)
  {
    if(src->flags & SDL_SRCALPHA)
    {
      SDL_SetAlpha(dst, SDL_SRCALPHA | SDL_RLEACCEL, src->format->alpha);
    }
    if(src->flags & SDL_SRCCOLORKEY)
    {
      SDL_SetColorKey(dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, src->format->colorkey);
    }
  }
  return dst;
}

SDL_Surface *optimize(SDL_Surface *src)
{
  if(!src->format->Amask)
  {
    return SDL_DisplayFormat(src);
  }
  else
  {
    int bpp;
    bool colors[(1 << 12)];
    memset(colors, 0, (1 << 12) * sizeof(bool));
#if 0
    int transparent = 0;
    int opaque = 0;
    int semitransparent = 0;
    int alphasum = 0;
    int squaredalphasum = 0;

    int bpp = src->format->BytesPerPixel;
    if(SDL_MUSTLOCK(src))
    {
      SDL_LockSurface(src);
    }
    for(int y = 0;y < src->h;y++) {
      for(int x = 0;x < src->w;x++) {
        Uint8 *pixel = (Uint8 *) src->pixels + y * src->pitch + x * bpp;
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
        if(alpha < 16)
        {
          transparent++;
        }
        else if (alpha > 240)
        {
          opaque++;
          alphasum += alpha;
          squaredalphasum += alpha * alpha;
        }
        else
        {
          semitransparent++;
          squaredalphasum += alpha * alpha;
        }
        if(alpha != 0)
        {
          colors[((red & 0xf0) << 4) | (green & 0xf0) | ((blue & 0xf0) >> 4)] = true;
        }
      }
    }
    if(SDL_MUSTLOCK(src))
    {
      SDL_UnlockSurface(src);
    }
    int avgalpha = (opaque + semitransparent) ? alphasum / (opaque + semitransparent) : 0;
    int avgsquaredalpha = (opaque + semitransparent) ? squaredalphasum / (opaque + semitransparent) : 0;
    int alphavariance = avgsquaredalpha - avgalpha * avgalpha;
    if(semitransparent > ((transparent + opaque + semitransparent) / 8) && alphavariance > 16)
    {
      return SDL_DisplayFormatAlpha(src);
    }
#endif
    int keycolor = -1;
    for(int i = 0;i < (1 << 12);i++)
    {
      if(!colors[i])
      {
        keycolor = i;
      }
    }
    if(keycolor == -1)
    {
      return SDL_DisplayFormatAlpha(src);
    }
    SDL_Surface *dst = SDL_CreateRGBSurface(src->flags & ~(SDL_SRCALPHA), src->w, src->h, src->format->BitsPerPixel, src->format->Rmask,  src->format->Gmask, src->format->Bmask, 0);
    bpp = dst->format->BytesPerPixel;
    Uint32 key = SDL_MapRGB(dst->format, (((keycolor & 0xf00) >> 4) | 0xf), ((keycolor & 0xf0) | 0xf), (((keycolor & 0xf) << 4) | 0xf));
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
        Uint8 red, green, blue, alpha;
        SDL_GetRGBA(mapped, src->format, &red, &green, &blue, &alpha);
        //if(alpha < (avgalpha / 4))
        if(alpha < 8)
        {
          mapped = key;
        }
        else
        {
          mapped = SDL_MapRGB(dst->format, red, green, blue);
        }
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
    /*
    if(avgalpha < 240)
    {
      SDL_SetAlpha(dst, SDL_SRCALPHA | SDL_RLEACCEL, avgalpha);
    }
    */
    SDL_SetColorKey(dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, key);
    SDL_Surface *convert = SDL_DisplayFormat(dst);
    SDL_FreeSurface(dst);
    return convert;
  }
}
}

SDLTexture::SDLTexture(SDL_Surface* image) :
  texture()
{
  texture = optimize(image);
  //width = texture->w;
  //height = texture->h;
  int numerator   = 1;
  int denominator = 1;
  //FIXME: float xfactor = (float) config->screenwidth / SCREEN_WIDTH;
  //FIXME: float yfactor = (float) config->screenheight / SCREEN_HEIGHT;
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
  cache[NO_EFFECT][Color::WHITE] = scale(texture, numerator, denominator);
}

SDLTexture::~SDLTexture()
{
  SDL_FreeSurface(texture);
}

SDL_Surface*
SDLTexture::get_transform(const Color &color, DrawingEffect effect)
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

/* EOF */
