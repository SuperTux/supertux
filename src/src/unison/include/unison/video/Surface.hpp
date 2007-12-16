//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_SURFACE_HPP
#define UNISON_VIDEO_SURFACE_HPP

#include <unison/video/Blittable.hpp>
#include <unison/video/RenderOptions.hpp>

#include <string>
#include <istream>
#include <assert.h>

namespace Unison
{
   namespace Video
   {
      class Color;
      class Rect;
      class Texture;
      /// An image that is optimized for easy manipulation
      class Surface : public Blittable
      {
         public:
            /// Default constructor
            Surface();

            /// Create a Surface from an input stream
            /// \param[in] src The input stream
            Surface(std::istream &stream);

            /// Create a Surface from an input stream
            /// \param[in] src The input stream
            /// \param[in] colorkey The colorkey used by the file
            Surface(std::istream &stream, const Color &colorkey);

            /// Opens image file indicated by filename
            /// \param[in] filename The filename of the image file
            Surface(const std::string &filename);

            /// Opens image file indicated by filename and use the specified color key
            /// \param[in] filename The filename of the image file
            /// \param[in] colorkey The colorkey used by the file
            Surface(const std::string &filename, const Color &colorkey);

            /// Creates Surface of indicated dimensions
            /// \param[in] size The size of the desired surface
            Surface(const Area &size);

            /// Copy constructor
            /// \param[in] rhs The source surface
            Surface(const Surface &rhs);

            /// Destructor
            ~Surface();

            /// Assignment operator
            /// \param[in] rhs The source surface
            Surface &operator =(const Surface &rhs);

            /// Saves the surface to file
            /// \param[in] filename The destination filename
            void save(const std::string &filename) const;

            /// Retrieves the window's size
            /// \return The size of the surface
            Area get_size() const
            {
               return pixels ? pixels->size : Area();
            }

            /// Retrieves a pixel at the specified coordinates
            /// \param[in] pos The position of the pixel to retrieve
            /// \return The pixel at the specified position
            Color &get_pixel(const Point &pos)
            {
               cow();
               assert(pixels);
               return pixels->buffer[pos.y * pixels->size.x + pos.x];
            }

            /// Retrieves the pixel color at the specified coordinates
            /// \param[in] x The x position of the pixel to retrieve
            /// \param[in] y The y position of the pixel to retrieve
            /// \return The color of the pixel at the specified position
            Color& get_pixel(int x, int y)
            {
               cow();
               assert(pixels);
               return pixels->buffer[y * pixels->size.x + x];
            }

            /// Retrieves the pixel color at the specified coordinates
            /// \param[in] pos The position of the pixel to retrieve
            /// \return The color of the pixel at the specified position
            Color get_pixel(const Point &pos) const
            {
               assert(pixels);
               return pixels->buffer[pos.y * pixels->size.x + pos.x];
            }

            /// Retrieves the pixel color at the specified coordinates
            /// \param[in] x The x position of the pixel to retrieve
            /// \param[in] y The y position of the pixel to retrieve
            /// \return The color of the pixel at the specified position
            Color get_pixel(int x, int y) const
            {
               assert(pixels);
               return pixels->buffer[y * pixels->size.x + x];
            }

            /// Acquire the pixel color buffer
            /// \return the pixel color buffer
            Color *get_pixels()
            {
               cow();
               assert(pixels);
               return pixels->buffer;
            }

            /// Acquire the pixel color buffer
            /// \return the pixel color buffer
            const Color *get_pixels() const
            {
               assert(pixels);
               return pixels->buffer;
            }

            /// Does a surface-to-surface blit
            /// \param[in] src The source surface
            /// \param[in] dst_pos The position to blit to
            /// \param[in] src_rect The part of the source surface to blit from
            /// \param[in] options Extra blit options
            void blit(const Surface &src, const Point &dst_pos = Point(), const Rect &src_rect = Rect(), const RenderOptions &options = RenderOptions());

            /// Does a texture-to-surface blit
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

            /// Scale the surface by a factor of (numerator / denominator)
            /// \param[in] numerator The numerator of the scale factor
            /// \param[in] denominator The denominator of the scale factor
            /// \return The scaled surface
            Surface scale(unsigned int numerator, unsigned int denominator) const;

            /// Flip the surface horizontally
            /// \return The flipped surface
            Surface h_flip() const;

            /// Flip the surface vertically
            /// \return The flipped surface
            Surface v_flip() const;

            /// Modulate the image with a color
            /// \return The modulated surface
            Surface modulate(const Color &color) const;

            /// Modulate the image with an alpha
            /// \return The modulated surface
            Surface modulate(unsigned char alpha) const;
         private:
            /// Copy on Write
            void cow();

            class PixelBuffer
            {
               public:
                  PixelBuffer(Area size)
                   : buffer(0),
                     size(size),
                     refcount(1)
                  {
                     if(size != Area())
                     {
                        buffer = new Color[size.x * size.y];
                     }
                  }

                  ~PixelBuffer()
                  {
                     delete buffer;
                  }

                  void ref()
                  {
                     refcount++;
                  }

                  void unref()
                  {
                     assert(refcount > 0);
                     refcount--;
                     if(refcount == 0)
                     {
                        delete this;
                     }
                  }

                  Color *buffer;
                  Area size;
                  int refcount;
            };

            /// The pixels
            PixelBuffer *pixels;
      };

      /// A section of a surface
      class SurfaceSection
      {
         public:
            /// The image
            Surface image;

            /// The clip rectangle
            Rect clip_rect;

            /// Create a section from an image and a rectangle
            /// \param[in] image The image
            /// \param[in] rect The clip rectangle
            SurfaceSection(const Surface &image = Surface(), const Rect &clip_rect = Rect()) :
               image(image),
               clip_rect(clip_rect)
            {
            }
      };
   }
}

#endif
