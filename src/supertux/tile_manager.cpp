//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "supertux/tile_manager.hpp"

#include <limits>

#include "supertux/tile_set.hpp"
#include "util/reader_collection.hpp"
#include "util/reader_mapping.hpp"

TileManager::TileManager() :
  tilesets()
{
}

TileManager::~TileManager()
{
}

TileSet*
TileManager::get_tileset(const std::string &filename)
{
  TileSets::const_iterator i = tilesets.find(filename);
  if(i != tilesets.end())
  {
    return i->second.get();
  }
  else
  {
    std::unique_ptr<TileSet> tileset(new TileSet(filename));
    TileSet* result = tileset.get();
    tilesets.insert(std::make_pair(filename, std::move(tileset)));
    return result;
  }
}

/* EOF */
