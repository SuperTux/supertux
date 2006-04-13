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

#ifndef __SURFACE_TEXTURE_HPP__
#define __SURFACE_TEXTURE_HPP__

#include <string>
#include <assert.h>
#include "texture.hpp"

class ImageTexture : public Texture
{
private:
  std::string filename;
  float image_width;
  float image_height;
  int refcount;

public:
  float get_image_width() const
  {
    return image_width;
  }

  float get_image_height() const
  {
    return image_height;
  }
  
  float get_uv_right() const
  {
    return image_width / static_cast<float> (get_width());
  }
  
  float get_uv_bottom() const
  {
    return image_height / static_cast<float> (get_height());
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
  friend class TextureManager;
  
  ImageTexture(SDL_Surface* surface);
  virtual ~ImageTexture();

  void release();
};

#endif

