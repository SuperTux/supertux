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
#ifndef __TEXTURE_HPP__
#define __TEXTURE_HPP__

#include <SDL.h>
#include <GL/gl.h>

/**
 * This class is a wrapper around a texture handle. It stores the texture width
 * and height and provides convenience functions for uploading SDL_Surfaces
 * into the texture
 */
class Texture
{
protected:
  friend class TextureManager;
  GLuint handle;
  unsigned int width;
  unsigned int height;

public:
  Texture(unsigned int width, unsigned int height, GLenum glformat);
  Texture(SDL_Surface* surface, GLenum glformat);
  virtual ~Texture();
  
  GLuint get_handle() const
  {
    return handle;
  }

  unsigned int get_width() const
  {
    return width;
  }

  unsigned int get_height() const
  {
    return height;
  }

private:
  void set_texture_params();
};

#endif

