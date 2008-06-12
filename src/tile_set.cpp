//  $Id$
//
//  SuperTux
//  Copyright (C) 2008 Matthias Braun <matze@braunis.de>
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

#include "tile_set.hpp"
#include "log.hpp"
#include "file_system.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "lisp/list_iterator.hpp"

TileSet::TileSet()
  : tiles_path(""), tiles_loaded(false)
{
  tiles.resize(1, 0);
  tiles[0] = new Tile(this);
}

TileSet::TileSet(const std::string& filename)
  : tiles_path(""), tiles_loaded(true)
{
  tiles_path = FileSystem::dirname(filename);

  tiles.resize(1, 0);
  tiles[0] = new Tile(this);

  lisp::Parser parser;
  const lisp::Lisp* root = parser.parse(filename);

  const lisp::Lisp* tiles_lisp = root->get_lisp("supertux-tiles");
  if(!tiles_lisp)
    throw std::runtime_error("file is not a supertux tiles file.");

  lisp::ListIterator iter(tiles_lisp);
  while(iter.next()) {
    if(iter.item() == "tile") {
      Tile* tile = new Tile(this);
      uint32_t id = tile->parse(*(iter.lisp()));

      if(id >= tiles.size())
        tiles.resize(id+1, 0);

      if(tiles[id] != 0) {
        log_warning << "Tile with ID " << id << " redefined" << std::endl;
        delete tile;
      } else {
        tiles[id] = tile;
      }
    } else if(iter.item() == "tilegroup") {
      /* tilegroups are only interesting for the editor */
    } else if (iter.item() == "tiles") {
      // List of ids (use 0 if the tile should be ignored)
      std::vector<uint32_t> ids;
      // List of attributes of the tile
      std::vector<uint32_t> attributes;
      std::string image;

      // width and height of the image in tile units, this is used for two
      // purposes:
      //  a) so we don't have to load the image here to know its dimensions
      //  b) so that the resulting 'tiles' entry is more robust,
      //  ie. enlarging the image won't break the tile id mapping
      // FIXME: height is actually not used, since width might be enough for
      // all purposes, still feels somewhat more natural this way
      unsigned int width  = 0;
      unsigned int height = 0;

      iter.lisp()->get_vector("ids",        ids);
      iter.lisp()->get_vector("attributes", attributes);
      iter.lisp()->get("image",      image);
      iter.lisp()->get("width",      width);
      iter.lisp()->get("height",     height);

      if (ids.size() != attributes.size()) {
        std::ostringstream err;
        err << "Number of ids (" << ids.size() <<  ") and attributes (" << attributes.size()
          << ") mismatch for image '" << image << "', but must be equal";
        throw std::runtime_error(err.str());
      }

      for(std::vector<uint32_t>::size_type i = 0; i < ids.size() && i < width*height; ++i) {
        if (ids[i] == 0)
          continue;

        if(ids[i] >= tiles.size())
          tiles.resize(ids[i]+1, 0);

        int x = 32*(i % width);
        int y = 32*(i / width);
        Tile* tile = new Tile(this, attributes[i], Tile::ImageSpec(image, Rect(x, y, x + 32, y + 32)));
        if (tiles[ids[i]] == 0) {
          tiles[ids[i]] = tile;
        } else {
          log_warning << "Tile with ID " << ids[i] << " redefined" << std::endl;
          delete tile;
        }
      }
    } else if(iter.item() == "properties") {
      // deprecated
    } else {
      log_warning << "Unknown symbol '" << iter.item() << "' in tileset file" << std::endl;
    }
  }
}

TileSet::~TileSet()
{
  if(tiles_loaded) {
    for(Tiles::iterator i = tiles.begin(); i != tiles.end(); ++i)
      delete *i;
  }
}

void TileSet::merge(const TileSet *tileset, uint32_t start, uint32_t end,
                    uint32_t offset)
{
  for(uint32_t id = start; id <= end && id < tileset->tiles.size(); ++id) {
    uint32_t dest_id = id - start + offset;

    if(dest_id >= tiles.size())
      tiles.resize(dest_id + 1, 0);

    if(dest_id == 0)
      continue;

    Tile *tile = tileset->tiles[id];
    if(tile == NULL)
        continue;

    if(tiles[dest_id] != NULL) {
      log_warning << "tileset merge resulted in multiple definitions for id "
                  << dest_id << "(originally " << id << ")" << std::endl;
    }
    tiles[dest_id] = tile;
  }
}
