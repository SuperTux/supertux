//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_SDL_RENDERER_HPP
#define UNISON_VIDEO_SDL_RENDERER_HPP

#include <unison/video/backend/Renderer.hpp>

#include <string> 
#include "SDL.h"

namespace Unison
{
   namespace Video
   {
      namespace Backend
      {
         class Texture;
         class Window;
      }
      namespace SDL
      {
         /// Does rendering tasks like blits and color fills using SDL
         class Renderer : public Backend::Renderer
         {
            public:
               /// Default constructor
               Renderer();

               /// Destructor
               ~Renderer();

               /// Initialize the backend
               void init();

               /// Cleanup the backend
               void quit();

               /// Get the name of the renderer
               /// \return the name of the renderer
               std::string get_name();

               /// Check if the backend is usable
               /// \return Whether the backend is usable
               bool is_usable();

               Surface load_surface(const std::string &filename);
               Surface load_surface(const std::string &filename, const Color &colorkey);
               void save_surface(const Surface &surface, const std::string &filename);

               /// Does a surface-to-surface blit
               /// \param[in] src The source surface
               /// \param[in] src_rect The part of the source surface to blit from
               /// \param[in] dst The destination surface
               /// \param[in] dst_pos The position to blit to
               /// \param[in] options Extra blit options
               void blit(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos, const RenderOptions &options);

               /// Does a texture-to-surface blit
               /// \param[in] src The source texture
               /// \param[in] src_rect The part of the source texture to blit from
               /// \param[in] dst The destination surface
               /// \param[in] dst_pos The position to blit to
               /// \param[in] options Extra blit options
               void blit(Backend::Texture *src, const Rect &src_rect, Surface &dst, const Point &dst_pos, const RenderOptions &options);

               /// Fills a portion of a surface with the given color
               /// \param[in] dst The destination surface
               /// \param[in] color The color
               /// \param[in] rect The portion to fill
               void fill(Surface &dst, const Color &color, const Rect &rect);

               /// Fills with alpha blend a portion of a surface with the given color
               /// \param[in] dst The destination surface
               /// \param[in] color The color
               /// \param[in] rect The portion to fill
               void fill_blend(Surface &dst, const Color &color, const Rect &rect);

               /// Create a window
               /// \param[in] size The size of the window
               /// \param[in] logical_size The logical size of the window
               /// \param[in] fullscreen Whether to open in fullscreen mode
               /// \return The created window
               Backend::Window *create_window(const Area &size, const Area &logical_size, bool fullscreen);

               /// Create a texture data for the given surface
               /// \param[in] surface The surface to convert
               /// \return The texture data for the surface
               Backend::Texture *create_texture(const Surface &surface);
         };
      }
   }
}

#endif
