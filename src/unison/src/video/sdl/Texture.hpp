//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_SDL_TEXTURE_HPP
#define UNISON_VIDEO_SDL_TEXTURE_HPP

#include <unison/video/backend/Texture.hpp>
#include <unison/video/Surface.hpp>

#include <string>
#include <map>

#include "SDL.h"

namespace Unison
{
   namespace Video
   {
      namespace SDL
      {
         class Texture : public Backend::Texture
         {
            public:
               Texture(const Surface &surface);
               //Texture(const Surface &surface, const std::string &name);
               //Texture(Backend::Texture *texture);
               ~Texture();

               const Surface get_surface();
               void save();
               void blit(const Surface &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options);
               void blit(const Video::Texture &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options);
               void fill(const Color &color, const Rect &rect);
               void fill_blend(const Color &color, const Rect &rect);

               SDL_Surface *get_transform(const RenderOptions &options, unsigned int numerator, unsigned int denominator);
            private:
               Surface scaled;
               struct AlphaMap
               {
                  static void ref(SDL_Surface *surface)
                  {
                    if(surface)
                    {
                      surface->refcount++;
                    }
                  }

                  SDL_Surface *data[256];

                  AlphaMap()
                  {
                     memset(data, 0, 256 * sizeof(SDL_Surface *));
                  }

                  ~AlphaMap()
                  {
                     std::for_each(data, data + 256, SDL_FreeSurface);
                  }

                  void operator = (const AlphaMap &other)
                  {
                     std::for_each(other.data, other.data + 256, ref);
                     std::for_each(data, data + 256, SDL_FreeSurface);
                     memcpy(data, other.data, 256 * sizeof(SDL_Surface *));
                  }

                  SDL_Surface *&operator [] (Uint8 alpha)
                  {
                     return data[alpha];
                  }
               };
               //std::map<Color, AlphaMap> cache;
               std::map<Color, AlphaMap> n_cache;
               std::map<Color, AlphaMap> h_cache;
               std::map<Color, AlphaMap> v_cache;
               std::map<Color, AlphaMap> d_cache;
         };
      }
   }
}

#endif
