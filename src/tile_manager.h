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
#include <stdint.h>
#include <assert.h>
#include "tile.h"

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
  TileManager();
  ~TileManager();
  
  typedef std::vector<Tile*> Tiles;
  Tiles tiles;

  static TileManager* instance_ ;
  std::set<TileGroup> tilegroups;
  void load_tileset(std::string filename);

public:
  static TileManager* instance()
  { return instance_ ? instance_ : instance_ = new TileManager(); }
  static void destroy_instance()
  { delete instance_; instance_ = 0; }

  const std::set<TileGroup>& get_tilegroups() const
  {
    return tilegroups;
  }

  const Tile* get(uint32_t id) const
  {
    assert(id < tiles.size());
    Tile* tile = tiles[id];
    if(!tile) {
      std::cout << "TileManager: Invalid tile: " << id << std::endl;
      return tiles[0];
    }

    if(tile->images.size() == 0 && tile->imagespecs.size() != 0)
      tile->load_images();
    
    return tile;
  }

  uint32_t get_max_tileid() const
  {
    return tiles.size();
  }
};

#endif
