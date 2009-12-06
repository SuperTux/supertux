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

#include "lisp/list_iterator.hpp"
#include "supertux/tile_set.hpp"

TileManager::TileManager() :
  tilesets()
{
}

TileManager::~TileManager()
{
}

TileSet* TileManager::get_tileset(const std::string &filename)
{
  TileSets::const_iterator i = tilesets.find(filename);
  if(i != tilesets.end())
    return i->second;

  std::auto_ptr<TileSet> tileset (new TileSet(filename));
  tilesets.insert(std::make_pair(filename, tileset.get()));

  return tileset.release();
}

TileSet* TileManager::parse_tileset_definition(const Reader& reader)
{
  std::auto_ptr<TileSet> result(new TileSet());

  lisp::ListIterator iter(&reader);
  while(iter.next()) {
    const std::string& token = iter.item();
    if(token != "tileset") {
      log_warning << "Skipping unrecognized token \"" << token << "\" in tileset definition" << std::endl;
      continue;
    }
    const lisp::Lisp* tileset_reader = iter.lisp();

    std::string file; 
    if (!tileset_reader->get("file", file)) {
      log_warning << "Skipping tileset import without file name" << std::endl;
      continue;
    }

    const TileSet *tileset = get_tileset(file);

    uint32_t start  = 0;
    uint32_t end    = std::numeric_limits<uint32_t>::max();
    uint32_t offset = 0;
    tileset_reader->get("start",  start);
    tileset_reader->get("end",    end);
    tileset_reader->get("offset", offset);

    result->merge(tileset, start, end, offset);
  }

  return result.release();
}

/* EOF */
