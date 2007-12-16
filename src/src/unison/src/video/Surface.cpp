//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <unison/video/Texture.hpp>
#include <unison/video/Window.hpp>
#include <unison/video/Renderers.hpp>
#include <unison/video/Color.hpp>
#include <unison/video/sdl/Blitters.hpp>
#include <unison/video/backend/Renderer.hpp>
#include <unison/video/backend/Texture.hpp>
#include <unison/vfs/stream.hpp>

#include <assert.h>

namespace Unison
{
   namespace Video
   {
      Surface::Surface() :
         pixels(0)
      {
      }

      Surface::Surface(const std::string &filename) :
         pixels(0)
      {
         *this = Renderers::get().get_renderer().load_surface(filename);
      }

      Surface::Surface(const std::string &filename, const Color &colorkey) :
         pixels(0)
      {
         *this = Renderers::get().get_renderer().load_surface(filename, colorkey);
      }

      Surface::Surface(const Area &size) :
         pixels(new PixelBuffer(size))
      {
         //fill(Color::WHITE);
      }

      Surface::Surface(const Surface &rhs) :
         Blittable(),
         pixels(rhs.pixels)
      {
         if(pixels)
         {
            pixels->ref();
         }
      }

      Surface::~Surface()
      {
         if(pixels)
         {
            pixels->unref();
         }
      }

      Surface &Surface::operator =(const Surface &rhs)
      {
         if(rhs.pixels)
         {
            rhs.pixels->ref();
         }
         if(pixels)
         {
            pixels->unref();
         }
         pixels = rhs.pixels;
         return *this;
      }

      void Surface::save(const std::string &filename) const
      {
         Renderers::get().get_renderer().save_surface(*this, filename);
      }

      void Surface::blit(const Surface &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options)
      {
         cow();
         assert(pixels);
         Renderers::get().get_renderer().blit(src, src_rect, *this, dst_pos, options);
      }

      void Surface::blit(const Texture &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options)
      {
         cow();
         assert(pixels);
         Renderers::get().get_renderer().blit(Backend::Texture::get_texture(src.get_id()), src_rect, *this, dst_pos, options);
      }

      void Surface::fill(const Color &color, const Rect &rect)
      {
         cow();
         assert(pixels);
         Renderers::get().get_renderer().fill(*this, color, rect);
      }

      void Surface::fill_blend(const Color &color, const Rect &rect)
      {
         cow();
         assert(pixels);
         Renderers::get().get_renderer().fill_blend(*this, color, rect);
      }

      namespace
      {
         Color merge(const Color &color0, const Color &color1, int rem, int total)
         {
            return Color((color0.red * (total - rem) + color1.red * rem) / total,
                         (color0.green * (total - rem) + color1.green * rem) / total,
                         (color0.blue * (total - rem) + color1.blue * rem) / total,
                         (color0.alpha * (total - rem) + color1.alpha * rem) / total);
         }
      }

      Surface Surface::scale(unsigned int numerator, unsigned int denominator) const
      {
         assert(pixels);
         if(numerator == denominator)
         {
            return *this;
         }
         else
         {
            Surface scaled(get_size() * numerator / denominator);
            for(unsigned int y = 0;y < scaled.get_size().y;y++)
            {
               for(unsigned int x = 0;x < scaled.get_size().x;x++)
               {
                  unsigned int srcx = x * denominator / numerator;
                  unsigned int srcy = y * denominator / numerator;
                  //scaled.set_pixel(Point(x, y), get_pixel(Point(srcx, srcy)));
                  int incx = (srcx + 1 == get_size().x ? 0 : 1);
                  int incy = (srcy + 1 == get_size().y ? 0 : 1);
                  Color color00 = get_pixel(srcx, srcy);
                  Color color01 = get_pixel(srcx + incx, srcy);
                  Color color10 = get_pixel(srcx, srcy + incy);
                  Color color11 = get_pixel(srcx + incx, srcy + incy);
                  int remx = x * denominator % numerator;
                  Color color0 = merge(color00, color01, remx, numerator);
                  Color color1 = merge(color10, color11, remx, numerator);
                  int remy = y * denominator % numerator;
                  Color color = merge(color0, color1, remy, numerator);
                  scaled.get_pixel(x, y) = color;
               }
            }
            return scaled;
         }
      }

      Surface Surface::h_flip() const
      {
         assert(pixels);
         Surface flipped(get_size());
         for(unsigned int y = 0;y < get_size().y;y++)
         {
            for(unsigned int x = 0;x < get_size().x;x++)
            {
               flipped.get_pixel(x, y) = get_pixel(get_size().x - x - 1, y);
            }
         }
         return flipped;
      }

      Surface Surface::v_flip() const
      {
         assert(pixels);
         Surface flipped(get_size());
         for(unsigned int y = 0;y < get_size().y;y++)
         {
            for(unsigned int x = 0;x < get_size().x;x++)
            {
               flipped.get_pixel(x, y) = get_pixel(x, get_size().y - y - 1);
            }
         }
         return flipped;
      }

      Surface Surface::modulate(const Color &color) const
      {
         assert(pixels);
         if(color == Color::WHITE)
         {
            return *this;
         }
         else
         {
            Surface modulated(get_size());
            for(unsigned int y = 0;y < get_size().y;y++)
            {
               for(unsigned int x = 0;x < get_size().x;x++)
               {
                  Color pixel = get_pixel(x, y);
                  pixel.red = pixel.red * color.red / 0xff;
                  pixel.green = pixel.green * color.green / 0xff;
                  pixel.blue = pixel.blue * color.blue / 0xff;
                  pixel.alpha = pixel.alpha * color.alpha / 0xff;
                  modulated.get_pixel(x, y) = pixel;
               }
            }
            return modulated;
         }
      }

      Surface Surface::modulate(unsigned char alpha) const
      {
         assert(pixels);
         if(alpha == 0xff)
         {
            return *this;
         }
         else
         {
            Surface modulated(get_size());
            for(unsigned int y = 0;y < get_size().y;y++)
            {
               for(unsigned int x = 0;x < get_size().x;x++)
               {
                  Color pixel = get_pixel(x, y);
                  pixel.alpha = pixel.alpha * alpha / 0xff;
                  modulated.get_pixel(x, y) = pixel;
               }
            }
            return modulated;
         }
      }

      void Surface::cow()
      {
         if(pixels && pixels->refcount > 1)
         {
            PixelBuffer *original = pixels;
            pixels = new PixelBuffer(pixels->size);
            memcpy(pixels->buffer, original->buffer, pixels->size.x * pixels->size.y * sizeof(Color));
         }
      }
   }
}
