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

namespace SDL
{
  class Texture : public ::Texture
  {
  protected:
    SDL_Surface *texture;
    //unsigned int width;
    //unsigned int height;

    struct ColorCache
    {
      static const int HASHED_BITS = 3;
      static const int CACHE_SIZE = 1 << (HASHED_BITS * 3);

      static void ref(SDL_Surface *surface)
      {
        if(surface)
        {
          surface->refcount++;
        }
      }

      static int hash(const Color &color)
      {
        return
      ((int) (color.red * ((1 << HASHED_BITS) - 1)) << (HASHED_BITS - 1) * 2) |
      ((int) (color.green * ((1 << HASHED_BITS) - 1)) << (HASHED_BITS - 1)) |
      ((int) (color.blue * ((1 << HASHED_BITS) - 1)) << 0);
      }

      SDL_Surface *data[CACHE_SIZE];

      ColorCache()
      {
        memset(data, 0, CACHE_SIZE * sizeof(SDL_Surface *));
      }

      ~ColorCache()
      {
        std::for_each(data, data + CACHE_SIZE, SDL_FreeSurface);
      }

      void operator = (const ColorCache &other)
      {
        std::for_each(other.data, other.data + CACHE_SIZE, ref);
        std::for_each(data, data + CACHE_SIZE, SDL_FreeSurface);
        memcpy(data, other.data, CACHE_SIZE * sizeof(SDL_Surface *));
      }

      SDL_Surface *&operator [] (const Color &color)
      {
        return data[hash(color)];
      }
    };
    //typedef std::map<Color, SDL_Surface *> ColorCache;
    ColorCache cache[NUM_EFFECTS];

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

    /*unsigned int get_texture_width() const
    {
      return width;
    }

    unsigned int get_texture_height() const
    {
      return height;
    }

    unsigned int get_image_width() const
    {
      return width;
    }

    unsigned int get_image_height() const
    {
      return height;
    }*/
  };
}

#endif
