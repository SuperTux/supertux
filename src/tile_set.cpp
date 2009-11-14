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
#include <stdexcept>

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
      // List of data for the tiles
      std::vector<uint32_t> datas;
      //List of frames that the tiles come in
      std::vector<std::string> images;

      // width and height of the image in tile units, this is used for two
      // purposes:
      //  a) so we don't have to load the image here to know its dimensions
      //  b) so that the resulting 'tiles' entry is more robust,
      //  ie. enlarging the image won't break the tile id mapping
      // FIXME: height is actually not used, since width might be enough for
      // all purposes, still feels somewhat more natural this way
      unsigned int width  = 0;
      unsigned int height = 0;

      iter.lisp()->get("ids",        ids);
      bool has_attributes = iter.lisp()->get("attributes", attributes);
      bool has_datas = iter.lisp()->get("datas", datas);

      if(!iter.lisp()->get("image",      images))
        iter.lisp()->get( "images",      images);

      iter.lisp()->get("width",      width);
      iter.lisp()->get("height",     height);

      float animfps = 10;
      iter.lisp()->get("anim-fps",     animfps);

      if(images.size() <= 0) {
        throw std::runtime_error("No images in tile.");
      }
      if(animfps < 0) {
        throw std::runtime_error("Negative fps.");
      }
      if (ids.size() != width*height) {
        std::ostringstream err;
        err << "Number of ids (" << ids.size() <<  ") and size of image (" << width*height
          << ") mismatch for image '" << images[0] << "', but must be equal";
        throw std::runtime_error(err.str());
      }

      if (has_attributes && ids.size() != attributes.size()) {
        std::ostringstream err;
        err << "Number of ids (" << ids.size() <<  ") and attributes (" << attributes.size()
          << ") mismatch for image '" << images[0] << "', but must be equal";
        throw std::runtime_error(err.str());
      }

      if (has_datas && ids.size() != datas.size()) {
        std::ostringstream err;
        err << "Number of ids (" << ids.size() <<  ") and datas (" << datas.size()
          << ") mismatch for image '" << images[0] << "', but must be equal";
        throw std::runtime_error(err.str());
      }

      for(std::vector<uint32_t>::size_type i = 0; i < ids.size() && i < width*height; ++i) {
        if (ids[i] == 0)
          continue;

        if(ids[i] >= tiles.size())
          tiles.resize(ids[i]+1, 0);

        int x = 32*(i % width);
        int y = 32*(i / width);
        Tile* tile = new Tile(this, images, Rect(x, y, x + 32, y + 32),
              (has_attributes ? attributes[i] : 0), (has_datas ? datas[i] : 0), animfps);
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
          if(tiles[i] == 0)
            continue;
          Tile* t = tiles[i];
          log_debug << " Tile: id " << i << ", data " << t->data << ", attributes " << t->attributes << ":" << std::endl;
          for(std::vector<Tile::ImageSpec>::iterator im = t->imagespecs.begin(); im !=
                t->imagespecs.end(); ++im) {
            log_debug << "  Imagespec: file " << im->file << "; rect " << im->rect << std::endl;
          }
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
