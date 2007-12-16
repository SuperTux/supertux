//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <unison/video/sdl/Blitters.hpp>
#include <unison/video/Surface.hpp>

#include <assert.h>

#include "SDL.h"

namespace
{
   unsigned char add_saturate(unsigned char lhs, int rhs)
   {
      if(lhs + rhs < 0x00)
      {
         return 0x00;
      }
      if(lhs + rhs > 0xff)
      {
         return 0xff;
      }
      return lhs + rhs;
   }
}


namespace Unison
{
   namespace Video
   {
      namespace SDL
      {
         /*SDL_Surface *Blitters::optimize(const Surface &src)
         {
            bool colors[(1 << 12)];
            memset(colors, 0, (1 << 12) * sizeof(bool));

            Surface dst(src);
            Color *iter = dst.get_pixels();
            for(unsigned int y = 0;y < dst.get_size().y;y++)
            {
               for(unsigned int x = 0;x < dst.get_size().x;x++, iter++)
               {
                  unsigned char oldalpha = iter->alpha;
                  unsigned char newalpha = oldalpha & 0x80 ? 0xff : 0x00;
                  iter->alpha = newalpha;
                  int error = oldalpha - newalpha;
                  if(x != dst.get_size().x - 1)
                  {
                     (iter + 1)->alpha = add_saturate((iter + 1)->alpha, error * 7 / 16);
                  }
                  if(y != dst.get_size().y - 1)
                  {
                     if(x != 0)
                     {
                        (iter + dst.get_size().x - 1)->alpha = add_saturate((iter + dst.get_size().x - 1)->alpha, error * 3 / 16);
                     }
                     (iter + dst.get_size().x)->alpha = add_saturate((iter + dst.get_size().x)->alpha, error * 5 / 16);
                     if(x != dst.get_size().x - 1)
                     {
                        (iter + dst.get_size().x + 1)->alpha = add_saturate((iter + dst.get_size().x + 1)->alpha, error * 1 / 16);
                     }
                  }
                  if(newalpha != 0)
                  {
                     colors[((iter->red & 0xf0) << 4) | (iter->green & 0xf0) | ((iter->blue & 0xf0) >> 4)] = true;
                  }
               }
            }

            int keycolor = -1;
            for(int i = 0;i < (1 << 12);i++)
            {
               if(!colors[i])
               {
                  keycolor = i;
                  break;
               }
            }
            if(keycolor == -1)
            {
               SDL_Surface *converted = create_sdl_surface_from(src);
               SDL_Surface *optimized = SDL_DisplayFormatAlpha(converted);
               SDL_FreeSurface(converted);
               return optimized;
            }

            Color key(((keycolor & 0xf00) >> 4) | 0xf, (keycolor & 0x0f0) | 0xf, ((keycolor & 0x00f) << 4) | 0xf);

            Color *end = dst.get_pixels() + dst.get_size().x * dst.get_size().y;
            for(iter = dst.get_pixels();iter != end;++iter)
            {
               if(iter->alpha == 0x00)
               {
                  *iter = key;
               }
            }

            SDL_Surface *converted = create_sdl_surface_from(dst);
            SDL_SetColorKey(converted, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(converted->format, key.red, key.green, key.blue));
            SDL_Surface *optimized = SDL_DisplayFormat(converted);
            SDL_FreeSurface(converted);
            return optimized;
         }*/
         SDL_Surface *Blitters::optimize(const Surface &src)
         {
            unsigned int transparent = 0;
            unsigned int opaque = 0;
            unsigned int semitransparent = 0;
            unsigned int alphasum = 0;
            unsigned int squaredalphasum = 0;
            bool colors[(1 << 12)];
            memset(colors, 0, (1 << 12) * sizeof(bool));

            const Color *src_end = src.get_pixels() + src.get_size().x * src.get_size().y;
            for(const Color *iter = src.get_pixels();iter != src_end;++iter)
            {
               if(iter->alpha < 16)
               {
                  transparent++;
               }
               else if(iter->alpha > 240)
               {
                  opaque++;
                  alphasum += iter->alpha;
                  squaredalphasum += iter->alpha * iter->alpha;
               }
               else
               {
                  semitransparent++;
                  squaredalphasum += iter->alpha * iter->alpha;
               }
               if(iter->alpha != 0)
               {
                  colors[((iter->red & 0xf0) << 4) | (iter->green & 0xf0) | ((iter->blue & 0xf0) >> 4)] = true;
               }
            }

            unsigned int avgalpha = (opaque + semitransparent) ? alphasum / (opaque + semitransparent) : 0;
            unsigned int avgsquaredalpha = (opaque + semitransparent) ? squaredalphasum / (opaque + semitransparent) : 0;
            unsigned int alphavariance = avgsquaredalpha - avgalpha * avgalpha;
            if(semitransparent > ((transparent + opaque + semitransparent) / 8) && alphavariance > 16)
            {
               SDL_Surface *converted = create_sdl_surface_from(src);
               SDL_Surface *optimized = SDL_DisplayFormatAlpha(converted);
               SDL_FreeSurface(converted);
               return optimized;
            }

            int keycolor = -1;
            for(int i = 0;i < (1 << 12);i++)
            {
               if(!colors[i])
               {
                  keycolor = i;
                  break;
               }
            }
            if(keycolor == -1)
            {
               SDL_Surface *converted = create_sdl_surface_from(src);
               SDL_Surface *optimized = SDL_DisplayFormatAlpha(converted);
               SDL_FreeSurface(converted);
               return optimized;
            }

            Color key(((keycolor & 0xf00) >> 4) | 0xf, (keycolor & 0x0f0) | 0xf, ((keycolor & 0x00f) << 4) | 0xf);
            Surface dst(src.get_size());
            Color *dst_end = dst.get_pixels() + dst.get_size().x * dst.get_size().y;
            const Color *src_iter = src.get_pixels();
            Color *dst_iter = dst.get_pixels();
            for(;dst_iter != dst_end;++dst_iter, ++src_iter)
            {
               *dst_iter = (src_iter->alpha < avgalpha / 4) ? key : *src_iter;
            }

            SDL_Surface *converted = create_sdl_surface_from(dst);
            if(avgalpha < 240)
            {
               SDL_SetAlpha(converted, SDL_SRCALPHA | SDL_RLEACCEL, avgalpha);
            }
            SDL_SetColorKey(converted, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(converted->format, key.red, key.green, key.blue));
            SDL_Surface *optimized = SDL_DisplayFormat(converted);
            SDL_FreeSurface(converted);
            return optimized;
         }

         void Blitters::blit_upper(SDL_Surface *src, Rect src_rect, SDL_Surface *dst, Point dst_pos, void (*blit_lower)(SDL_Surface *, const Rect &, SDL_Surface *, const Point &))
         {
            assert(src);
            assert(dst);
            assert(blit_lower);
            if(src_rect == Rect())
            {
               src_rect.size.x = src->w;
               src_rect.size.y = src->h;
            }
            if(dst_pos.x < 0)
            {
               if(src_rect.size.x < (unsigned int) -dst_pos.x)
               {
                  return;
               }
               src_rect.pos.x += -dst_pos.x;
               src_rect.size.x += dst_pos.x;
               dst_pos.x = 0;
            }
            if(dst_pos.y < 0)
            {
               if(src_rect.size.y < (unsigned int) -dst_pos.y)
               {
                  return;
               }
               src_rect.pos.y += -dst_pos.y;
               src_rect.size.y += dst_pos.y;
               dst_pos.y = 0;
            }
            if(src_rect.pos.x < 0)
            {
               if(src_rect.size.x < (unsigned int) -src_rect.pos.x)
               {
                  return;
               }
               src_rect.size.x += src_rect.pos.x;
               src_rect.pos.x = 0;
            }
            if(src_rect.pos.y < 0)
            {
               if(src_rect.size.y < (unsigned int) -src_rect.pos.y)
               {
                  return;
               }
               src_rect.size.y += src_rect.pos.y;
               src_rect.pos.y = 0;
            }
            if(src_rect.get_right() > src->w)
            {
               src_rect.size.x = src->w - src_rect.pos.x;
            }
            if(src_rect.get_bottom() > src->h)
            {
               src_rect.size.y = src->h - src_rect.pos.y;
            }
            if(int(dst_pos.x + src_rect.size.x) > dst->w)
            {
               src_rect.size.x = dst->w - dst_pos.x;
            }
            if(int(dst_pos.y + src_rect.size.y) > dst->h)
            {
               src_rect.size.y = dst->h - dst_pos.y;
            }
            blit_lower(src, src_rect, dst, dst_pos);
         }

         void Blitters::blit_lower_none(SDL_Surface *src, const Rect &src_rect, SDL_Surface *dst, const Point &dst_pos)
         {
            SDL_Rect sdl_src_rect = {src_rect.pos.x, src_rect.pos.y, src_rect.size.x, src_rect.size.y};
            SDL_Rect sdl_dst_rect = {dst_pos.x, dst_pos.y, 0, 0};

            Uint8 alpha = src->format->alpha;
            SDL_SetAlpha(src, 0, 0);
            SDL_BlitSurface(src, &sdl_src_rect, dst, &sdl_dst_rect);
            SDL_SetAlpha(src, SDL_SRCALPHA | SDL_RLEACCEL, alpha);
         }

         void Blitters::blit_lower_mask(SDL_Surface *src, const Rect &src_rect, SDL_Surface *dst, const Point &dst_pos)
         {
            if(SDL_MUSTLOCK(src))
            {
               SDL_LockSurface(src);
            }
            if(SDL_MUSTLOCK(dst))
            {
               SDL_LockSurface(dst);
            }
            int src_bpp = src->format->BytesPerPixel;
            int dst_bpp = dst->format->BytesPerPixel;
            Uint8 *src_pixel = (Uint8 *)src->pixels + src_rect.pos.y * src->pitch + src_rect.pos.x * src_bpp;
            Uint8 *dst_pixel = (Uint8 *)dst->pixels + dst_pos.y * dst->pitch + dst_pos.x * dst_bpp;
            for(unsigned int y = 0;y < src_rect.size.y;y++)
            {
               for(unsigned int x = 0;x < src_rect.size.x;x++)
               {
                  Uint32 src_mapped = 0;
                  switch(src_bpp) {
                     case 1:
                        src_mapped = *src_pixel;
                        break;
                     case 2:
                        src_mapped = *(Uint16 *)src_pixel;
                        break;
                     case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                        src_mapped |= src_pixel[0] << 16;
                        src_mapped |= src_pixel[1] << 8;
                        src_mapped |= src_pixel[2] << 0;
#else
                        src_mapped |= src_pixel[0] << 0;
                        src_mapped |= src_pixel[1] << 8;
                        src_mapped |= src_pixel[2] << 16;
#endif
                        break;
                     case 4:
                        src_mapped = *(Uint32 *)src_pixel;
                        break;
                  }
                  Uint8 src_red, src_green, src_blue, src_alpha;
                  SDL_GetRGBA(src_mapped, src->format, &src_red, &src_green, &src_blue, &src_alpha);
                  if(src_alpha)
                  {
                     Uint32 blend_mapped = SDL_MapRGBA(dst->format, src_red, src_green, src_blue, src_alpha);
                     switch(dst_bpp) {
                        case 1:
                           *dst_pixel = blend_mapped;
                           break;
                        case 2:
                           *(Uint16 *)dst_pixel = blend_mapped;
                           break;
                        case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                           dst_pixel[0] = (blend_mapped >> 16) & 0xff;
                           dst_pixel[1] = (blend_mapped >> 8) & 0xff;
                           dst_pixel[2] = (blend_mapped >> 0) & 0xff;
#else
                           dst_pixel[0] = (blend_mapped >> 0) & 0xff;
                           dst_pixel[1] = (blend_mapped >> 8) & 0xff;
                           dst_pixel[2] = (blend_mapped >> 16) & 0xff;
#endif
                           break;
                        case 4:
                           *(Uint32 *)dst_pixel = blend_mapped;
                           break;
                     }
                  }
                  src_pixel += src_bpp;
                  dst_pixel += dst_bpp;
               }
               src_pixel += src->pitch - src_rect.size.x * src_bpp;
               dst_pixel += dst->pitch - src_rect.size.x * dst_bpp;
            }
            if(SDL_MUSTLOCK(dst))
            {
               SDL_UnlockSurface(dst);
            }
            if(SDL_MUSTLOCK(src))
            {
               SDL_UnlockSurface(src);
            }
         }

         void Blitters::blit_lower_alpha(SDL_Surface *src, const Rect &src_rect, SDL_Surface *dst, const Point &dst_pos)
         {
            SDL_Rect sdl_src_rect = {src_rect.pos.x, src_rect.pos.y, src_rect.size.x, src_rect.size.y};
            SDL_Rect sdl_dst_rect = {dst_pos.x, dst_pos.y, 0, 0};

            SDL_BlitSurface(src, &sdl_src_rect, dst, &sdl_dst_rect);
         }

         void Blitters::blit_lower_add(SDL_Surface *src, const Rect &src_rect, SDL_Surface *dst, const Point &dst_pos)
         {
            if(SDL_MUSTLOCK(src))
            {
               SDL_LockSurface(src);
            }
            if(SDL_MUSTLOCK(dst))
            {
               SDL_LockSurface(dst);
            }
            int src_bpp = src->format->BytesPerPixel;
            int dst_bpp = dst->format->BytesPerPixel;
            Uint8 *src_pixel = (Uint8 *)src->pixels + src_rect.pos.y * src->pitch + src_rect.pos.x * src_bpp;
            Uint8 *dst_pixel = (Uint8 *)dst->pixels + dst_pos.y * dst->pitch + dst_pos.x * dst_bpp;
            for(unsigned int y = 0;y < src_rect.size.y;y++)
            {
               for(unsigned int x = 0;x < src_rect.size.x;x++)
               {
                  Uint32 src_mapped = 0;
                  switch(src_bpp) {
                     case 1:
                        src_mapped = *src_pixel;
                        break;
                     case 2:
                        src_mapped = *(Uint16 *)src_pixel;
                        break;
                     case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                        src_mapped |= src_pixel[0] << 16;
                        src_mapped |= src_pixel[1] << 8;
                        src_mapped |= src_pixel[2] << 0;
#else
                        src_mapped |= src_pixel[0] << 0;
                        src_mapped |= src_pixel[1] << 8;
                        src_mapped |= src_pixel[2] << 16;
#endif
                        break;
                     case 4:
                        src_mapped = *(Uint32 *)src_pixel;
                        break;
                  }
                  Uint8 src_red, src_green, src_blue, src_alpha;
                  SDL_GetRGBA(src_mapped, src->format, &src_red, &src_green, &src_blue, &src_alpha);
                  Uint32 dst_mapped = 0;
                  switch(dst_bpp) {
                     case 1:
                        dst_mapped = *dst_pixel;
                        break;
                     case 2:
                        dst_mapped = *(Uint16 *)dst_pixel;
                        break;
                     case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                        dst_mapped |= dst_pixel[0] << 16;
                        dst_mapped |= dst_pixel[1] << 8;
                        dst_mapped |= dst_pixel[2] << 0;
#else
                        dst_mapped |= dst_pixel[0] << 0;
                        dst_mapped |= dst_pixel[1] << 8;
                        dst_mapped |= dst_pixel[2] << 16;
#endif
                        break;
                     case 4:
                        dst_mapped = *(Uint32 *)dst_pixel;
                        break;
                  }
                  Uint8 dst_red, dst_green, dst_blue, dst_alpha;
                  SDL_GetRGBA(dst_mapped, dst->format, &dst_red, &dst_green, &dst_blue, &dst_alpha);
                  Uint8 blend_red = src_red, blend_green = src_green, blend_blue = src_blue, blend_alpha = src_alpha;
                  if(src_red != 0 && dst_red != 0xff)
                  {
                     int redsum = dst_red + src_red * src_alpha / 0xff;
                     blend_red = redsum & ~0xff ? 0xff : redsum;
                  }
                  else
                  {
                  }
                  if(src_green != 0 && dst_green != 0xff)
                  {
                     int greensum = dst_green + src_green * src_alpha / 0xff;
                     blend_green = greensum & ~0xff ? 0xff : greensum;
                  }
                  if(src_blue != 0 && dst_blue != 0xff)
                  {
                     int bluesum = dst_blue + src_blue * src_alpha / 0xff;
                     blend_blue = bluesum & ~0xff ? 0xff : bluesum;
                  }
                  if(src_red != blend_red || src_green != blend_green || src_blue != blend_blue || src_alpha != blend_alpha)
                  {
                     Uint32 blend_mapped = SDL_MapRGBA(dst->format, blend_red, blend_green, blend_blue, blend_alpha);
                     switch(dst_bpp) {
                        case 1:
                           *dst_pixel = blend_mapped;
                           break;
                        case 2:
                           *(Uint16 *)dst_pixel = blend_mapped;
                           break;
                        case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                           dst_pixel[0] = (blend_mapped >> 16) & 0xff;
                           dst_pixel[1] = (blend_mapped >> 8) & 0xff;
                           dst_pixel[2] = (blend_mapped >> 0) & 0xff;
#else
                           dst_pixel[0] = (blend_mapped >> 0) & 0xff;
                           dst_pixel[1] = (blend_mapped >> 8) & 0xff;
                           dst_pixel[2] = (blend_mapped >> 16) & 0xff;
#endif
                           break;
                        case 4:
                           *(Uint32 *)dst_pixel = blend_mapped;
                           break;
                     }
                  }
                  src_pixel += src_bpp;
                  dst_pixel += dst_bpp;
               }
               src_pixel += src->pitch - src_rect.size.x * src_bpp;
               dst_pixel += dst->pitch - src_rect.size.x * dst_bpp;
            }
            if(SDL_MUSTLOCK(dst))
            {
               SDL_UnlockSurface(dst);
            }
            if(SDL_MUSTLOCK(src))
            {
               SDL_UnlockSurface(src);
            }
         }

         void Blitters::blit_lower_mod(SDL_Surface *src, const Rect &src_rect, SDL_Surface *dst, const Point &dst_pos)
         {
            if(SDL_MUSTLOCK(src))
            {
               SDL_LockSurface(src);
            }
            if(SDL_MUSTLOCK(dst))
            {
               SDL_LockSurface(dst);
            }
            int src_bpp = src->format->BytesPerPixel;
            int dst_bpp = dst->format->BytesPerPixel;
            Uint8 *src_pixel = (Uint8 *)src->pixels + src_rect.pos.y * src->pitch + src_rect.pos.x * src_bpp;
            Uint8 *dst_pixel = (Uint8 *)dst->pixels + dst_pos.y * dst->pitch + dst_pos.x * dst_bpp;
            for(unsigned int y = 0;y < src_rect.size.y;y++)
            {
               for(unsigned int x = 0;x < src_rect.size.x;x++)
               {
                  Uint32 src_mapped = 0;
                  switch(src_bpp) {
                     case 1:
                        src_mapped = *src_pixel;
                        break;
                     case 2:
                        src_mapped = *(Uint16 *)src_pixel;
                        break;
                     case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                        src_mapped |= src_pixel[0] << 16;
                        src_mapped |= src_pixel[1] << 8;
                        src_mapped |= src_pixel[2] << 0;
#else
                        src_mapped |= src_pixel[0] << 0;
                        src_mapped |= src_pixel[1] << 8;
                        src_mapped |= src_pixel[2] << 16;
#endif
                        break;
                     case 4:
                        src_mapped = *(Uint32 *)src_pixel;
                        break;
                  }
                  Uint8 src_red, src_green, src_blue, src_alpha;
                  SDL_GetRGBA(src_mapped, src->format, &src_red, &src_green, &src_blue, &src_alpha);
                  Uint32 dst_mapped = 0;
                  switch(dst_bpp) {
                     case 1:
                        dst_mapped = *dst_pixel;
                        break;
                     case 2:
                        dst_mapped = *(Uint16 *)dst_pixel;
                        break;
                     case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                        dst_mapped |= dst_pixel[0] << 16;
                        dst_mapped |= dst_pixel[1] << 8;
                        dst_mapped |= dst_pixel[2] << 0;
#else
                        dst_mapped |= dst_pixel[0] << 0;
                        dst_mapped |= dst_pixel[1] << 8;
                        dst_mapped |= dst_pixel[2] << 16;
#endif
                        break;
                     case 4:
                        dst_mapped = *(Uint32 *)dst_pixel;
                        break;
                  }
                  Uint8 dst_red, dst_green, dst_blue, dst_alpha;
                  SDL_GetRGBA(dst_mapped, dst->format, &dst_red, &dst_green, &dst_blue, &dst_alpha);
                  Uint8 blend_red, blend_green, blend_blue, blend_alpha;
                  blend_red = dst_red * src_red / 0xff;
                  blend_green = dst_green * src_green / 0xff;
                  blend_blue = dst_blue * src_blue / 0xff;
                  blend_alpha = dst_alpha * src_alpha / 0xff;
                  if(src_red != blend_red || src_green != blend_green || src_blue != blend_blue || src_alpha != blend_alpha)
                  {
                     Uint32 blend_mapped = SDL_MapRGBA(dst->format, blend_red, blend_green, blend_blue, blend_alpha);
                     switch(dst_bpp) {
                        case 1:
                           *dst_pixel = blend_mapped;
                           break;
                        case 2:
                           *(Uint16 *)dst_pixel = blend_mapped;
                           break;
                        case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                           dst_pixel[0] = (blend_mapped >> 16) & 0xff;
                           dst_pixel[1] = (blend_mapped >> 8) & 0xff;
                           dst_pixel[2] = (blend_mapped >> 0) & 0xff;
#else
                           dst_pixel[0] = (blend_mapped >> 0) & 0xff;
                           dst_pixel[1] = (blend_mapped >> 8) & 0xff;
                           dst_pixel[2] = (blend_mapped >> 16) & 0xff;
#endif
                           break;
                        case 4:
                           *(Uint32 *)dst_pixel = blend_mapped;
                           break;
                     }
                  }
                  src_pixel += src_bpp;
                  dst_pixel += dst_bpp;
               }
               src_pixel += src->pitch - src_rect.size.x * src_bpp;
               dst_pixel += dst->pitch - src_rect.size.x * dst_bpp;
            }
            if(SDL_MUSTLOCK(dst))
            {
               SDL_UnlockSurface(dst);
            }
            if(SDL_MUSTLOCK(src))
            {
               SDL_UnlockSurface(src);
            }
         }
      }
   }
}
