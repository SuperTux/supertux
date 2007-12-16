//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_SDL_BLITTERS_HPP
#define UNISON_VIDEO_SDL_BLITTERS_HPP

#include <unison/video/Blitters.hpp>
#include <unison/video/Coord.hpp>
#include <unison/video/Surface.hpp>

#include "SDL.h"

namespace Unison
{
   namespace Video
   {
      namespace SDL
      {
         struct Blitters
         {
            static SDL_Surface *create_sdl_surface_from(Surface &src)
            {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
               SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(src.get_pixels(), src.get_size().x, src.get_size().y, 32, src.get_size().x * 4, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#else
               SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(src.get_pixels().get_pixels(), src.get_size().x, src.get_size().y, 32, src.get_size().x * 4, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#endif
               return surface;
            }

            static SDL_Surface *create_sdl_surface_from(const Surface &src)
            {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
               SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(const_cast<Color *>(src.get_pixels()), src.get_size().x, src.get_size().y, 32, src.get_size().x * 4, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#else
               SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(const_cast<Color *>(src.get_pixels()).get_pixels(), src.get_size().x, src.get_size().y, 32, src.get_size().x * 4, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#endif
               return surface;
            }

            static SDL_Surface *optimize(const Surface &src);

            static void blit_upper(SDL_Surface *src, Rect src_rect, SDL_Surface *dst, Point dst_pos, void (*blit_lower)(SDL_Surface *, const Rect &, SDL_Surface *, const Point &));

            static void blit_lower_none(SDL_Surface *src, const Rect &src_rect, SDL_Surface *dst, const Point &dst_pos);

            static void blit_lower_mask(SDL_Surface *src, const Rect &src_rect, SDL_Surface *dst, const Point &dst_pos);

            static void blit_lower_alpha(SDL_Surface *src, const Rect &src_rect, SDL_Surface *dst, const Point &dst_pos);

            static void blit_lower_add(SDL_Surface *src, const Rect &src_rect, SDL_Surface *dst, const Point &dst_pos);

            static void blit_lower_mod(SDL_Surface *src, const Rect &src_rect, SDL_Surface *dst, const Point &dst_pos);

            static void blit_blend_none(SDL_Surface *src, const Rect &src_rect, SDL_Surface *dst, const Point &dst_pos)
            {
               blit_upper(src, src_rect, dst, dst_pos, blit_lower_none);
            }

            static void blit_blend_mask(SDL_Surface *src, const Rect &src_rect, SDL_Surface *dst, const Point &dst_pos)
            {
               blit_upper(src, src_rect, dst, dst_pos, blit_lower_mask);
            }

            static void blit_blend_alpha(SDL_Surface *src, const Rect &src_rect, SDL_Surface *dst, const Point &dst_pos)
            {
               blit_upper(src, src_rect, dst, dst_pos, blit_lower_alpha);
            }

            static void blit_blend_add(SDL_Surface *src, const Rect &src_rect, SDL_Surface *dst, const Point &dst_pos)
            {
               blit_upper(src, src_rect, dst, dst_pos, blit_lower_add);
            }

            static void blit_blend_mod(SDL_Surface *src, const Rect &src_rect, SDL_Surface *dst, const Point &dst_pos)
            {
               blit_upper(src, src_rect, dst, dst_pos, blit_lower_mod);
            }

            static void blit_blend(SDL_Surface *src, const Rect &src_rect, SDL_Surface *dst, const Point &dst_pos, BlendMode blend)
            {
               switch(blend)
               {
                  case BLEND_NONE:
                     blit_blend_none(src, src_rect, dst, dst_pos);
                     break;
                  case BLEND_MASK:
                     blit_blend_mask(src, src_rect, dst, dst_pos);
                     break;
                  case BLEND_ALPHA:
                     blit_blend_alpha(src, src_rect, dst, dst_pos);
                     break;
                  case BLEND_ADD:
                     blit_blend_add(src, src_rect, dst, dst_pos);
                     break;
                  case BLEND_MOD:
                     blit_blend_mod(src, src_rect, dst, dst_pos);
                     break;
               }
            }
         };
      }
   }
}

#endif
