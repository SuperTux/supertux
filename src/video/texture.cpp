//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <config.h>

#include "texture.hpp"

#include <GL/gl.h>
#include <assert.h>
#include "glutil.hpp"

static inline bool is_power_of_2(int v)
{
  return (v & (v-1)) == 0;
}

Texture::Texture(unsigned int w, unsigned int h, GLenum glformat)
{
  assert(is_power_of_2(w));
  assert(is_power_of_2(h));

  this->width = w;
  this->height = h;
  
  assert_gl("before creating texture");
  glGenTextures(1, &handle);
  
  try {
    glBindTexture(GL_TEXTURE_2D, handle);

    glTexImage2D(GL_TEXTURE_2D, 0, glformat, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, 0);

    set_texture_params();
  } catch(...) {
    glDeleteTextures(1, &handle);
    throw;
  }
}

Texture::Texture(SDL_Surface* image, GLenum glformat)
{
  const SDL_PixelFormat* format = image->format;
  if(!is_power_of_2(image->w) || !is_power_of_2(image->h))
    throw std::runtime_error("image has no power of 2 size");
  if(format->BitsPerPixel != 24 && format->BitsPerPixel != 32)
    throw std::runtime_error("image has no 24 or 32 bit color depth");
  
  this->width = image->w;
  this->height = image->h;

  assert_gl("before creating texture");
  glGenTextures(1, &handle);
  
  try {
    GLenum sdl_format;
    if(format->BytesPerPixel == 3)
      sdl_format = GL_RGB;
    else if(format->BytesPerPixel == 4)
      sdl_format = GL_RGBA;
    else
      assert(false);

    glBindTexture(GL_TEXTURE_2D, handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, image->pitch/format->BytesPerPixel);
    glTexImage2D(GL_TEXTURE_2D, 0, glformat, width, height, 0, sdl_format,
            GL_UNSIGNED_BYTE, image->pixels);

    assert_gl("creating texture");

    set_texture_params();    
  } catch(...) {
    glDeleteTextures(1, &handle);
    throw;
  }
}

Texture::~Texture()
{
  glDeleteTextures(1, &handle);
}

void
Texture::set_texture_params()
{
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  assert_gl("set texture params");
}

