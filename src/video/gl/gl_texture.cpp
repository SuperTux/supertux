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

#include "video/gl/gl_texture.hpp"

#include <SDL.h>
#include <assert.h>

#ifdef USE_GLBINDING
  #include <glbinding/ContextInfo.h>
#endif

namespace {

#ifdef GL_VERSION_ES_CM_1_0
inline bool is_power_of_2(int v)
{
  return (v & (v-1)) == 0;
}
#endif

inline int next_power_of_two(int val)
{
  int result = 1;
  while(result < val)
    result *= 2;
  return result;
}

} // namespace

GLTexture::GLTexture(unsigned int width, unsigned int height) :
  m_handle(),
  m_texture_width(),
  m_texture_height(),
  m_image_width(),
  m_image_height()
{
#ifdef GL_VERSION_ES_CM_1_0
  assert(is_power_of_2(width));
  assert(is_power_of_2(height));
#endif
  m_texture_width  = width;
  m_texture_height = height;
  m_image_width  = width;
  m_image_height = height;

  assert_gl("before creating texture");
  glGenTextures(1, &m_handle);

  try {
    glBindTexture(GL_TEXTURE_2D, m_handle);

    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(GL_RGBA), m_texture_width,
				 m_texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    set_texture_params();
  } catch(...) {
    glDeleteTextures(1, &m_handle);
    throw;
  }
}

GLTexture::GLTexture(SDL_Surface* image) :
  m_handle(),
  m_texture_width(),
  m_texture_height(),
  m_image_width(),
  m_image_height()
{
#ifdef GL_VERSION_ES_CM_1_0
  m_texture_width = next_power_of_two(image->w);
  m_texture_height = next_power_of_two(image->h);
#else
#  ifdef USE_GLBINDING
  static auto extensions = glbinding::ContextInfo::extensions();
  if (extensions.find(GLextension::GL_ARB_texture_non_power_of_two) != extensions.end())
  {
    m_texture_width  = image->w;
    m_texture_height = image->h;
  }
#  else
  if (GLEW_ARB_texture_non_power_of_two)
  {
    m_texture_width  = image->w;
    m_texture_height = image->h;
  }
#  endif
  else
  {
    m_texture_width = next_power_of_two(image->w);
    m_texture_height = next_power_of_two(image->h);
  }
#endif

  m_image_width  = image->w;
  m_image_height = image->h;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  SDL_Surface* convert = SDL_CreateRGBSurface(0,
                                              m_texture_width, m_texture_height, 32,
                                              0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#else
  SDL_Surface* convert = SDL_CreateRGBSurface(0,
                                              m_texture_width, m_texture_height, 32,
                                              0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#endif

  if(convert == 0) {
    throw std::runtime_error("Couldn't create texture: out of memory");
  }

  SDL_SetSurfaceBlendMode(image, SDL_BLENDMODE_NONE);
  SDL_BlitSurface(image, 0, convert, 0);

  assert_gl("before creating texture");
  glGenTextures(1, &m_handle);

  try {
    GLenum sdl_format;
    if(convert->format->BytesPerPixel == 3)
      sdl_format = GL_RGB;
    else if(convert->format->BytesPerPixel == 4)
      sdl_format = GL_RGBA;
    else {
      sdl_format = GL_RGBA;
      assert(false);
    }

    glBindTexture(GL_TEXTURE_2D, m_handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#if defined(GL_UNPACK_ROW_LENGTH) || defined(USE_GLBINDING)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, convert->pitch/convert->format->BytesPerPixel);
#else
    /* OpenGL ES doesn't support UNPACK_ROW_LENGTH, let's hope SDL didn't add
     * padding bytes, otherwise we need some extra code here... */
    assert(convert->pitch == m_texture_width * convert->format->BytesPerPixel);
#endif

    if(SDL_MUSTLOCK(convert))
    {
      SDL_LockSurface(convert);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(GL_RGBA),
                 m_texture_width, m_texture_height, 0, sdl_format,
                 GL_UNSIGNED_BYTE, convert->pixels);

    // no not use mipmaps
    if(false)
    {
      glGenerateMipmap(GL_TEXTURE_2D);
    }

    if(SDL_MUSTLOCK(convert))
    {
      SDL_UnlockSurface(convert);
    }

    assert_gl("creating texture");

    set_texture_params();
  } catch(...) {
    glDeleteTextures(1, &m_handle);
    SDL_FreeSurface(convert);
    throw;
  }
  SDL_FreeSurface(convert);
}

GLTexture::~GLTexture()
{
  glDeleteTextures(1, &m_handle);
}

void
GLTexture::set_texture_params()
{
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(GL_LINEAR));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(GL_LINEAR));

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(GL_CLAMP_TO_EDGE));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(GL_CLAMP_TO_EDGE));

  assert_gl("set texture params");
}

/* EOF */
