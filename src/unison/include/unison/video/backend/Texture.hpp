//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_BACKEND_TEXTURE_HPP
#define UNISON_VIDEO_BACKEND_TEXTURE_HPP

#include <unison/video/Blittable.hpp>
#include <unison/video/Surface.hpp>
#include <unison/video/Texture.hpp>

#include <string>
#include <vector>
#include <map>

namespace Unison
{
   namespace Video
   {
      namespace Backend
      {
         class Renderer;
         /// Backend-specific texture interface
         class Texture : public Blittable
         {
            public:
               /// Destructor
               virtual ~Texture();

               /// Get the size of the texture
               /// \return The texture size 
               Area get_size();

               /// Called when referenced
               void ref();

               /// Called when a reference goes away
               void unref();

               /// Get the number of references to the texture
               /// \return The reference count
               int get_refcount();

               /// Get the equavalent surface to the texture
               virtual const Surface get_surface() = 0;

               /// Save the texture, called when the window is about to be created or recreated
               virtual void save() = 0;

               /// Unload the textures
               static void unload();

               /// Swap out the textures
               static std::vector<Surface> swap_out();

               /// Swap in the textures
               static void swap_in(const std::vector<Surface> &surfaces);

               /// Recover previously used but now unused texture IDs
               /// \return A map of what IDs changed during recovery
               static std::map<TextureID, TextureID> recover_ids();

               /// Retrieve the texture ID for the filename
               /// \param[in] filename The filename of the image file
               /// \return The texture ID of the texture
               static TextureID get_texture_id(const std::string &filename);

               /// Retrieve the texture ID for the filename
               /// \param[in] filename The filename of the image file
               /// \param[in] colorkey The colorkey used by the file
               /// \return The texture ID of the texture
               static TextureID get_texture_id(const std::string &filename, const Color &colorkey);

               /// Retrieve the texture ID for the surface
               /// \param[in] surface The surface to optimize
               /// \return The texture ID of the texture
               static TextureID get_texture_id(const Surface &surface);

               /// Retrieve the texture ID for the texture
               /// \param[in] texture The texture
               /// \return The texture ID of the texture
               static TextureID get_texture_id(Texture *texture);

               /// Retrieve the texture corresponding to the texture ID
               /// \param[in] id The texture ID
               /// \return The corresponding texture
               static Texture *get_texture(TextureID id);

               /// Retrieve the name associated with the texture ID
               /// \param[in] texture The texture
               /// \return The texture ID of the texture
               static std::string get_name(TextureID id);
            protected:
               /// Create a texture from the given surface with the given name
               /// \param[in] surface The surface to optimize
               Texture(const Surface &surface);

               /// The surface the texture is based from
               Surface surface;

               /// The size of the texture
               Area size;

               /// The number of references to the texture
               int refcount;

               /// All of the textures in existence
               static std::vector<Texture *> textures;

               /// The subset of all textures that have names
               static std::map<std::string, TextureID> named_textures;
         };
      }
   }
}

#endif
