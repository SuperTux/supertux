//
// C++ Interface: tile
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TILE_H
#define TILE_H

#include <vector>
#include "texture.h"
#include "globals.h"
#include "lispreader.h"
#include "setup.h"

/**
Tile Class
*/
struct Tile
{
  std::vector<texture_type> images;

  /** solid tile that is indestructable by Tux */
  bool solid;

  /** a brick that can be destroyed by jumping under it */
  bool brick;

  /** FIXME: ? */
  bool ice;

  /** Bonusbox, content is stored in \a data */
  bool fullbox;

  /** Tile is a distro/coin */
  bool distro;

  /** General purpose data attached to a tile (content of a box, type of coin) */
  int data;

  int anim_speed;
  unsigned char alpha;
};

class TileManager
{
 private:
  TileManager();
  std::vector<Tile*> tiles;
  static TileManager* instance_ ;
  void load_tileset(std::string filename);
  
 public:
  static TileManager* instance() { return instance_ ? instance_ : instance_ = new TileManager(); }
  Tile* get(unsigned int id) { if( id < tiles.size()) { return tiles[id]; } else { return NULL; } }
};

#endif
