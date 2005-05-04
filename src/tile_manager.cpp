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
#include <config.h>

#include <memory>
#include <stdexcept>
#include <assert.h>
#include "video/drawing_context.h"
#include "lisp/lisp.h"
#include "lisp/parser.h"
#include "lisp/list_iterator.h"
#include "tile.h"
#include "tile_manager.h"
#include "resources.h"

TileManager::TileManager(const std::string& filename)
{
  load_tileset(filename);
}

TileManager::~TileManager()
{
  for(Tiles::iterator i = tiles.begin(); i != tiles.end(); ++i)
    delete *i;
}

void TileManager::load_tileset(std::string filename)
{
  // free old tiles
  for(Tiles::iterator i = tiles.begin(); i != tiles.end(); ++i)
    delete *i;
  tiles.clear();

  std::string::size_type t = filename.rfind('/');
  if(t == std::string::npos) {
    tiles_path = "";
  } else {
    tiles_path = filename.substr(0, t+1);
  }

  lisp::Parser parser;
  std::auto_ptr<lisp::Lisp> root (parser.parse(
        get_resource_filename(filename)));

  const lisp::Lisp* tiles_lisp = root->get_lisp("supertux-tiles");
  if(!tiles_lisp)
    throw std::runtime_error("file is not a supertux tiles file.");

  lisp::ListIterator iter(tiles_lisp);
  while(iter.next()) {
    if(iter.item() == "tile") {
      Tile* tile = new Tile();
      tile->parse(*(iter.lisp()));
      while(tile->id >= tiles.size()) {
        tiles.push_back(0);
      }
      tiles[tile->id] = tile;
    } else if(iter.item() == "tilegroup") {
        TileGroup tilegroup;
        const lisp::Lisp* tilegroup_lisp = iter.lisp();
        tilegroup_lisp->get("name", tilegroup.name);
        tilegroup_lisp->get_vector("tiles", tilegroup.tiles);
        tilegroups.insert(tilegroup);
    } else if(iter.item() == "properties") {
      // deprecated
    } else {
      std::cerr << "Unknown symbol '" << iter.item() << "'.\n";
    }
  }
}

