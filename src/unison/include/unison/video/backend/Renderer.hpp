//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_BACKEND_RENDERER_HPP
#define UNISON_VIDEO_BACKEND_RENDERER_HPP

#include <unison/video/Coord.hpp>

#include <string>
#include <istream>

namespace Unison
{
   namespace Video
   {
      class Surface;
      class Texture;
      class Window;
      class Color;
      class Rect;
      class RenderOptions;

      namespace Backend
      {
         class Texture;
         class Window;
         /// Backend-specific renderer interface
         class Renderer
         {
            public:
               /// Destructor
               virtual ~Renderer()
               {
               }

               /// Initialize the backend
               virtual void init() = 0;

               /// Cleanup the backend
               virtual void quit() = 0;

               /// Get the name of the renderer
               /// \return the name of the renderer
               virtual std::string get_name() = 0;

               /// Check if the backend is usable
               /// \return Whether the backend is usable
               virtual bool is_usable() = 0;

               virtual Surface load_surface(const std::string &filename) = 0;
               virtual Surface load_surface(const std::string &filename, const Color &colorkey) = 0;

               virtual void save_surface(const Surface &surface, const std::string &filename) = 0;

               /// Does a surface-to-surface blit
               /// \param[in] src The source surface
               /// \param[in] src_rect The part of the source surface to blit from
               /// \param[in] dst The destination surface
               /// \param[in] dst_pos The position to blit to
               /// \param[in] options Extra blit options
               virtual void blit(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos, const RenderOptions &options) = 0;

               /// Does a texture-to-surface blit
               /// \param[in] src The source texture
               /// \param[in] src_rect The part of the source texture to blit from
               /// \param[in] dst The destination surface
               /// \param[in] dst_pos The position to blit to
               /// \param[in] options Extra blit options
               virtual void blit(Texture *src, const Rect &src_rect, Surface &dst, const Point &dst_pos, const RenderOptions &options) = 0;

               /// Fills a portion of a surface with the given color
               /// \param[in] dst The destination surface
               /// \param[in] color The color
               /// \param[in] rect The portion to fill
               virtual void fill(Surface &dst, const Color &color, const Rect &rect) = 0;

               /// Fills with alpha blend a portion of a surface with the given color
               /// \param[in] dst The destination surface
               /// \param[in] color The color
               /// \param[in] rect The portion to fill
               virtual void fill_blend(Surface &dst, const Color &color, const Rect &rect) = 0;

               /// Create a window
               /// \param[in] size The size of the window
               /// \param[in] logical_size The logical size of the window
               /// \param[in] fullscreen Whether to open in fullscreen mode
               /// \return The created window
               virtual Window *create_window(const Area &size, const Area &logical_size, bool fullscreen) = 0;

               /// Create a texture for the given surface
               /// \param[in] surface The surface to convert
               /// \param[in] name The name of the texture
               /// \return The texture for the surface
               virtual Texture *create_texture(const Surface &surface) = 0;
         };
      }
   }
}

#endif
