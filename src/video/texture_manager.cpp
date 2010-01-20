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
#include "video/video_systems.hpp"

#ifdef HAVE_OPENGL
#include "video/gl/gl_texture.hpp"
#endif

TextureManager::TextureManager() :
  image_textures()
#ifdef HAVE_OPENGL
  ,textures(),
  saved_textures()
#endif
{
}

TextureManager::~TextureManager()
{
  for(ImageTextures::iterator i = image_textures.begin(); i != image_textures.end(); ++i)
  {
    if(i->second.lock())
    {
      log_warning << "Texture '" << i->first << "' not freed" << std::endl;
    }
  }
  image_textures.clear();
}

TexturePtr
TextureManager::get(const std::string& _filename)
{
  std::string filename = FileSystem::normalize(_filename);
  ImageTextures::iterator i = image_textures.find(filename);

  TexturePtr texture;
  if(i != image_textures.end())
    texture = i->second.lock();

  if(!texture) {
    texture = create_image_texture(filename);
    image_textures[texture->get_filename()] = texture;
  }

  return texture;
}

TexturePtr
TextureManager::get(const std::string& filename, const Rect& rect)
{
  // FIXME: implement caching
  return create_image_texture(filename, rect);
}

void
TextureManager::release(Texture* texture)
{
  image_textures.erase(texture->get_filename());
}

#ifdef HAVE_OPENGL
void
TextureManager::register_texture(GLTexture* texture)
{
  textures.insert(texture);
}

void
TextureManager::remove_texture(GLTexture* texture)
{
  textures.erase(texture);
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
    TexturePtr texture = create_dummy_texture();
    texture->set_filename(filename);
    return texture;
  }
}

TexturePtr
TextureManager::create_image_texture_raw(const std::string& filename, const Rect& rect)
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
    else
    {
      if (image->format->palette)
      { // copy the image palette to subimage if present
        SDL_SetColors(subimage.get(), image->format->palette->colors, 0, image->format->palette->ncolors);
      }

      TexturePtr result = VideoSystem::new_texture(subimage.get());
      result->set_filename(filename);
      return result;
    }
  }
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
    TexturePtr texture = create_dummy_texture();
    texture->set_filename(filename);
    return texture;
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
    TexturePtr result = VideoSystem::new_texture(image.get());
    result->set_filename(filename);
    return result;
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
      throw err;
    }
    else
    {
      TexturePtr result = VideoSystem::new_texture(image.get());
      result->set_filename("-dummy-texture-.png");
      log_warning << "Couldn't load texture '" << dummy_texture_fname << "' (now using empty one): " << err.what() << std::endl;
      return result;
    }
  }
}

#ifdef HAVE_OPENGL
void
TextureManager::save_textures()
{
#ifdef GL_PACK_ROW_LENGTH
  /* all this stuff is not support by OpenGL ES */
  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
  glPixelStorei(GL_PACK_IMAGE_HEIGHT, 0);
  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  glPixelStorei(GL_PACK_SKIP_IMAGES, 0);
#endif

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  for(Textures::iterator i = textures.begin(); i != textures.end(); ++i) {
    save_texture(*i);
  }
  for(ImageTextures::iterator i = image_textures.begin();
      i != image_textures.end(); ++i) {
    save_texture(dynamic_cast<GLTexture*>(i->second.lock().get()));
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

  saved_textures.push_back(saved_texture);

  glDeleteTextures(1, &(texture->get_handle()));
  texture->set_handle(0);

  assert_gl("retrieving texture for save");
}

void
TextureManager::reload_textures()
{
#ifdef GL_UNPACK_ROW_LENGTH
  /* OpenGL ES doesn't support these */
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  glPixelStorei(GL_UNPACK_SKIP_IMAGES, 0);
#endif
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  for(std::vector<SavedTexture>::iterator i = saved_textures.begin();
      i != saved_textures.end(); ++i) {
    SavedTexture& saved_texture = *i;

    GLuint handle;
    glGenTextures(1, &handle);
    assert_gl("creating texture handle");

    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
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

  saved_textures.clear();
}
#endif

/* EOF */
