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

#include "supertux/gameconfig.hpp"
#include "video/gl/gl_texture.hpp"

namespace {

inline bool is_power_of_2(int v)
{
  return (v & (v-1)) == 0;
}

inline int next_power_of_two(int val)
{
  int result = 1;
  while(result < val)
    result *= 2;
  return result;
}

} // namespace

GLTexture::GLTexture(unsigned int width, unsigned int height) :
  handle(),
  texture_width(),
  texture_height(),
  image_width(),
  image_height()
{
#ifdef GL_VERSION_ES_CM_1_0
  assert(is_power_of_2(width));
  assert(is_power_of_2(height));
#endif
  texture_width  = width;
  texture_height = height;
  image_width  = width;
  image_height = height;

  assert_gl("before creating texture");
  glGenTextures(1, &handle);

  try {
    glBindTexture(GL_TEXTURE_2D, handle);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width,
                 texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    set_texture_params();
  } catch(...) {
    glDeleteTextures(1, &handle);
    throw;
  }
}

GLTexture::GLTexture(SDL_Surface* image) :
  handle(),
  texture_width(),
  texture_height(),
  image_width(),
  image_height()
{
#ifdef GL_VERSION_ES_CM_1_0
  texture_width = next_power_of_two(image->w);
  texture_height = next_power_of_two(image->h);
#else
  if (GLEW_ARB_texture_non_power_of_two)
  {
    texture_width  = image->w;
    texture_height = image->h;
  }
  else
  {
    texture_width = next_power_of_two(image->w);
    texture_height = next_power_of_two(image->h);
  }
#endif

  image_width  = image->w;
  image_height = image->h;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  SDL_Surface* convert = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                              texture_width, texture_height, 32,
                                              0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#else
  SDL_Surface* convert = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                              texture_width, texture_height, 32,
                                              0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#endif

  if(convert == 0) {
    throw std::runtime_error("Couldn't create texture: out of memory");
  }

  SDL_SetAlpha(image, 0, 0);
  SDL_BlitSurface(image, 0, convert, 0);

  assert_gl("before creating texture");
  glGenTextures(1, &handle);

  try {
    GLenum sdl_format;
    if(convert->format->BytesPerPixel == 3)
      sdl_format = GL_RGB;
    else if(convert->format->BytesPerPixel == 4)
      sdl_format = GL_RGBA;
    else
      assert(false);

    glBindTexture(GL_TEXTURE_2D, handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#ifdef GL_UNPACK_ROW_LENGTH
    glPixelStorei(GL_UNPACK_ROW_LENGTH, convert->pitch/convert->format->BytesPerPixel);
#else
    /* OpenGL ES doesn't support UNPACK_ROW_LENGTH, let's hope SDL didn't add
     * padding bytes, otherwise we need some extra code here... */
    assert(convert->pitch == texture_width * convert->format->BytesPerPixel);
#endif

    if(SDL_MUSTLOCK(convert))
    {
      SDL_LockSurface(convert);
    }

    if (true)
    { // no not use mipmaps
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width,
                 texture_height, 0, sdl_format,
                 GL_UNSIGNED_BYTE, convert->pixels);
    }
    else
    { // build mipmaps
      gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, texture_width,
                        texture_height, sdl_format,
                        GL_UNSIGNED_BYTE, convert->pixels);
    }

    if(SDL_MUSTLOCK(convert))
    {
      SDL_UnlockSurface(convert);
    }

    assert_gl("creating texture");

    set_texture_params();
  } catch(...) {
    glDeleteTextures(1, &handle);
    SDL_FreeSurface(convert);
    throw;
  }
  SDL_FreeSurface(convert);
}

GLTexture::~GLTexture()
{
  glDeleteTextures(1, &handle);
}

void
GLTexture::set_texture_params()
{
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  assert_gl("set texture params");
}

/* EOF */
