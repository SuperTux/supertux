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

class Tile;

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
  
  std::vector<Tile*> tiles;
  static TileManager* instance_ ;
  static std::set<TileGroup>* tilegroups_;
  void load_tileset(std::string filename);

  std::string current_tileset;
  
 public:
  static TileManager* instance()
  { return instance_ ? instance_ : instance_ = new TileManager(); }
  static void destroy_instance()
  { delete instance_; instance_ = 0; }

  void draw_tile(DrawingContext& context, unsigned int id,
      const Vector& pos, int layer);
  
  static std::set<TileGroup>* tilegroups() { if(!instance_) { instance_ = new TileManager(); } return tilegroups_ ? tilegroups_ : tilegroups_ = new std::set<TileGroup>; }

  unsigned int total_ids()
    { return tiles.size(); }

  Tile& get(unsigned int id) {

    if(id < tiles.size())
      {
        return *tiles[id]; 
      }
    else
      {
        // Never return 0, but return the 0th tile instead so that
        // user code doesn't have to check for NULL pointers all over
        // the place
        return *tiles[0]; 
      } 
  }
};

#endif

/* EOF */
