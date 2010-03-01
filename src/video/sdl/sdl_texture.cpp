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
#include "util/log.hpp"
#include "math/random_generator.hpp"

#include <assert.h>

#include <SDL.h>

namespace {
#define BILINEAR

static Uint32 get_pixel_mapping (SDL_Surface *src, void *pixel)
{
  Uint32 mapped = 0;

  switch (src->format->BytesPerPixel)
  {
    case 1:
      mapped = *((Uint8 *) pixel);
      break;
    case 2:
      mapped = *((Uint16 *) pixel);
      break;
    case 3:
    {
      Uint8 *tmp = (Uint8 *) pixel;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
      mapped |= tmp[0] << 16;
      mapped |= tmp[1] << 8;
      mapped |= tmp[2] << 0;
#else
      mapped |= tmp[0] << 0;
      mapped |= tmp[1] << 8;
      mapped |= tmp[2] << 16;
#endif
      break;
    }
    case 4:
      mapped = *((Uint32 *) pixel);
      break;

    default:
      log_warning << "Unknown BytesPerPixel value: "
        << src->format->BytesPerPixel << std::endl;
      mapped = 0;
  } /* switch (bpp) */

  return (mapped);
} /* Uint32 get_pixel_mapping */

static Uint32 get_random_color (SDL_Surface *src)
{
  Uint32 r;

  r = (Uint32) graphicsRandom.rand ();
  /* rand() returns 31bit random numbers. So call it twice to get full 32 bit. */
  r <<= 1;
  r |= (Uint32) graphicsRandom.rand ();

  switch (src->format->BytesPerPixel)
  {
    case 1:
      r &= 0x000000ff;
      break;

    case 2:
      r &= 0x0000ffff;
      break;

    case 3:
      r &= 0x0000ffff;
      break;
  }

  return (r);
} /* Uint32 get_random_color */

static bool color_is_used (SDL_Surface *src, Uint32 color)
{
  if(SDL_MUSTLOCK(src))
    SDL_LockSurface(src);

  for(int y = 0; y < src->h; y++) {
    for(int x = 0; x < src->w; x++) {
      Uint8 *pixel = (Uint8 *) src->pixels
        + (y * src->pitch) + (x * src->format->BytesPerPixel);
      Uint32 mapped = get_pixel_mapping (src, pixel);

      if (color == mapped)
        return (true);
    }
  }

  return (false);
} /* bool color_is_used */

static Uint32 get_unused_color (SDL_Surface *src)
{
  Uint32 random_color;

  do
  {
    random_color = get_random_color (src);
  } while (color_is_used (src, random_color));

  return (random_color);
} /* Uint32 get_unused_color */

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
  Uint32 mapped = get_pixel_mapping (src, srcpixel);
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

/** Optimizes a SDL_Surface surface and returns it in the "display format". If
 *  the surface does not have an alpha channel, simply calls
 *  "SDL_DisplayFormat". If the surface has an alpha channel, examines all the
 *  pixels. If in fact semi-transparent pixels are found, calls
 *  "SDL_DisplayFormatAlpha". If only fully transparent and fully opaque pixels
 *  are found, converts the surface to a 1-bit alpha surface with
 *  "SDL_SetColorKey". */
SDL_Surface *optimize(SDL_Surface *src)
{
  bool have_transparent = false;
  bool have_semi_trans = false;
  bool have_opaque = false;

  if(!src->format->Amask)
    return SDL_DisplayFormat(src);

  if(SDL_MUSTLOCK(src))
    SDL_LockSurface(src);

  /* Iterate over all the pixels and record which ones we found. */
  for(int y = 0; y < src->h; y++) {
    for(int x = 0; x < src->w; x++) {
      Uint8 *pixel = (Uint8 *) src->pixels
        + (y * src->pitch) + (x * src->format->BytesPerPixel);
      Uint32 mapped = get_pixel_mapping (src, pixel);
      Uint8 red, green, blue, alpha;
      SDL_GetRGBA(mapped, src->format, &red, &green, &blue, &alpha);

      if (alpha < 16)
        have_transparent = true;
      else if (alpha > 240)
        have_opaque = true;
      else
        have_semi_trans = true;
    } /* for (x) */
  } /* for (y) */

  if(SDL_MUSTLOCK(src))
    SDL_UnlockSurface(src);

  if (have_semi_trans)
    return SDL_DisplayFormatAlpha(src);

  if (!have_transparent /* && !have_semi_trans */)
    return SDL_DisplayFormat(src);

  /* The surface is totally transparent. We shouldn't return a surface at all,
   * but since the calling code can't cope with that, use the alpha channel in
   * this case. */
  if (!have_opaque /* && !have_semi_trans */)
    return SDL_DisplayFormatAlpha(src);

  /* If we get here, the surface has fully transparent pixels and fully opaque
   * pixels, but no semi-transparent pixels. We can therefore use a one bit
   * transparency, which is pretty fast to draw. This code path is a bit bulky
   * and rarely used (~25 surfaces when starting the game and entering a
   * level), so it could be removed for readabilities sake. -octo */

  /* Create a new surface without alpha channel */
  SDL_Surface *dst = SDL_CreateRGBSurface(src->flags & ~(SDL_SRCALPHA),
      src->w, src->h, src->format->BitsPerPixel,
      src->format->Rmask,  src->format->Gmask, src->format->Bmask, /* Amask = */ 0);
  /* Get a color that's not in the source surface. It is used to mark
   * transparent pixels. There's a possible race condition: Maybe we should
   * lock the surface before calling this function and add a "bool have_lock"
   * argument to "get_unused_color"? -octo */
  Uint32 color_key = get_unused_color (src);

  if(SDL_MUSTLOCK(src))
    SDL_LockSurface(src);
  if(SDL_MUSTLOCK(dst))
    SDL_LockSurface(dst);

  /* Copy all the pixels to the new surface */
  for(int y = 0; y < src->h; y++) {
    for(int x = 0; x < src->w; x++) {
      Uint8 *src_pixel = (Uint8 *) src->pixels
        + (y * src->pitch) + (x * src->format->BytesPerPixel);
      Uint8 *dst_pixel = (Uint8 *) dst->pixels
        + (y * dst->pitch) + (x * dst->format->BytesPerPixel);
      Uint32 mapped = get_pixel_mapping (src, src_pixel);
      Uint8 red, green, blue, alpha;
      SDL_GetRGBA(mapped, src->format, &red, &green, &blue, &alpha);

      /* "alpha" should either be smaller than 16 or greater than 240. We
       * unlocked the surface in between though, so use 128 to play it save,
       * i.e. don't leave any unspecified code paths. */
      if (alpha < 128)
        mapped = color_key;

      switch (dst->format->BytesPerPixel)
      {
        case 1:
          *dst_pixel = (Uint8) mapped;
          break;

        case 2:
          *((Uint16 *) dst_pixel) = (Uint16) mapped;
          break;

        case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            dst_pixel[0] = (mapped >> 16) & 0xff;
            dst_pixel[1] = (mapped >> 8) & 0xff;
            dst_pixel[2] = (mapped >> 0) & 0xff;
#else
            dst_pixel[0] = (mapped >> 0) & 0xff;
            dst_pixel[1] = (mapped >> 8) & 0xff;
            dst_pixel[2] = (mapped >> 16) & 0xff;
#endif
            break;

        case 4:
            *((Uint32 *) dst_pixel) = mapped;
      } /* switch (dst->format->BytesPerPixel) */
    } /* for (x) */
  } /* for (y) */

  if(SDL_MUSTLOCK(src))
    SDL_UnlockSurface(src);
  if(SDL_MUSTLOCK(dst))
    SDL_UnlockSurface(dst);

  /* Tell SDL that the "color_key" color is supposed to be transparent. */
  SDL_SetColorKey (dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, color_key);
  SDL_Surface *convert = SDL_DisplayFormat(dst);
  SDL_FreeSurface(dst);
  return convert;
} /* SDL_Surface *optimize */

} /* namespace */

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

/* vim: set sw=2 sts=2 et : */
/* EOF */
