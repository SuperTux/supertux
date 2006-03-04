//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#ifndef __SURFACE_HPP__
#define __SURFACE_HPP__

#include <SDL_image.h>
#include <string>
#include <list>

class ImageTexture;

/// bitset for drawing effects
enum DrawingEffect {
  /** Don't apply anything */
  NO_EFFECT       = 0x0000,
  /** Draw the Surface upside down */
  VERTICAL_FLIP     = 0x0001,
  /** Draw the Surface from left to down */
  HORIZONTAL_FLIP   = 0x0002,
};

/**
 * Helper class to buffer a pre-transformed SDL_Surface
 */
class TransformedSurface {
    public:
       SDL_Surface* surface;
       DrawingEffect effect;
};

/**
 * A rectangular image.
 * The class basically holds a reference to a texture with additional UV
 * coordinates that specify a rectangular area on this texture
 */
class Surface
{
private:
  friend class DrawingContext;
  friend class Font;
  ImageTexture* texture;

  bool flipx;

  /** draw the surface on the screen, applying a ::DrawingEffect on-the-fly. Transformed Surfaces will be cached in ::transformedSurfaces */
  void draw(float x, float y, float alpha, DrawingEffect effect) const;

  /** draw the surface on the screen, applying a ::DrawingEffect on-the-fly. Transformed Surfaces will be cached in ::transformedSurfaces */
  void draw_part(float src_x, float src_y, float dst_x, float dst_y,
                 float width, float height,
                 float alpha, DrawingEffect effect) const;

  int offsetx; /**< Region in ::surface to be used for blitting */
  int offsety; /**< Region in ::surface to be used for blitting */
  int width;   /**< Region in ::surface to be used for blitting */
  int height;  /**< Region in ::surface to be used for blitting */

  mutable std::list<TransformedSurface*> transformedSurfaces; /**< Cache for pre-transformed surfaces */

public:
  Surface(const std::string& file);
  Surface(const std::string& file, int x, int y, int w, int h);
  Surface(const Surface& other);
  ~Surface();

  /** flip the surface horizontally */
  void hflip();
  
  const Surface& operator= (const Surface& other);

  float get_width() const
  {
    return width;
  }

  float get_height() const
  {
    return height;
  }
};

#endif
