//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <unison/video/Texture.hpp>
#include <unison/video/backend/Texture.hpp>

#include <assert.h>

namespace Unison
{
   namespace Video
   {
      std::set<Texture *> Texture::textures = std::set<Texture*>();

      Texture::Texture() :
         id(INVALID_TEXTURE_ID)
      {
      }

      Texture::Texture(const std::string &filename) :
         id(Backend::Texture::get_texture_id(filename))
      {
         assert(id != INVALID_TEXTURE_ID);
         Backend::Texture::get_texture(id)->ref();
         textures.insert(this);
      }

      Texture::Texture(const std::string &filename, const Color &colorkey) :
         id(Backend::Texture::get_texture_id(filename, colorkey))
      {
         assert(id != INVALID_TEXTURE_ID);
         Backend::Texture::get_texture(id)->ref();
         textures.insert(this);
      }

      Texture::Texture(const Surface &surface) :
         id(Backend::Texture::get_texture_id(surface))
      {
         assert(id != INVALID_TEXTURE_ID);
         Backend::Texture::get_texture(id)->ref();
         textures.insert(this);
      }

      Texture::Texture(const Texture &rhs) :
         Blittable(),
         id(rhs.id)
      {
         if(id != INVALID_TEXTURE_ID)
         {
            Backend::Texture::get_texture(id)->ref();
         }
         textures.insert(this);
      }

      Texture::~Texture()
      {
         textures.erase(this);
         if(id != INVALID_TEXTURE_ID)
         {
            Backend::Texture::get_texture(id)->unref();
         }
      }

      Texture &Texture::operator =(const Texture &rhs)
      {
         if(rhs.id != INVALID_TEXTURE_ID)
         {
            Backend::Texture::get_texture(rhs.id)->ref();
         }
         if(id != INVALID_TEXTURE_ID)
         {
            Backend::Texture::get_texture(id)->unref();
         }
         id = rhs.id;
         return *this;
      }

      TextureID Texture::get_id() const
      {
         return id;
      }

      Area Texture::get_size() const
      {
         assert(id != INVALID_TEXTURE_ID);
         return Backend::Texture::get_texture(id)->get_size();
      }

      void Texture::blit(const Surface &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options)
      {
         assert(id != INVALID_TEXTURE_ID);
         cow();
         Backend::Texture::get_texture(id)->blit(src, dst_pos, src_rect, options);
      }

      void Texture::blit(const Texture &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options)
      {
         assert(id != INVALID_TEXTURE_ID);
         cow();
         Backend::Texture::get_texture(id)->blit(src, dst_pos, src_rect, options);
      }

      void Texture::fill(const Color &color, const Rect &rect)
      {
         assert(id != INVALID_TEXTURE_ID);
         cow();
         Backend::Texture::get_texture(id)->fill(color, rect);
      }

      void Texture::fill_blend(const Color &color, const Rect &rect)
      {
         assert(id != INVALID_TEXTURE_ID);
         cow();
         Backend::Texture::get_texture(id)->fill_blend(color, rect);
      }

      std::vector<Surface> Texture::save_textures()
      {
         recover_texture_ids();
         return Backend::Texture::save_textures();
      }

      namespace
      {
         void ref(Texture *texture)
         {
            assert(texture);
            TextureID id = texture->get_id();
            if(id != INVALID_TEXTURE_ID)
            {
               Backend::Texture::get_texture(id)->ref();
            }
         }
      }

      void Texture::load_textures(const std::vector<Surface> &surfaces)
      {
         Backend::Texture::load_textures(surfaces);
         std::for_each(textures.begin(), textures.end(), ref);
      }

      void Texture::recover_texture_ids()
      {
         std::map<TextureID, TextureID> change_map = Backend::Texture::recover_texture_ids();
         if(!change_map.empty())
         {
            for(std::set<Texture *>::iterator iter = textures.begin(), end = textures.end();iter != end;++iter)
            {
               if(change_map.find((*iter)->id) != change_map.end())
               {
                  (*iter)->id = change_map[(*iter)->id];
               }
            }
         }
      }

      void Texture::cow()
      {
         assert(id != INVALID_TEXTURE_ID);
         if(Backend::Texture::get_texture(id)->get_refcount() > 1)
         {
            TextureID old = id;
            id = Backend::Texture::get_texture_id(Backend::Texture::get_texture(id)->get_surface());
            assert(id != INVALID_TEXTURE_ID);
            Backend::Texture::get_texture(id)->ref();
            Backend::Texture::get_texture(old)->unref();
         }
      }
   }
}
