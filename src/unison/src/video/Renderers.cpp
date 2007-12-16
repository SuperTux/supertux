//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <unison/video/Renderers.hpp>
#include <unison/video/Window.hpp>
#include <unison/video/Texture.hpp>
#include "auto/Renderer.hpp"
#include "sdl/Renderer.hpp"
#include "opengl/Renderer.hpp"

#include <assert.h>

namespace Unison
{
   namespace Video
   {
      Renderers::Renderers() :
         auto_renderer(0),
         renderer(0),
         renderers()
      {
         auto_renderer = new Auto::Renderer(renderers);
         renderer = auto_renderer;
         add_renderer(new SDL::Renderer());
         add_renderer(new OpenGL::Renderer());
         renderer->init();
      }

      Renderers::~Renderers()
      {
         assert(renderer);
         renderer->quit();
         std::for_each(renderers.begin(), renderers.end(), std::ptr_fun(operator delete));
         delete auto_renderer;
      }

      Renderers &Renderers::get()
      {
         static Renderers renderers;
         return renderers;
      }

      namespace
      {
         bool match_name(Backend::Renderer *renderer, std::string name)
         {
            return renderer && renderer->get_name() == name;
         }
      }

      void Renderers::set_renderer(const std::string &name)
      {
         Area window_size;
         bool fullscreen = false;
         if(Window::get().is_open())
         {
            window_size = Window::get().get_size();
            fullscreen = Window::get().is_fullscreen();
         }
         std::vector<Surface> surfaces = Texture::save_textures();
         renderer->quit();
         if(name == "auto")
         {
            renderer = auto_renderer;
         }
         else
         {
            std::vector<Backend::Renderer *>::iterator found = std::find_if(renderers.begin(), renderers.end(), std::bind2nd(std::ptr_fun(match_name), name));
            if(found == renderers.end())
            {
               fprintf(stderr, "Renderer '%s' not found.\n", name.c_str());
               return;
            }
            renderer = *found;
         }
         renderer->init();
         Texture::load_textures(surfaces);
         if(window_size != Area())
         {
            Window::get().open(window_size, fullscreen);
         }
      }


      Backend::Renderer &Renderers::get_renderer()
      {
         assert(renderer);
         return *renderer;
      }

      void Renderers::add_renderer(Backend::Renderer *renderer)
      {
         renderers.push_back(renderer);
      }
   }
}
