//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <unison/video/backend/Texture.hpp>
#include <unison/video/Renderers.hpp>
#include <unison/video/backend/Renderer.hpp>

#include <assert.h>
#include <algorithm>

namespace Unison
{
   namespace Video
   {
      namespace Backend
      {
         std::vector<Texture *> Texture::textures = std::vector<Texture *>();
         std::map<std::string, TextureID> Texture::named_textures = std::map<std::string, TextureID>();

         Texture::Texture(const Surface &surface) :
            surface(surface),
            size(surface.get_size()),
            refcount(0)
         {
            assert(surface.get_size() != Area());
         }

         Texture::~Texture()
         {
         }

         Area Texture::get_size()
         {
            return size;
         }

         void Texture::ref()
         {
            refcount++;
         }

         void Texture::unref()
         {
            assert(refcount > 0);
            refcount--;
            if(refcount == 0)
            {
               std::string name = get_name(get_texture_id(this));
               *std::find(textures.begin(), textures.end(), this) = 0;
               if(!name.empty())
               {
                  named_textures.erase(name);
               }
               delete this;
            }
         }

         int Texture::get_refcount()
         {
            return refcount;
         }

         namespace
         {
            class TextureSaver
            {
               public:
                  void operator () (Texture *texture)
                  {
                     if(texture)
                     {
                        texture->save();
                        surfaces.push_back(texture->get_surface());
                     }
                     else
                     {
                        surfaces.push_back(Surface());
                     }
                     delete texture;
                  }
                  std::vector<Surface> surfaces;
            };

            class TextureLoader
            {
               public:
                  void operator () (Surface surface)
                  {
                     if(surface.get_size() != Area())
                     {
                        textures.push_back(Renderers::get().get_renderer().create_texture(surface));
                     }
                     else
                     {
                        textures.push_back(0);
                     }
                  }
                  std::vector<Texture *> textures;
            };
         }

         std::vector<Surface> Texture::save_textures()
         {
            std::vector<Surface> surfaces = std::for_each(textures.begin(), textures.end(), TextureSaver()).surfaces;
            textures.clear();
            return surfaces;
         }

         void Texture::load_textures(const std::vector<Surface> &surfaces)
         {
            assert(textures.empty());
            textures = std::for_each(surfaces.begin(), surfaces.end(), TextureLoader()).textures;
         }

         std::map<TextureID, TextureID> Texture::recover_texture_ids()
         {
            std::map<TextureID, TextureID> change_map;
            std::vector<Texture *> new_textures;
            bool null_texture_found = false;
            for(std::vector<Texture *>::iterator iter = textures.begin(), end = textures.end();iter != end;++iter)
            {
               if(*iter)
               {
                  new_textures.push_back(*iter);
                  if(null_texture_found)
                  {
                     change_map[iter - textures.begin()] = new_textures.size() -1;
                  }
               }
               else
               {
                  null_texture_found = true;
               }
            }
            textures = new_textures;
            return change_map;
         }

         TextureID Texture::get_texture_id(const std::string &filename)
         {
            if(named_textures.find(filename) != named_textures.end())
            {
               return named_textures[filename];
            }
            textures.push_back(Renderers::get().get_renderer().create_texture(Surface(filename)));
            named_textures[filename] = textures.size() - 1;
            return textures.size() - 1;
         }

         TextureID Texture::get_texture_id(const std::string &filename, const Color&colorkey)
         {
            if(named_textures.find(filename) != named_textures.end())
            {
               return named_textures[filename];
            }
            textures.push_back(Renderers::get().get_renderer().create_texture(Surface(filename, colorkey)));
            named_textures[filename] = textures.size() - 1;
            return textures.size() - 1;
         }

         TextureID Texture::get_texture_id(const Surface &surface)
         {
            textures.push_back(Renderers::get().get_renderer().create_texture(surface));
            return textures.size() - 1;
         }

         TextureID Texture::get_texture_id(Texture *texture)
         {
            std::vector<Texture *>::iterator found = std::find(textures.begin(), textures.end(), texture);
            if(found != textures.end())
            {
               return found - textures.begin();
            }
            return INVALID_TEXTURE_ID;
         }

         Texture *Texture::get_texture(TextureID id)
         {
            assert(id < textures.size());
            assert(textures[id]);
            return textures[id];
         }

         namespace
         {
            bool match_id(std::pair<std::string, TextureID> pair, TextureID id)
            {
               return pair.second == id;
            }
         }

         std::string Texture::get_name(TextureID id)
         {
            if(id == INVALID_TEXTURE_ID)
            {
               return std::string();
            }
            std::map<std::string, TextureID>::iterator found = std::find_if(named_textures.begin(), named_textures.end(), std::bind2nd(std::ptr_fun(match_id), id));
            if(found == named_textures.end())
            {
               return std::string();
            }
            return found->first;
         }
      }
   }
}
