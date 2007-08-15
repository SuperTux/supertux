//  $Id: sdl_texture.hpp 4063 2006-07-21 21:05:23Z anmaster $
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

#ifndef __SDL_TEXTURE_HPP__
#define __SDL_TEXTURE_HPP__

#include <config.h>

#include <SDL.h>

#include "texture.hpp"

class Color;

namespace SDL
{
  class Texture : public ::Texture
  {
  protected:
    SDL_Surface *texture;
    int numerator;
    int denominator;

    struct Cache
    {
      static void ref(SDL_Surface *surface)
      {
        if(surface)
        {
          surface->refcount++;
        }
      }

      SDL_Surface *data[NUM_EFFECTS];

      Cache()
      {
        memset(data, 0, NUM_EFFECTS * sizeof(SDL_Surface *));
      }

      ~Cache()
      {
        std::for_each(data, data + NUM_EFFECTS, SDL_FreeSurface);
      }

      void operator = (const Cache &other)
      {
        std::for_each(other.data, other.data + NUM_EFFECTS, ref);
        std::for_each(data, data + NUM_EFFECTS, SDL_FreeSurface);
        memcpy(data, other.data, sizeof(Cache));
      }

      SDL_Surface *&operator [] (DrawingEffect effect)
      {
        return data[effect];
      }
    };
    mutable std::map<Color, Cache> cache; /**< Cache for processed surfaces */

  public:
    Texture(SDL_Surface* sdlsurface);
    virtual ~Texture();

    SDL_Surface *get_transform(const Color &color, DrawingEffect effect);

    SDL_Surface *get_texture() const
    {
      return texture;
    }

    unsigned int get_texture_width() const
    {
      return texture->w;
    }

    unsigned int get_texture_height() const
    {
      return texture->h;
    }

    unsigned int get_image_width() const
    {
      return texture->w;
    }

    unsigned int get_image_height() const
    {
      return texture->h;
    }
  };
}

#endif
