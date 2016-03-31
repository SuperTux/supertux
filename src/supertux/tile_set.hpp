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

#ifndef HEADER_SUPERTUX_SUPERTUX_TILE_SET_HPP
#define HEADER_SUPERTUX_SUPERTUX_TILE_SET_HPP

#include <stdint.h>

#include "supertux/tile.hpp"
#include "util/log.hpp"

class TileSet
{
private:
  std::vector<std::unique_ptr<Tile> > m_tiles;

public:
  TileSet(const std::string& filename);
  TileSet();
  ~TileSet();

  void add_tile(int id, std::unique_ptr<Tile> tile);

  const Tile* get(const uint32_t id) const
  {
    assert(id < m_tiles.size());
    Tile* tile = m_tiles[id].get();
    if(!tile) {
      log_warning << "Invalid tile: " << id << std::endl;
      return m_tiles[0].get();
    } else {
      tile->load_images();
      return tile;
    }
  }

  uint32_t get_max_tileid() const
  {
    return m_tiles.size();
  }
};

#endif

/* EOF */
