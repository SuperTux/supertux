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

#include "video/texture_manager.hpp"

/// bitset for drawing effects
enum DrawingEffect {
  /** Don't apply anything */
  NO_EFFECT,
  /** Draw the Surface upside down */
  VERTICAL_FLIP,
  /** Draw the Surface from left to down */
  HORIZONTAL_FLIP,
  NUM_EFFECTS
};

/**
 * This class is a wrapper around a texture handle. It stores the texture width
 * and height and provides convenience functions for uploading SDL_Surfaces
 * into the texture
 */
class Texture
{
protected:
  int refcount;
  std::string filename;

public:
  Texture() : refcount(0), filename() {}
  virtual ~Texture() {}

  virtual unsigned int get_texture_width() const = 0;
  virtual unsigned int get_texture_height() const = 0;
  virtual unsigned int get_image_width() const = 0;
  virtual unsigned int get_image_height() const = 0;

  std::string get_filename() const
  {
    return filename;
  }

  void set_filename(std::string filename)
  {
    this->filename = filename;
  }

  void ref()
  {
    refcount++;
  }

  void unref()
  {
    assert(refcount > 0);
    refcount--;
    if(refcount == 0)
      release();
  }

private:
  void release()
  {
    texture_manager->release(this);
  }

private:
  Texture(const Texture&);
  Texture& operator=(const Texture&);
};

#endif

/* EOF */
