//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
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

#ifndef TILE_H
#define TILE_H

#include <vector>
#include "SDL.h"
#include "video/surface.h"

class Vector;
class LispReader;

/**
Tile Class
*/
class Tile
{
public:
  Tile();
  ~Tile();

  /// parses the tile and returns it's id number
  int read(LispReader& reader);

  int id;

  std::vector<Surface*> images;
  std::vector<Surface*> editor_images;
  
  /// bitset for tileflags
  enum {
      /** solid tile that is indestructable by Tux */
      SOLID     = 0x0001,
      /** uni-directional solid tile */
      UNISOLID  = 0x0002,
      /** a brick that can be destroyed by jumping under it */
      BRICK     = 0x0004,
      /** an ice brick that makes tux sliding more than usual */
      ICE       = 0x0008,
      /** a water tile in which tux starts to swim */
      WATER     = 0x0010,
      /** a tile that hurts the player if he touches it */
      SPIKE     = 0x0020,
      /** Bonusbox, content is stored in \a data */
      FULLBOX   = 0x0040,
      /** Tile is a coin */
      COIN      = 0x0080,
      /** the level should be finished when touching a goaltile.
       * if data is 0 then the endsequence should be triggered, if data is 1
       * then we can finish the level instantly.
       */
      GOAL      = 0x0100
  };

  /** tile attributes */
  Uint32 attributes;
  
  /** General purpose data attached to a tile (content of a box, type of coin)*/
  int data;

  /** Id of the tile that is going to replace this tile once it has
      been collected or jumped at */
  int next_tile;

  int anim_speed;

  /** This is the angle of the slope. Set to 0, if this is no slope. */
  float slope_angle;
  
  /** Draw a tile on the screen: */
  static void draw(const Vector& pos, unsigned int c, Uint8 alpha = 255);

  /// returns the width of the tile in pixels
  int getWidth() const
  { 
    if(!images.size())
      return 0;
    return images[0]->w;
  }

  /// returns the height of the tiles in pixels
  int getHeight() const
  {
    if(!images.size())
      return 0;
    return images[0]->h;
  }
};

#endif

/* EOF */
