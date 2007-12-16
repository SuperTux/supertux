//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_AUTO_RENDERER_HPP
#define UNISON_VIDEO_AUTO_RENDERER_HPP

#include <unison/video/backend/Renderer.hpp>

#include <string> 
#include <vector> 

namespace Unison
{
   namespace Video
   {
      namespace Backend
      {
         class Texture;
         class Window;
      }
      namespace Auto
      {
         /// Does rendering tasks like blits and color fills using SDL
         class Renderer : public Backend::Renderer
         {
            public:
               Renderer(const std::vector<Backend::Renderer *> &renderers);
               ~Renderer();

               void init();
               void quit();

               std::string get_name();
               bool is_usable();

               Surface load_surface(const std::string &filename);
               Surface load_surface(const std::string &filename, const Color &colorkey);
               void save_surface(const Surface &surface, const std::string &filename);

               void blit(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos, const RenderOptions &options);
               void blit(Backend::Texture *src, const Rect &src_rect, Surface &dst, const Point &dst_pos, const RenderOptions &options);
               void fill(Surface &dst, const Color &color, const Rect &rect);
               void fill_blend(Surface &dst, const Color &color, const Rect &rect);

               Backend::Window *create_window(const Area &size, const Area &logical_size, bool fullscreen);
               Backend::Texture *create_texture(const Surface &surface);
            private:
               Backend::Renderer *renderer;
               const std::vector<Backend::Renderer *> &renderers;
         };
      }
   }
}

#endif
