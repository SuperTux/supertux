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

#include <config.h>

#include "video/texture_manager.hpp"

#include <assert.h>
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "supertux/gameconfig.hpp"
#include "video/gl_texture.hpp"
#include "video/glutil.hpp"
#include "physfs/physfs_sdl.hpp"
#include "video/texture.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "video/video_systems.hpp"

TextureManager* texture_manager = NULL;

TextureManager::TextureManager()
{
}

TextureManager::~TextureManager()
{
  for(ImageTextures::iterator i = image_textures.begin();
      i != image_textures.end(); ++i) {
    if(i->second == NULL)
      continue;
    log_warning << "Texture '" << i->first << "' not freed" << std::endl;
    delete i->second;
  }
}

Texture*
TextureManager::get(const std::string& _filename)
{
  std::string filename = FileSystem::normalize(_filename);
  ImageTextures::iterator i = image_textures.find(filename);

  Texture* texture = NULL;
  if(i != image_textures.end())
    texture = i->second;

  if(texture == NULL) {
    texture = create_image_texture(filename);
    image_textures[filename] = texture;
  }

  return texture;
}

void
TextureManager::release(Texture* texture)
{
  image_textures.erase(texture->get_filename());
  delete texture;
}

#ifdef HAVE_OPENGL
void
TextureManager::register_texture(GL::Texture* texture)
{
  textures.insert(texture);
}

void
TextureManager::remove_texture(GL::Texture* texture)
{
  textures.erase(texture);
}
#endif

Texture*
TextureManager::create_image_texture(const std::string& filename)
{
  try {

    SDL_Surface* image = IMG_Load_RW(get_physfs_SDLRWops(filename), 1);
    if(image == 0) {
      std::ostringstream msg;
      msg << "Couldn't load image '" << filename << "' :" << SDL_GetError();
      throw std::runtime_error(msg.str());
    }

    Texture* result = 0;
    try {
      result = new_texture(image);
      result->set_filename(filename);
    } catch(...) {
      delete result;
      SDL_FreeSurface(image);
      throw;
    }

    SDL_FreeSurface(image);
    return result;

  } catch (const std::runtime_error& err) {
    const std::string dummy_texture_fname = "images/engine/missing.png";
    if (filename == dummy_texture_fname) throw err;

    // on error, try loading placeholder file
    try {

      Texture* tex = create_image_texture(dummy_texture_fname);
      log_warning << "Couldn't load texture '" << filename << "' (now using dummy texture): " << err.what() << std::endl;
      return tex;

    } catch (...) {

      // on error (when loading placeholder), try using empty surface
      try {

	SDL_Surface* image = SDL_CreateRGBSurface(0, 1024, 1024, 8, 0, 0, 0, 0);
	if(image == 0) {
	  throw err;
	}

	Texture* result = 0;
	try {
	  result = new_texture(image);
	  result->set_filename("-dummy-texture-.png");
	} catch(...) {
	  delete result;
	  SDL_FreeSurface(image);
	  throw err;
	}

	SDL_FreeSurface(image);
        log_warning << "Couldn't load texture '" << filename << "' (now using empty one): " << err.what() << std::endl;
	return result;

      // on error (when trying to use empty surface), give up
      } catch (const std::runtime_error& err) {
	throw err;
      }
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
    save_texture(dynamic_cast<GL::Texture *>(i->second));
  }
}

void
TextureManager::save_texture(GL::Texture* texture)
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
