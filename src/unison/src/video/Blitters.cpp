//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <unison/video/Blitters.hpp>
#include <unison/video/Surface.hpp>

#include <assert.h>

namespace Unison
{
   namespace Video
   {
      void Blitters::blit_upper(const Surface &src, Rect src_rect, Surface &dst, Point dst_pos, void (*blit_lower)(const Surface &, const Rect &, Surface &, const Point &))
      {
         assert(src.get_pixels());
         assert(dst.get_pixels());
         assert(blit_lower);
         if(src_rect == Rect())
         {
            src_rect.size.x = src.get_size().x;
            src_rect.size.y = src.get_size().y;
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
         if(src_rect.get_right() > (int) src.get_size().x)
         {
            src_rect.size.x = src.get_size().x - src_rect.pos.x;
         }
         if(src_rect.get_bottom() > (int) src.get_size().y)
         {
            src_rect.size.y = src.get_size().y - src_rect.pos.y;
         }
         if(dst_pos.x + src_rect.size.x > dst.get_size().x)
         {
            src_rect.size.x = dst.get_size().x - dst_pos.x;
         }
         if(dst_pos.y + src_rect.size.y > dst.get_size().y)
         {
            src_rect.size.y = dst.get_size().y - dst_pos.y;
         }
         blit_lower(src, src_rect, dst, dst_pos);
      }

      void Blitters::blit_lower_none(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos)
      {
         if(src_rect.pos == Point() && dst_pos == Point() && src.get_size().x == dst.get_size().x && src_rect.size.x == dst.get_size().x)
         {
            memcpy(dst.get_pixels(), src.get_pixels(), src_rect.size.x * src_rect.size.y * sizeof(Color));
         }
         else
         {
            const Color *src_line = src.get_pixels() + src_rect.pos.y * src.get_size().x + src_rect.pos.x;
            Color *dst_line = dst.get_pixels() + dst_pos.y * dst.get_size().x + dst_pos.x;
            for(unsigned int y = 0;y < src_rect.size.y;y++)
            {
               memcpy(dst_line, src_line, src_rect.size.x * sizeof(Color));
               src_line += src.get_size().x;
               dst_line += dst.get_size().x;
            }
         }
      }

      void Blitters::blit_lower_mask(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos)
      {
         const Color *src_pixel = src.get_pixels() + src_rect.pos.y * src.get_size().x + src_rect.pos.x;
         Color *dst_pixel = dst.get_pixels() + dst_pos.y * dst.get_size().x + dst_pos.x;
         for(unsigned int y = 0;y < src_rect.size.y;y++)
         {
            for(unsigned int x = 0;x < src_rect.size.x;x++)
            {
               if(src_pixel->alpha)
               {
                  *dst_pixel = *src_pixel;
               }
               src_pixel++;
               dst_pixel++;
            }
            src_pixel += src.get_size().x - src_rect.size.x;
            dst_pixel += dst.get_size().x - src_rect.size.x;
         }
      }

      void Blitters::blit_lower_alpha(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos)
      {
         const Color *src_pixel = src.get_pixels() + src_rect.pos.y * src.get_size().x + src_rect.pos.x;
         Color *dst_pixel = dst.get_pixels() + dst_pos.y * dst.get_size().x + dst_pos.x;
         for(unsigned int y = 0;y < src_rect.size.y;y++)
         {
            for(unsigned int x = 0;x < src_rect.size.x;x++)
            {
               dst_pixel->red += src_pixel->red * src_pixel->alpha - dst_pixel->red * src_pixel->alpha;
               dst_pixel->green += src_pixel->green * src_pixel->alpha - dst_pixel->green * src_pixel->alpha;
               dst_pixel->blue += src_pixel->blue * src_pixel->alpha - dst_pixel->green * src_pixel->blue;
               src_pixel++;
               dst_pixel++;
            }
            src_pixel += src.get_size().x - src_rect.size.x;
            dst_pixel += dst.get_size().x - src_rect.size.x;
         }
      }

      void Blitters::blit_lower_add(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos)
      {
         const Color *src_pixel = src.get_pixels() + src_rect.pos.y * src.get_size().x + src_rect.pos.x;
         Color *dst_pixel = dst.get_pixels() + dst_pos.y * dst.get_size().x + dst_pos.x;
         for(unsigned int y = 0;y < src_rect.size.y;y++)
         {
            for(unsigned int x = 0;x < src_rect.size.x;x++)
            {
               if(src_pixel->red != 0 && dst_pixel->red != 0xff)
               {
                  int redsum = dst_pixel->red + src_pixel->red * src_pixel->alpha / 0xff;
                  dst_pixel->red = redsum & ~0xff ? 0xff : redsum;
               }
               if(src_pixel->green != 0 && dst_pixel->green != 0xff)
               {
                  int greensum = dst_pixel->green + src_pixel->green * src_pixel->alpha / 0xff;
                  dst_pixel->green = greensum & ~0xff ? 0xff : greensum;
               }
               if(src_pixel->blue != 0 && dst_pixel->blue != 0xff)
               {
                  int bluesum = dst_pixel->blue + src_pixel->blue * src_pixel->alpha / 0xff;
                  dst_pixel->blue = bluesum & ~0xff ? 0xff : bluesum;
               }
               src_pixel++;
               dst_pixel++;
            }
            src_pixel += src.get_size().x - src_rect.size.x;
            dst_pixel += dst.get_size().x - src_rect.size.x;
         }
      }

      void Blitters::blit_lower_mod(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos)
      {
         const Color *src_pixel = src.get_pixels() + src_rect.pos.y * src.get_size().x + src_rect.pos.x;
         Color *dst_pixel = dst.get_pixels() + dst_pos.y * dst.get_size().x + dst_pos.x;
         for(unsigned int y = 0;y < src_rect.size.y;y++)
         {
            for(unsigned int x = 0;x < src_rect.size.x;x++)
            {
               dst_pixel->red = dst_pixel->red * src_pixel->red / 0xff;
               dst_pixel->green = dst_pixel->green * src_pixel->green / 0xff;
               dst_pixel->blue = dst_pixel->blue * src_pixel->blue / 0xff;
               dst_pixel->alpha = dst_pixel->alpha * src_pixel->alpha / 0xff;
               src_pixel++;
               dst_pixel++;
            }
            src_pixel += src.get_size().x - src_rect.size.x;
            dst_pixel += dst.get_size().x - src_rect.size.x;
         }
      }
   }
}
