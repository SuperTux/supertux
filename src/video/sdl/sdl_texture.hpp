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

#ifndef HEADER_SUPERTUX_VIDEO_SDL_TEXTURE_HPP
#define HEADER_SUPERTUX_VIDEO_SDL_TEXTURE_HPP

#include <algorithm>
#include <config.h>

#include <SDL.h>

#include "video/color.hpp"
#include "video/texture.hpp"

class SDLTexture : public Texture
{
protected:
  SDL_Texture* m_texture;
  int m_width;
  int m_height;

public:
  SDLTexture(SDL_Surface* sdlsurface);
  virtual ~SDLTexture();

  SDL_Texture *get_texture() const
  {
    return m_texture;
  }

  unsigned int get_texture_width() const
  {
    return m_width;
  }

  unsigned int get_texture_height() const
  {
    return m_height;
  }

  unsigned int get_image_width() const
  {
    return m_width;
  }

  unsigned int get_image_height() const
  {
    return m_height;
  }

private:
  SDLTexture(const SDLTexture&);
  SDLTexture& operator=(const SDLTexture&);
};

#endif

/* EOF */
