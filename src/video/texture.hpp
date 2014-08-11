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

#ifndef HEADER_SUPERTUX_VIDEO_TEXTURE_HPP
#define HEADER_SUPERTUX_VIDEO_TEXTURE_HPP

#include <config.h>

#include <assert.h>
#include <string>

#include "supertux/globals.hpp"
#include "video/texture_manager.hpp"

/// bitset for drawing effects
enum {
  /** Don't apply anything */
  NO_EFFECT = 0,
  /** Draw the Surface upside down */
  VERTICAL_FLIP = (1<<1),
  /** Draw the Surface from left to down */
  HORIZONTAL_FLIP = (1<<2),
  NUM_EFFECTS
};

typedef unsigned int DrawingEffect;

/**
 * This class is a wrapper around a texture handle. It stores the texture width
 * and height and provides convenience functions for uploading SDL_Surfaces
 * into the texture
 */
class Texture
{
private:
  friend class TextureManager;
  /* The name under which this texture is cached by the texture manager,
   * or the empty string if not. */
  std::string cache_filename;

public:
  Texture() : cache_filename() {}
  virtual ~Texture() 
  {
    if (texture_manager && cache_filename != "")
      /* The cache entry is now useless: its weak pointer to us has been
       * cleared.  Remove the entry altogether to save memory. */
      texture_manager->reap_cache_entry(cache_filename);
  }

  virtual unsigned int get_texture_width() const = 0;
  virtual unsigned int get_texture_height() const = 0;
  virtual unsigned int get_image_width() const = 0;
  virtual unsigned int get_image_height() const = 0;

private:
  Texture(const Texture&);
  Texture& operator=(const Texture&);
};

#endif

/* EOF */
