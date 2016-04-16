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
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "math/rect.hpp"
#include "physfs/physfs_sdl.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "video/sdl_surface_ptr.hpp"
#include "video/texture.hpp"
#include "video/video_system.hpp"

#ifdef HAVE_OPENGL
#include "video/gl/gl_texture.hpp"
#endif

TextureManager::TextureManager() :
  m_image_textures()
  ,m_font_textures()
  ,m_surfaces()
#ifdef HAVE_OPENGL
  ,m_textures(),
  m_saved_textures()
#endif
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
  FontTextures::iterator i = m_font_textures.find(key);

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

#ifdef HAVE_OPENGL
void
TextureManager::register_texture(GLTexture* texture)
{
  m_textures.insert(texture);
}

void
TextureManager::remove_texture(GLTexture* texture)
{
  m_textures.erase(texture);
}
#endif

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

#ifdef HAVE_OPENGL
void
TextureManager::save_textures()
{
#if defined(GL_PACK_ROW_LENGTH) || defined(USE_GLBINDING)
  /* all this stuff is not support by OpenGL ES */
  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
  glPixelStorei(GL_PACK_IMAGE_HEIGHT, 0);
  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  glPixelStorei(GL_PACK_SKIP_IMAGES, 0);
#endif

  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  for(auto& texture : m_textures)
  {
    save_texture(texture);
  }

  for(auto& tex : m_image_textures)
  {
    auto texture = dynamic_cast<GLTexture*>(tex.second.lock().get());
    if(texture == NULL)
      continue;

    save_texture(texture);
  }
}

void
TextureManager::save_texture(GLTexture* texture)
{
  SavedTexture saved_texture;
  saved_texture.texture = texture;
  glBindTexture(GL_TEXTURE_2D, texture->get_handle());

  //this doesn't work with OpenGL ES (but we don't need it on the GP2X anyway)
#ifndef GL_VERSION_ES_CM_1_0
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,
                           &saved_texture.width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT,
                           &saved_texture.height);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BORDER,
                           &saved_texture.border);
  glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      &saved_texture.min_filter);
  glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                      &saved_texture.mag_filter);
  glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                      &saved_texture.wrap_s);
  glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                      &saved_texture.wrap_t);

  size_t pixelssize = saved_texture.width * saved_texture.height * 4;
  saved_texture.pixels = new char[pixelssize];

  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                saved_texture.pixels);
#endif

  m_saved_textures.push_back(saved_texture);

  glDeleteTextures(1, &(texture->get_handle()));
  texture->set_handle(0);

  assert_gl("retrieving texture for save");
}

void
TextureManager::reload_textures()
{
#if defined(GL_UNPACK_ROW_LENGTH) || defined(USE_GLBINDING)
  /* OpenGL ES doesn't support these */
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  glPixelStorei(GL_UNPACK_SKIP_IMAGES, 0);
#endif
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  for(auto& saved_texture : m_saved_textures) {
    GLuint handle;
    glGenTextures(1, &handle);
    assert_gl("creating texture handle");

    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(GL_RGBA),
                 saved_texture.width, saved_texture.height,
                 saved_texture.border, GL_RGBA,
                 GL_UNSIGNED_BYTE, saved_texture.pixels);
    delete[] saved_texture.pixels;
    assert_gl("uploading texture pixel data");

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    saved_texture.min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    saved_texture.mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    saved_texture.wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    saved_texture.wrap_t);

    assert_gl("setting texture_params");
    saved_texture.texture->set_handle(handle);
  }

  m_saved_textures.clear();
}
#endif

/* EOF */
