//  SuperTux
//  Copyright (C) 2008 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "supertux/tile_set.hpp"

#include "supertux/tile_set_parser.hpp"

TileSet::TileSet() :
  tiles(1)
{
  tiles[0] = std::unique_ptr<Tile>(new Tile);
}

TileSet::TileSet(const std::string& filename) :
  tiles()
{
  TileSetParser parser(*this, filename);
  parser.parse();

  if (0)
  { // enable this if you want to see a list of free tiles
    log_info << "Last Tile ID is " << tiles.size()-1 << std::endl;
    int last = -1;
    for(int i = 0; i < int(tiles.size()); ++i)
    {
      if (tiles[i] == 0 && last == -1)
      {
        last = i;
      }
      else if (tiles[i] && last != -1)
      {
        log_info << "Free Tile IDs (" << i - last << "): " << last << " - " << i-1 << std::endl;
        last = -1;
      }
    }
  }

  if (0)
  { // enable this to dump the (large) list of tiles to log_debug
    // Two dumps are identical iff the tilesets specify identical tiles
    log_debug << "Tileset in " << filename << std::endl;
    for(int i = 0; i < int(tiles.size()); ++i)
    {
      if(tiles[i] != 0)
      {
        tiles[i]->print_debug(i);
      }
    }
  }
}

TileSet::~TileSet()
{
}

/* EOF */
