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

#include <map>
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
  int id;

  std::vector<texture_type> images;
  std::vector<texture_type> editor_images;
  
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
  std::vector<Tile*> tiles;
  static TileManager* instance_ ;
  static std::vector<TileGroup>* tilegroups_;
  void load_tileset(std::string filename);
  
 public:
  static TileManager* instance() { return instance_ ? instance_ : instance_ = new TileManager(); }
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
