//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_BLITTERS_HPP
#define UNISON_VIDEO_BLITTERS_HPP

#include <unison/video/Coord.hpp>
#include <unison/video/RenderOptions.hpp>

namespace Unison
{
   namespace Video
   {
      class Rect;
      class Surface;

      struct Blitters
      {
         static void blit_upper(const Surface &src, Rect src_rect, Surface &dst, Point dst_pos, void (*blit_lower)(const Surface &, const Rect &, Surface &, const Point &));

         static void blit_lower_none(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos);

         static void blit_lower_mask(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos);

         static void blit_lower_alpha(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos);

         static void blit_lower_add(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos);

         static void blit_lower_mod(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos);

         static void blit_blend_none(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos)
         {
            blit_upper(src, src_rect, dst, dst_pos, blit_lower_none);
         }

         static void blit_blend_mask(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos)
         {
            blit_upper(src, src_rect, dst, dst_pos, blit_lower_mask);
         }

         static void blit_blend_alpha(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos)
         {
            blit_upper(src, src_rect, dst, dst_pos, blit_lower_alpha);
         }

         static void blit_blend_add(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos)
         {
            blit_upper(src, src_rect, dst, dst_pos, blit_lower_add);
         }

         static void blit_blend_mod(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos)
         {
            blit_upper(src, src_rect, dst, dst_pos, blit_lower_mod);
         }

         static void blit_blend(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos, BlendMode blend)
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

#endif
