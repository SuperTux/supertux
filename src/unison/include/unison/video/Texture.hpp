//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_TEXTURE_HPP
#define UNISON_VIDEO_TEXTURE_HPP

#include <unison/video/Blittable.hpp>
#include <unison/video/Surface.hpp>
#include <unison/video/Coord.hpp>

#include <string>
#include <vector>
#include <set>

namespace Unison
{
   namespace Video
   {
      typedef unsigned int TextureID;
      static const TextureID INVALID_TEXTURE_ID = ~0;
      /// An image that is optimized for fast drawing
      class Texture : public Blittable
      {
         public:
            /// Default constructor
            Texture();

            /// Opens image file indicated by filename
            /// \param[in] filename The filename of the image file
            Texture(const std::string &filename);

            /// Opens image file indicated by filename and use the specified colorkey
            /// \param[in] filename The filename of the image file
            /// \param[in] colorkey The colorkey used by the file
            Texture(const std::string &filename, const Color &colorkey);

            /// Create a texture from the given surface
            /// \param[in] surface The surface to optimize
            Texture(const Surface &surface);

            /// Copy constructor
            /// \param[in] rhs The source texture
            Texture(const Texture &rhs);

            /// Destructor
            ~Texture();

            /// Assignment operator
            /// \param[in] rhs The source surface
            Texture &operator =(const Texture &rhs);

            /// Retrieves the texture's id
            /// \return The id of the surface
            TextureID get_id() const;

            /// Retrieves the texture's size
            /// \return The size of the surface
            Area get_size() const;

            /// Does a surface-to-texture blit
            /// \param[in] src The source surface
            /// \param[in] dst_pos The position to blit to
            /// \param[in] src_rect The part of the source surface to blit from
            /// \param[in] options Extra blit options
            void blit(const Surface &src, const Point &dst_pos = Point(), const Rect &src_rect = Rect(), const RenderOptions &options = RenderOptions());

            /// Does a texture-to-texture blit
            /// \param[in] src The source texture
            /// \param[in] dst_pos The position to blit to
            /// \param[in] src_rect The part of the source texture to blit from
            /// \param[in] options Extra blit options
            void blit(const Texture &src, const Point &dst_pos = Point(), const Rect &src_rect = Rect(), const RenderOptions &options = RenderOptions());

            /// Fills a portion of the image with the given color
            /// \param[in] color The color
            /// \param[in] rect The portion to fill
            void fill(const Color &color, const Rect &rect = Rect());

            /// Fills and alpha blend a portion of the image with the given color
            /// \param[in] color The color
            /// \param[in] rect The portion to fill
            void fill_blend(const Color &color, const Rect &rect = Rect());

            static void unload();
            static std::vector<Surface> swap_out();
            static void swap_in(const std::vector<Surface> &surfaces);

            /// Recover previously used but now unused texture IDs
            static void recover_ids();
         private:
            /// Copy on Write
            void cow();

            /// The texture ID
            TextureID id;

            /// All the textures in existence
            static std::set<Texture *> textures;
      };

      /// A section of a texture
      class TextureSection
      {
         public:
            /// The image
            Texture image;

            /// The clip rectangle
            Rect clip_rect;

            /// Create a section from an image and a rectangle
            /// \param[in] image The image
            /// \param[in] rect The clip rectangle
            TextureSection(const Texture &image = Texture(), const Rect &clip_rect = Rect()) :
               image(image),
               clip_rect(clip_rect)
            {
            }
      };
   }
}

#endif
