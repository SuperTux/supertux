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

#include <map>
#include <vector>
#include "texture.h"
#include "globals.h"
#include "lispreader.h"
#include "setup.h"

/**
Tile Class
*/
class Tile
{
public:
  Tile();
  ~Tile();

  int id;

  std::vector<Surface*> images;
  std::vector<Surface*> editor_images;
  
  std::vector<std::string>  filenames;

  /** solid tile that is indestructable by Tux */
  bool solid;

  /** a brick that can be destroyed by jumping under it */
  bool brick;

  /** FIXME: ? */
  bool ice;

  /** water */
  bool water;

  /** Bonusbox, content is stored in \a data */
  bool fullbox;

  /** Tile is a distro/coin */
  bool distro;

  /** General purpose data attached to a tile (content of a box, type of coin) */
  int data;

  /** Id of the tile that is going to replace this tile once it has
      been collected or jumped at */
  int next_tile;

  int anim_speed;
  
  /** Draw a tile on the screen: */
  static void draw(float x, float y, unsigned int c, Uint8 alpha = 255);
};

struct TileGroup
{
  std::string name;
  std::vector<int> tiles;
};

class TileManager
{
 private:
  TileManager();
  ~TileManager();
  
  std::vector<Tile*> tiles;
  static TileManager* instance_ ;
  static std::vector<TileGroup>* tilegroups_;
  void load_tileset(std::string filename);

  std::string current_tileset;
  
 public:
  static TileManager* instance() { return instance_ ? instance_ : instance_ = new TileManager(); }
  static void destroy_instance() { delete instance_; instance_ = 0; }
  
  static std::vector<TileGroup>* tilegroups() { return tilegroups_ ? tilegroups_ : tilegroups_ = new std::vector<TileGroup>; }
  Tile* get(unsigned int id) {
    if(id < tiles.size())
      {
        return tiles[id]; 
      }
    else
      {
        // Never return 0, but return the 0th tile instead so that
        // user code doesn't have to check for NULL pointers all over
        // the place
        return tiles[0]; 
      } 
  }
};

#endif
