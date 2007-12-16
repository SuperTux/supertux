//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "Renderer.hpp"
#include <unison/video/Surface.hpp>
#include <unison/video/Window.hpp>

#include <assert.h>

namespace Unison
{
   namespace Video
   {
      namespace Auto
      {
         Renderer::Renderer(const std::vector<Backend::Renderer *> &renderers) :
            renderer(0),
            renderers(renderers)
         {
         }

         Renderer::~Renderer()
         {
         }

         void Renderer::init()
         {
            assert(!renderer);
            std::vector<Backend::Renderer *>::const_reverse_iterator found = std::find_if(renderers.rbegin(), renderers.rend(), std::mem_fun(&Unison::Video::Backend::Renderer::is_usable));
            if(found == renderers.rend())
            {
               fputs("No usable renderers found\n", stderr);
               return;
            }
            renderer = *found;
            renderer->init();
         }

         void Renderer::quit()
         {
            assert(renderer);
            renderer->quit();
            renderer = 0;
         }

         std::string Renderer::get_name()
         {
            return "auto";
         }

         bool Renderer::is_usable()
         {
            return std::find_if(renderers.begin(), renderers.end(), std::mem_fun(&Unison::Video::Backend::Renderer::is_usable)) != renderers.end();
         }

         Surface Renderer::load_surface(const std::string &filename)
         {
            assert(renderer);
            return renderer->load_surface(filename);
         }

         Surface Renderer::load_surface(const std::string &filename, const Color &colorkey)
         {
            assert(renderer);
            return renderer->load_surface(filename, colorkey);
         }

         void Renderer::save_surface(const Surface &surface, const std::string &filename)
         {
            assert(renderer);
            return renderer->save_surface(surface, filename);
         }

         void Renderer::blit(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos, const RenderOptions &options)
         {
            assert(renderer);
            renderer->blit(src, src_rect, dst, dst_pos, options);
         }

         void Renderer::blit(Backend::Texture *src, const Rect &src_rect, Surface &dst, const Point &dst_pos, const RenderOptions &options)
         {
            assert(renderer);
            renderer->blit(src, src_rect, dst, dst_pos, options);
         }

         void Renderer::fill(Surface &dst, const Color &color, const Rect &rect)
         {
            assert(renderer);
            renderer->fill(dst, color, rect);
         }

         void Renderer::fill_blend(Surface &dst, const Color &color, const Rect &rect)
         {
            assert(renderer);
            renderer->fill_blend(dst, color, rect);
         }

         Backend::Window *Renderer::create_window(const Area &size, const Area &logical_size, bool fullscreen)
         {
            assert(renderer);
            return renderer->create_window(size, logical_size, fullscreen);
         }

         Backend::Texture *Renderer::create_texture(const Surface &surface)
         {
            assert(renderer);
            return renderer->create_texture(surface);
         }
      }
   }
}
