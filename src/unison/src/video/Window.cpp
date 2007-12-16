//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <unison/video/Window.hpp>
#include <unison/video/Surface.hpp>
#include <unison/video/Texture.hpp>
#include <unison/video/Renderers.hpp>
#include <unison/video/backend/Renderer.hpp>
#include <unison/video/backend/Window.hpp>

#include <assert.h>

namespace Unison
{
   namespace Video
   {
      Window::Window() :
         logical_size(),
         title(),
         icon(Surface(Area(1, 1))),
         window(0),
         list(),
         layers()
      {
      }

      Window::~Window()
      {
      }

      Window &Window::get()
      {
         // FIXME: fix init order more naturally
         Renderers::get();
         static Window window;
         return window;
      }

      void Window::set_logical_size(const Area &logical_size)
      {
         this->logical_size = logical_size;
         if(window)
         {
            std::vector<Surface> surfaces = Texture::save_textures();
            open(get_size(), is_fullscreen());
            Texture::load_textures(surfaces);
         }
      }

      Area Window::get_logical_size() const
      {
         return logical_size;
      }

      void Window::open(const Area &size, bool fullscreen)
      {
         std::vector<Surface> surfaces = Texture::save_textures();
         if(logical_size.x == 0 || logical_size.y == 0)
         {
            logical_size = size;
         }
         delete window;
         window = Renderers::get().get_renderer().create_window(size, logical_size, fullscreen);
         assert(window);
         Texture::load_textures(surfaces);
         window->set_title(title);
         window->set_icon(icon);
         redraw();
      }

      void Window::take_screenshot(const std::string &filename) const
      {
         assert(window);
         window->take_screenshot(filename);
      }

      void Window::flip()
      {
         list = layers;
         layers.clear();
         redraw();
      }

      void Window::redraw()
      {
         assert(window);
         fill(Color::BLACK);
         window->draw(list);
         window->flip();
      }

      void Window::set_title(const std::string &title)
      {
         this->title = title;
         if(window)
         {
            window->set_title(title);
         }
      }

      void Window::set_icon(const Surface &icon)
      {
         this->icon = icon.get_size() == Area() ? Surface(Area(1, 1)) : icon;
         if(window)
         {
            window->set_icon(icon);
         }
      }

      Area Window::get_size() const
      {
         assert(window);
         return window->get_size();
      }

      bool Window::is_fullscreen() const
      {
         assert(window);
         return window->is_fullscreen();
      }

      bool Window::is_open() const
      {
         return window;
      }
   }
}
