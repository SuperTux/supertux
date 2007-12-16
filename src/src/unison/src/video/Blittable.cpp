//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <unison/video/Blittable.hpp>
#include <unison/video/DisplayList.hpp>

namespace Unison
{
   namespace Video
   {
      Blittable::~Blittable()
      {
      }

      void Blittable::blit_section(const SurfaceSection &section, const Point &dst_pos, const RenderOptions &options)
      {
         blit(section.image, dst_pos, section.clip_rect, options);
      }

      void Blittable::blit_section(const TextureSection &section, const Point &dst_pos, const RenderOptions &options)
      {
         blit(section.image, dst_pos, section.clip_rect, options);
      }

      void Blittable::draw(const DisplayList &list)
      {
         list.draw(this);
      }

      BlittableSection::BlittableSection(Blittable &image, const Rect &clip_rect) :
         image(image),
         clip_rect(clip_rect)
      {
      }

      void BlittableSection::blit(const Surface &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options)
      {
         Rect overlap = clip_rect.get_overlap(Rect(dst_pos, src_rect.size));
         if(overlap == Rect())
         {
            return;
         }
         Rect clipped_src_rect(src_rect.pos, overlap.size);
         clipped_src_rect.pos += overlap.pos;
         clipped_src_rect.pos -= dst_pos;
         image.blit(src, overlap.pos - clip_rect.pos, clipped_src_rect, options);
      }

      void BlittableSection::blit(const Texture &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options)
      {
         Rect overlap = clip_rect.get_overlap(Rect(dst_pos, src_rect.size));
         if(overlap == Rect())
         {
            return;
         }
         Rect clipped_src_rect(src_rect.pos, overlap.size);
         clipped_src_rect.pos += overlap.pos;
         clipped_src_rect.pos -= dst_pos;
         image.blit(src, overlap.pos - clip_rect.pos, clipped_src_rect, options);
      }

      void BlittableSection::fill(const Color &color, const Rect &rect)
      {
         Rect overlap = clip_rect.get_overlap(rect);
         overlap.pos -= clip_rect.pos;
         image.fill(color, overlap);
      }

      void BlittableSection::fill_blend(const Color &color, const Rect &rect)
      {
         Rect overlap = clip_rect.get_overlap(rect);
         overlap.pos -= clip_rect.pos;
         image.fill_blend(color, overlap);
      }
   }
}
