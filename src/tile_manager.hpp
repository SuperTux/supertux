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

#ifndef HEADER_TILE_MANAGER_HXX
#define HEADER_TILE_MANAGER_HXX

#include <set>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <stdint.h>
#include <assert.h>
#include "msg.hpp"
#include "tile.hpp"

struct TileGroup
{
  friend bool operator<(const TileGroup& lhs, const TileGroup& rhs)
  { return lhs.name < rhs.name; };
  friend bool operator>(const TileGroup& lhs, const TileGroup& rhs)
  { return lhs.name > rhs.name; };

  std::string name;
  std::vector<int> tiles;
};

class TileManager
{
private:
  typedef std::vector<Tile*> Tiles;
  Tiles tiles;

  static TileManager* instance_ ;
  std::set<TileGroup> tilegroups;

  std::string tiles_path;
  
  void load_tileset(std::string filename);

public:
  TileManager(const std::string& filename);
  ~TileManager();

  const std::set<TileGroup>& get_tilegroups() const
  {
    return tilegroups;
  }

  const Tile* get(uint32_t id) const
  {
    //FIXME: Commenting out tiles in sprites.strf makes tiles.size() fail - it's being set to the first tile commented out.
    assert(id < tiles.size());
    Tile* tile = tiles[id];
    if(!tile) {
      msg_warning << "Invalid tile: " << id << std::endl;
      return tiles[0];
    }

    if(tile->images.size() == 0 && tile->imagespecs.size() != 0)
      tile->load_images(tiles_path);
    
    return tile;
  }

  uint32_t get_max_tileid() const
  {
    return tiles.size();
  }

  int get_default_width() const
  {
    return 32;
  }

  int get_default_height() const
  {
    return 32;
  }
};

#endif
