//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "Texture.hpp"
#include <unison/video/Surface.hpp>
#include <unison/video/Window.hpp>
#include <unison/video/Renderers.hpp>
#include <unison/video/sdl/Blitters.hpp>
#include <unison/video/backend/Renderer.hpp>

#include <assert.h>

namespace Unison
{
   namespace Video
   {
      namespace SDL
      {
         Texture::Texture(const Surface &surface) :
            Backend::Texture(surface),
            scaled(),
            n_cache(),
            h_cache(),
            v_cache(),
            d_cache()
         {
         }
         /*Texture::Texture(const Surface &surface, const std::string &name) :
            Backend::Texture(surface, name),
            scaled(),
            n_cache(),
            h_cache(),
            v_cache(),
            d_cache()
         {
         }

         Texture::Texture(Backend::Texture *texture) :
            Backend::Texture(texture),
            scaled(),
            n_cache(),
            h_cache(),
            v_cache(),
            d_cache()
         {
         }*/

         Texture::~Texture()
         {
         }

         const Surface Texture::get_surface()
         {
            return surface;
         }

         void Texture::save()
         {
            scaled = Surface();
            n_cache.clear();
            h_cache.clear();
            v_cache.clear();
            d_cache.clear();
         }

         void Texture::blit(const Surface &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options)
         {
            save();
            Renderers::get().get_renderer().blit(src, src_rect, surface, dst_pos, options);
         }

         void Texture::blit(const Video::Texture &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options)
         {
            save();
            Texture *texture = dynamic_cast<Texture *>(Backend::Texture::get_texture(src.get_id()));
            Renderers::get().get_renderer().blit(texture, src_rect, surface, dst_pos, options);
         }

         void Texture::fill(const Color &color, const Rect &rect)
         {
            save();
            Renderers::get().get_renderer().fill(surface, color, rect);
         }

         void Texture::fill_blend(const Color &color, const Rect &rect)
         {
            save();
            Renderers::get().get_renderer().fill_blend(surface, color, rect);
         }

         SDL_Surface *Texture::get_transform(const RenderOptions &options, unsigned int numerator, unsigned int denominator)
         {
            if(scaled.get_size() == Area())
            {
               scaled = surface.scale(numerator, denominator);
            }
            assert(scaled.get_size() == surface.get_size() * numerator / denominator);
            std::map<Color, AlphaMap> &cache = options.h_flip ? (options.v_flip ? d_cache : h_cache) : (options.v_flip ? v_cache : n_cache);
            if(!cache[options.color][options.alpha])
            {
               Surface transformed = scaled.modulate(options.color).modulate(options.alpha);
               if(options.h_flip)
               {
                  transformed = transformed.h_flip();
               }
               if(options.v_flip)
               {
                  transformed = transformed.v_flip();
               }
               cache[options.color][options.alpha] = Blitters::optimize(transformed);
            }
            return cache[options.color][options.alpha];
         }
      }
   }
}
