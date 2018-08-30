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
  m_font_textures(),
  m_surfaces()
{
}

TextureManager::~TextureManager()
{
  for(const auto& texture : m_image_textures)
  {
    if(!texture.second.expired())
    {
      log_warning << "Texture '" << texture.first << "' not freed" << std::endl;
    }
  }
  m_image_textures.clear();

  for(auto& surface : m_surfaces)
  {
    SDL_FreeSurface(surface.second);
  }
  m_surfaces.clear();
}

TexturePtr
TextureManager::get(const std::string& _filename)
{
  std::string filename = FileSystem::normalize(_filename);
  auto i = m_image_textures.find(filename);

  TexturePtr texture;
  if(i != m_image_textures.end())
    texture = i->second.lock();

  if(!texture) {
    texture = create_image_texture(filename);
    texture->cache_filename = filename;
    m_image_textures[filename] = texture;
  }

  return texture;
}

TexturePtr
TextureManager::get(const std::string& _filename, const Rect& rect)
{
  std::string filename = FileSystem::normalize(_filename);
  std::string key = filename + "_" +
                    std::to_string(rect.left)  + "|" +
                    std::to_string(rect.top)   + "|" +
                    std::to_string(rect.right) + "|" +
                    std::to_string(rect.bottom);
  auto i = m_image_textures.find(key);

  TexturePtr texture;
  if(i != m_image_textures.end())
    texture = i->second.lock();

  if(!texture) {
    texture = create_image_texture(filename, rect);
    texture->cache_filename = key;
    m_image_textures[key] = texture;
  }

  return texture;
}

TexturePtr
TextureManager::get(TTF_Font* font, const std::string& text, const Color& color)
{
  std::string key = std::to_string(color.red) + "|" +
                    std::to_string(color.green) + "|" +
                    std::to_string(color.blue) + text;
  auto i = m_font_textures.find(key);
  TexturePtr texture;
  if(i != m_font_textures.end())
    texture = i->second;
  if(!texture) {
    texture = create_text_texture(font, text, color);
    m_font_textures[key] = texture;
  }
  return texture;
}

void
TextureManager::reap_cache_entry(const std::string& filename)
{
  auto i = m_image_textures.find(filename);
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

TexturePtr
TextureManager::create_image_texture_raw(const std::string& filename, const Rect& rect)
{
  SDL_Surface *image = nullptr;

  auto i = m_surfaces.find(filename);
  if (i != m_surfaces.end())
  {
    image = i->second;
  }
  else
  {
    image = IMG_Load_RW(get_physfs_SDLRWops(filename), 1);
    if (!image)
    {
      std::ostringstream msg;
      msg << "Couldn't load image '" << filename << "' :" << SDL_GetError();
      throw std::runtime_error(msg.str());
    }

    m_surfaces[filename] = image;
  }

  auto format = image->format;
  if(format->Rmask == 0 && format->Gmask == 0 && format->Bmask == 0 && format->Amask == 0) {
    log_debug << "Wrong surface format for image " << filename << ". Compensating." << std::endl;
    image = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_RGBA8888, 0);
  }

  SDLSurfacePtr subimage(SDL_CreateRGBSurfaceFrom(static_cast<uint8_t*>(image->pixels) +
                                                  rect.top * image->pitch +
                                                  rect.left * image->format->BytesPerPixel,
                                                  rect.get_width(), rect.get_height(),
                                                  image->format->BitsPerPixel,
                                                  image->pitch,
                                                  image->format->Rmask,
                                                  image->format->Gmask,
                                                  image->format->Bmask,
                                                  image->format->Amask));
  if (!subimage)
  {
    throw std::runtime_error("SDL_CreateRGBSurfaceFrom() call failed");
  }

  return VideoSystem::current()->new_texture(subimage.get());
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
    TexturePtr texture = VideoSystem::current()->new_texture(image.get());
    image.reset(NULL);
    return texture;
  }
}

TexturePtr
TextureManager::create_text_texture(TTF_Font* font, const std::string& text,
                                    const Color& color)
{
  Uint8 r = static_cast<Uint8>(color.red * 255);
  Uint8 g = static_cast<Uint8>(color.green * 255);
  Uint8 b = static_cast<Uint8>(color.blue * 255);
  Uint8 a = static_cast<Uint8>(color.alpha * 255);
  SDLSurfacePtr image(TTF_RenderUTF8_Blended(font, text.c_str(), {r, g, b, a}));
  if (!image)
  {
    std::ostringstream msg;
    msg << "Couldn't load image '" << text << "' :" << SDL_GetError();
    throw std::runtime_error(msg.str());
  }
  else
  {
    TexturePtr texture = VideoSystem::current()->new_texture(image.get());
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
      TexturePtr texture = VideoSystem::current()->new_texture(image.get());
      image.reset(NULL);
      return texture;
    }
  }
}

/* EOF */
