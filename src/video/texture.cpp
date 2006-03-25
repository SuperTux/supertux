//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
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

#include <assert.h>
#include "glutil.hpp"

static inline bool is_power_of_2(int v)
{
  return (v & (v-1)) == 0;
}

Texture::Texture(unsigned int w, unsigned int h, GLenum glformat)
{
  glformat = 0; // FIXME: this is just so glformat gets used once

  assert(is_power_of_2(w));
  assert(is_power_of_2(h));

  this->width = w;
  this->height = h;

  surface = 0;
}

Texture::Texture(SDL_Surface* image, GLenum glformat)
{
  glformat = 0; // FIXME: this is just so glformat gets used once

  const SDL_PixelFormat* format = image->format;
  if(!is_power_of_2(image->w) || !is_power_of_2(image->h))
    throw std::runtime_error("image has no power of 2 size");
  if(format->BitsPerPixel != 24 && format->BitsPerPixel != 32)
    throw std::runtime_error("image has no 24 or 32 bit color depth");

  this->width = image->w;
  this->height = image->h;

  surface = SDL_DisplayFormatAlpha(image);
}

Texture::~Texture()
{
}

void
Texture::set_texture_params()
{
}

