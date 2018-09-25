//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "video/texture_manager.hpp"

#include <SDL_image.h>
#include <assert.h>
#include <sstream>

#include "math/rect.hpp"
#include "physfs/physfs_sdl.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "video/sdl_surface_ptr.hpp"
#include "video/texture.hpp"
#include "video/video_system.hpp"

TextureManager::TextureManager() :
  m_image_textures(),
  m_surfaces()
{
}

TextureManager::~TextureManager()
{
  for(const auto& texture : m_image_textures)
  {
    if(!texture.second.expired())
    {
      log_warning << "Texture '" << std::get<0>(texture.first) << "' not freed" << std::endl;
    }
  }
  m_image_textures.clear();
  m_surfaces.clear();
}

TexturePtr
TextureManager::get(const std::string& _filename)
{
  std::string filename = FileSystem::normalize(_filename);
  Texture::Key key(filename, 0, 0, 0, 0);
  auto i = m_image_textures.find(key);

  TexturePtr texture;
  if(i != m_image_textures.end())
    texture = i->second.lock();

  if(!texture) {
    texture = create_image_texture(filename);
    texture->m_cache_key = key;
    m_image_textures[key] = texture;
  }

  return texture;
}

TexturePtr
TextureManager::get(const std::string& _filename, const Rect& rect)
{
  std::string filename = FileSystem::normalize(_filename);
  Texture::Key key(filename, rect.left, rect.top, rect.right, rect.bottom);
  auto i = m_image_textures.find(key);

  TexturePtr texture;
  if(i != m_image_textures.end())
    texture = i->second.lock();

  if(!texture) {
    texture = create_image_texture(filename, rect);
    texture->m_cache_key = key;
    m_image_textures[key] = texture;
  }

  return texture;
}

void
TextureManager::reap_cache_entry(const Texture::Key& key)
{
  auto i = m_image_textures.find(key);
  assert(i != m_image_textures.end());
  assert(i->second.expired());
  m_image_textures.erase(i);
}

TexturePtr
TextureManager::create_image_texture(const std::string& filename, const Rect& rect)
{
  try
  {
    return create_image_texture_raw(filename, rect);
  }
  catch(const std::exception& err)
  {
    log_warning << "Couldn't load texture '" << filename << "' (now using dummy texture): " << err.what() << std::endl;
    return create_dummy_texture();
  }
}

const SDL_Surface&
TextureManager::get_surface(const std::string& filename)
{
  auto i = m_surfaces.find(filename);
  if (i != m_surfaces.end())
  {
    return *i->second;
  }
  else
  {
    SDLSurfacePtr image(IMG_Load_RW(get_physfs_SDLRWops(filename), 1));
    if (!image)
    {
      std::ostringstream msg;
      msg << "Couldn't load image '" << filename << "' :" << SDL_GetError();
      throw std::runtime_error(msg.str());
    }

    return *(m_surfaces[filename] = std::move(image));
  }
}

TexturePtr
TextureManager::create_image_texture_raw(const std::string& filename, const Rect& rect)
{
  const SDL_Surface& src_surface = get_surface(filename);

  SDLSurfacePtr convert;
  if (src_surface.format->Rmask == 0 &&
      src_surface.format->Gmask == 0 &&
      src_surface.format->Bmask == 0 &&
      src_surface.format->Amask == 0)
  {
    log_debug << "Wrong surface format for image " << filename << ". Compensating." << std::endl;
    convert.reset(SDL_ConvertSurfaceFormat(const_cast<SDL_Surface*>(&src_surface), SDL_PIXELFORMAT_RGBA8888, 0));
  }

  const SDL_Surface& surface = convert ? *convert : src_surface;

  SDLSurfacePtr subimage(SDL_CreateRGBSurfaceFrom(static_cast<uint8_t*>(surface.pixels) +
                                                  rect.top * surface.pitch +
                                                  rect.left * surface.format->BytesPerPixel,
                                                  rect.get_width(), rect.get_height(),
                                                  surface.format->BitsPerPixel,
                                                  surface.pitch,
                                                  surface.format->Rmask,
                                                  surface.format->Gmask,
                                                  surface.format->Bmask,
                                                  surface.format->Amask));
  if (!subimage)
  {
    throw std::runtime_error("SDL_CreateRGBSurfaceFrom() call failed");
  }

  return VideoSystem::current()->new_texture(*subimage);
}

TexturePtr
TextureManager::create_image_texture(const std::string& filename)
{
  try
  {
    return create_image_texture_raw(filename);
  }
  catch (const std::exception& err)
  {
    log_warning << "Couldn't load texture '" << filename << "' (now using dummy texture): " << err.what() << std::endl;
    return create_dummy_texture();
  }
}

TexturePtr
TextureManager::create_image_texture_raw(const std::string& filename)
{
  SDLSurfacePtr image(IMG_Load_RW(get_physfs_SDLRWops(filename), 1));
  if (!image)
  {
    std::ostringstream msg;
    msg << "Couldn't load image '" << filename << "' :" << SDL_GetError();
    throw std::runtime_error(msg.str());
  }
  else
  {
    TexturePtr texture = VideoSystem::current()->new_texture(*image);
    image.reset(NULL);
    return texture;
  }
}

TexturePtr
TextureManager::create_dummy_texture()
{
  const std::string dummy_texture_fname = "images/engine/missing.png";

  // on error, try loading placeholder file
  try
  {
    TexturePtr tex = create_image_texture_raw(dummy_texture_fname);
    return tex;
  }
  catch (const std::exception& err)
  {
    // on error (when loading placeholder), try using empty surface,
    // when that fails to, just give up
    SDLSurfacePtr image(SDL_CreateRGBSurface(0, 1024, 1024, 8, 0, 0, 0, 0));
    if (!image)
    {
      throw;
    }
    else
    {
      log_warning << "Couldn't load texture '" << dummy_texture_fname << "' (now using empty one): " << err.what() << std::endl;
      TexturePtr texture = VideoSystem::current()->new_texture(*image);
      return texture;
    }
  }
}

/* EOF */
