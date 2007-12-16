//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_BLITTABLE_HPP
#define UNISON_VIDEO_BLITTABLE_HPP

#include <unison/video/Coord.hpp>
#include <unison/video/Rect.hpp>
#include <unison/video/RenderOptions.hpp>

namespace Unison
{
   namespace Video
   {
      class Surface;
      class SurfaceSection;
      class Texture;
      class TextureSection;
      class DisplayList;
      class Blittable
      {
         public:
            virtual ~Blittable();

            /// Does a surface blit
            /// \param[in] src The source surface
            /// \param[in] dst_pos The position to blit to
            /// \param[in] src_rect The part of the source surface to blit from
            /// \param[in] options Extra blit options
            virtual void blit(const Surface &src, const Point &dst_pos = Point(), const Rect &src_rect = Rect(), const RenderOptions &options = RenderOptions()) = 0;

            /// Does a texture blit
            /// \param[in] src The source texture
            /// \param[in] dst_pos The position to blit to
            /// \param[in] src_rect The part of the source texture to blit from
            /// \param[in] options Extra blit options
            virtual void blit(const Texture &src, const Point &dst_pos = Point(), const Rect &src_rect = Rect(), const RenderOptions &options = RenderOptions()) = 0;
            /// Does a surface section blit
            /// \param[in] section The section to blit
            /// \param[in] dst_pos The position to blit to
            /// \param[in] options Extra blit options
            void blit_section(const SurfaceSection &section, const Point &dst_pos = Point(), const RenderOptions &options = RenderOptions());

            /// Does a texture section blit
            /// \param[in] section The section to blit
            /// \param[in] dst_pos The position to blit to
            /// \param[in] options Extra blit options
            void blit_section(const TextureSection &section, const Point &dst_pos = Point(), const RenderOptions &options = RenderOptions());

            /// \param[in] color The color
            /// \param[in] rect The portion to fill
            virtual void fill(const Color &color, const Rect &rect = Rect()) = 0;

            /// \param[in] color The color
            /// \param[in] rect The portion to fill
            virtual void fill_blend(const Color &color, const Rect &rect = Rect()) = 0;

            /// Draw the requests in the display list
            /// \param[in] list The display list
            void draw(const DisplayList &list);
      };

      /// A section of a blittable
      class BlittableSection : public Blittable
      {
         public:
            /// The image
            Blittable &image;

            /// The clip rectangle
            Rect clip_rect;

            /// Create a section from an image and a rectangle
            /// \param[in] image The image
            /// \param[in] rect The clip rectangle
            BlittableSection(Blittable &image, const Rect &clip_rect = Rect());

            /// Does a clipped blit to the image
            /// \param[in] src The source surface
            /// \param[in] dst_pos The position to blit to
            /// \param[in] src_rect The part of the blit source to blit from
            /// \param[in] options Extra blit options
            void blit(const Surface &src, const Point &dst_pos = Point(), const Rect &src_rect = Rect(), const RenderOptions &options = RenderOptions());

            /// Does a clipped blit to the image
            /// \param[in] src The source texture
            /// \param[in] dst_pos The position to blit to
            /// \param[in] src_rect The part of the blit source to blit from
            /// \param[in] options Extra blit options
            void blit(const Texture &src, const Point &dst_pos = Point(), const Rect &src_rect = Rect(), const RenderOptions &options = RenderOptions());

            /// Fills the camera viewable portion with a color
            /// \param[in] color The color
            /// \param[in] rect The portion to fill
            void fill(const Color &color, const Rect &rect = Rect());

            /// Fills the camera viewable portion with a color using alpha blending
            /// \param[in] color The color
            /// \param[in] rect The portion to fill
            void fill_blend(const Color &color, const Rect &rect = Rect());
      };
   }
}

#endif
