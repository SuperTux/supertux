//  $Id$
//
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
#include <assert.h>
#include <SDL.h>
#include "video/drawing_context.hpp"
#include "log.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "lisp/list_iterator.hpp"
#include "tile.hpp"
#include "tile_manager.hpp"
#include "resources.hpp"

TileManager::TileManager()
{
  tiles.resize(1, 0);
  tiles[0] = new Tile();
}

TileManager::~TileManager()
{
  for(Tiles::iterator i = tiles.begin(); i != tiles.end(); ++i)
    delete *i;
  tiles.clear();
}

void TileManager::load_tileset(std::string filename, unsigned int start, unsigned int end, int offset)
{

#ifdef DEBUG
  Uint32 ticks = SDL_GetTicks();
#endif

  std::string::size_type t = filename.rfind('/');
  if(t == std::string::npos) {
    tiles_path = "";
  } else {
    tiles_path = filename.substr(0, t+1);
  }

  lisp::Parser parser;
  const lisp::Lisp* root = parser.parse(filename);

  const lisp::Lisp* tiles_lisp = root->get_lisp("supertux-tiles");
  if(!tiles_lisp)
    throw std::runtime_error("file is not a supertux tiles file.");

  lisp::ListIterator iter(tiles_lisp);
  while(iter.next()) {
    if(iter.item() == "tile") {
      Tile* tile = new Tile();
      tile->parse(*(iter.lisp()));

      if ((tile->id < start) || (tile->id > end)) {
        delete tile;
        continue;
      }
      tile->id += offset;

      if(tile->id >= tiles.size())
        tiles.resize(tile->id+1, 0);

      if(tiles[tile->id] != 0) {
        log_warning << "Tile with ID " << tile->id << " redefined" << std::endl;
        delete tile;
      } else {
        tiles[tile->id] = tile;
      }
    } else if(iter.item() == "tilegroup") {
      TileGroup tilegroup;
      const lisp::Lisp* tilegroup_lisp = iter.lisp();
      tilegroup_lisp->get("name", tilegroup.name);
      tilegroup_lisp->get_vector("tiles", tilegroup.tiles);
      tilegroups.insert(tilegroup);
    } else if (iter.item() == "tiles") {
      // List of ids (use 0 if the tile should be ignored)
      std::vector<unsigned int> ids;
      // List of attributes of the tile
      std::vector<unsigned int> attributes;
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

      if (ids.size() != attributes.size())
        {
          std::ostringstream err;
          err << "Number of ids (" << ids.size() <<  ") and attributes (" << attributes.size()
              << ") missmatch for image '" << image << "', but must be equal";
          throw std::runtime_error(err.str());
        }

      for(std::vector<unsigned int>::size_type i = 0; i < ids.size() && i < width*height; ++i)
        {
          if (ids[i])
            {
              if ((ids[i] < start) || (ids[i] > end)) {
                continue;
              }
              ids[i] += offset;
              if(ids[i] >= tiles.size())
                tiles.resize(ids[i]+1, 0);

              int x = 32*(i % width);
              int y = 32*(i / width);
              Tile* tile = new Tile(ids[i], attributes[i], Tile::ImageSpec(image, Rect(x, y, x + 32, y + 32)));
              if (tiles[ids[i]] == 0) {
                tiles[ids[i]] = tile;
              } else {
                log_warning << "Tile with ID " << ids[i] << " redefined" << std::endl;
                delete tile;
              }
            }
        }

    } else if(iter.item() == "properties") {
      // deprecated
    } else {
      log_warning << "Unknown symbol '" << iter.item() << "' tile defintion file" << std::endl;
    }
  }

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

#ifdef DEBUG
  log_debug << "Tiles loaded in " << (SDL_GetTicks() - ticks) / 1000.0 << " seconds" << std::endl;
#endif

}
