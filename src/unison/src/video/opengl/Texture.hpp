//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_OPENGL_TEXTURE_HPP
#define UNISON_VIDEO_OPENGL_TEXTURE_HPP

#include <unison/video/backend/Texture.hpp>
#include <unison/video/Surface.hpp>

#include <string>
#include <vector>

#include "SDL.h"
#include "SDL_gl.h"

namespace Unison
{
   namespace Video
   {
      namespace OpenGL
      {
         class Texture : public Backend::Texture
         {
            public:
               Texture(const Surface &surface);
               //Texture(const Surface &surface, const std::string &name);
               //Texture(Backend::Texture *texture);
               Texture();
               ~Texture();

               const Surface get_surface();
               void save();
               void blit(const Surface &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options);
               void blit(const Video::Texture &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options);
               void fill(const Color &color, const Rect &rect);
               void fill_blend(const Color &color, const Rect &rect);

               void blit_draw_buffer(const Rect &src_rect, const Point &dst_pos, const RenderOptions &options);
            private:
               struct Handle
               {
                  GLuint texture;
                  Rect rect;
               };
               std::vector<Handle> handles;

               static Handle create_handle(const Surface &surface, const Rect &rect);
         };
      }
   }
}

#endif
