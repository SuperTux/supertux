//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_BACKEND_WINDOW_HPP
#define UNISON_VIDEO_BACKEND_WINDOW_HPP

#include <unison/video/Blittable.hpp>
#include <unison/video/RenderOptions.hpp>
#include <unison/video/Coord.hpp>
#include <unison/video/Rect.hpp>

#include <string>
#include <vector>

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
         /// Backend-specific window interface
         class Window : public Blittable
         {
            public:
               /// Destructor
               virtual ~Window()
               {
               }

               /// Take a screenshot of the window
               /// \param[in] filename The destination filename
               virtual void take_screenshot(const std::string &filename) const = 0;

               /// Flip request buffers
               /// \note Should be called only once per frame!
               virtual void flip() = 0;

               /// Set window title
               virtual void set_title(const std::string &title) = 0;

               /// Set window icon
               virtual void set_icon(const Surface &icon) = 0;

               /// Retrieves the window's size
               /// \return The size of the window
               virtual Area get_size() const = 0;

               /// Queries whether the window is in fullscreen mode
               /// \return Whether the window is fullscreen
               virtual bool is_fullscreen() const = 0;
         };
      }
   }
}

#endif
