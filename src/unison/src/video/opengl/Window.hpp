//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_OPENGL_WINDOW_HPP
#define UNISON_VIDEO_OPENGL_WINDOW_HPP

#include <unison/video/backend/Window.hpp>
#include <unison/video/RenderOptions.hpp>

#include <string>

#include "SDL.h"

namespace Unison
{
   namespace Video
   {
      namespace Backend
      {
         class Texture;
      }
      namespace OpenGL
      {
         class Window : public Backend::Window
         {
            public:
               Window(const Area &size, const Area &logical_size, bool fullscreen = false);

               ~Window();
               void take_screenshot(const std::string &filename) const;
               void flip();
               void set_title(const std::string &title);
               void set_icon(const Surface &icon);
               Area get_size() const;
               bool is_fullscreen() const;
               void blit(const Surface &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options);
               void blit(const Video::Texture &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options);
               void fill(const Color &color, const Rect &rect);
               void fill_blend(const Color &color, const Rect &rect);
            private:
               /// The logical size of the window;
               Area logical_size;

               /// The window
               SDL_Surface *window;
         };
      }
   }
}

#endif
