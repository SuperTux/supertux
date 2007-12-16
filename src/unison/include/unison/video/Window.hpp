//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_WINDOW_HPP
#define UNISON_VIDEO_WINDOW_HPP

#include <unison/video/Blittable.hpp>
#include <unison/video/DisplayList.hpp>
#include <unison/video/RenderOptions.hpp>
#include <unison/video/Surface.hpp>
#include <unison/video/Texture.hpp>

#include <string>
#include <vector>
#include <map>

namespace Unison
{
   namespace Video
   {
      class Color;
      class Rect;
      class Texture;
      class Surface;
      namespace Backend
      {
         class Window;
      }
      /// Window management singleton
      class Window : public Blittable
      {
         public:
            /// Initialize and retrieve singleton
            static Window &get();

            /// Set the logical size of the window
            /// \param[in] logical_size The logical size of the window
            void set_logical_size(const Area &logical_size);

            /// Get the logical size of the window
            /// \return The logical size of the window
            Area get_logical_size() const;

            /// Open the window
            /// \param[in] size The size of the window
            /// \param[in] fullscreen Whether to open in fullscreen mode
            void open(const Area &size, bool fullscreen = false);

            /// Take a screenshot of the window
            /// \param[in] filename The destination filename
            void take_screenshot(const std::string &filename) const;

            /// Flip request buffers
            /// \note Should be called only once per frame!
            void flip();

            /// Redraw requests
            void redraw();

            /// Set window title
            void set_title(const std::string &title);

            /// Set window icon
            void set_icon(const Surface &icon);

            /// Retrieves the window's size
            /// \return The size of the window
            Area get_size() const;

            /// Queries whether the window is open
            /// \return Whether the window is open
            bool is_open() const;

            /// Queries whether the window is in fullscreen mode
            /// \return Whether the window is fullscreen
            bool is_fullscreen() const;

            /// Does a surface-to-window blit
            /// \param[in] src The source surface
            /// \param[in] dst_pos The position to blit to
            /// \param[in] src_rect The part of the source surface to blit from
            /// \param[in] options Extra blit options
            void blit(const Surface &src, const Point &dst_pos = Point(), const Rect &src_rect = Rect(), const RenderOptions &options = RenderOptions())
            {
               layers[0].blit(src, dst_pos, src_rect, options);
            }

            /// Does a texture-to-window blit
            /// \param[in] src The source texture
            /// \param[in] dst_pos The position to blit to
            /// \param[in] src_rect The part of the source texture to blit from
            /// \param[in] options Extra blit options
            void blit(const Texture &src, const Point &dst_pos = Point(), const Rect &src_rect = Rect(), const RenderOptions &options = RenderOptions())
            {
               layers[0].blit(src, dst_pos, src_rect, options);
            }

            /// Fills a portion of the window with the given color
            /// \param[in] color The color
            /// \param[in] rect The portion to fill
            void fill(const Color &color, const Rect &rect = Rect())
            {
               layers[0].fill(color, rect);
            }

            /// Fills and alpha blend a portion of the window with the given color
            /// \param[in] color The color
            /// \param[in] rect The portion to fill
            void fill_blend(const Color &color, const Rect &rect = Rect())
            {
               layers[0].fill_blend(color, rect);
            }

            DisplayList &operator [] (int layer)
            {
               return layers[layer];
            }
         private:
            /// The logical size of the window
            Area logical_size;

            /// The title of the window
            std::string title;

            /// The window icon
            Surface icon;

            /// The window
            Backend::Window *window;

            /// Display list currently being drawn
            DisplayList list;

            /// Layers of pending display lists
            std::map<int, DisplayList> layers;

            /// Default constructor
            Window();

            /// Destructor
            ~Window();
      };
   }
}

#endif
