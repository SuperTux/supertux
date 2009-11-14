//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
#include <config.h>

#include <memory>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <limits>
#include <assert.h>
#include <SDL.h>
#include "video/drawing_context.hpp"
#include "log.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "lisp/list_iterator.hpp"
#include "tile.hpp"
#include "tile_set.hpp"
#include "tile_manager.hpp"
#include "resources.hpp"

TileManager *tile_manager    = NULL;
TileSet     *current_tileset = NULL;

TileManager::TileManager()
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

TileSet* TileManager::parse_tileset_definition(const lisp::Lisp& reader)
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

