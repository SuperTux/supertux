//  SuperTux
//  Copyright (C) 2008 Matthias Braun <matze@braunis.de>
//                     Ingo Ruhnke <grumbel@gmx.de>
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

#include "supertux/tile_set_parser.hpp"

#include <stdexcept>
#include <sstream>

#include "lisp/parser.hpp"
#include "lisp/list_iterator.hpp"
#include "supertux/tile_set.hpp"
#include "util/file_system.hpp"
#include "util/reader.hpp"

TileSetParser::TileSetParser(TileSet& tileset, const std::string& filename) :
  m_tileset(tileset),
  m_filename(filename)
{  
}

void
TileSetParser::parse()
{
  m_tileset.tiles_path = FileSystem::dirname(m_filename);

  m_tileset.tiles.resize(1, 0);
  m_tileset.tiles[0] = new Tile(m_tileset);

  lisp::Parser parser;
  const lisp::Lisp* root = parser.parse(m_filename);

  const lisp::Lisp* tiles_lisp = root->get_lisp("supertux-tiles");
  if(!tiles_lisp)
    throw std::runtime_error("file is not a supertux tiles file.");

  lisp::ListIterator iter(tiles_lisp);
  while(iter.next()) {
    if(iter.item() == "tile") {
      std::auto_ptr<Tile> tile(new Tile(m_tileset));
      uint32_t id = tile->parse(*(iter.lisp()));

      if(id >= m_tileset.tiles.size())
        m_tileset.tiles.resize(id+1, 0);

      if(m_tileset.tiles[id] != 0) {
        log_warning << "Tile with ID " << id << " redefined" << std::endl;
      } else {
        m_tileset.tiles[id] = tile.release();
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

        if(ids[i] >= m_tileset.tiles.size())
          m_tileset.tiles.resize(ids[i]+1, 0);

        int x = 32*(i % width);
        int y = 32*(i / width);
        std::auto_ptr<Tile> tile(new Tile(m_tileset, images, Rect(x, y, x + 32, y + 32),
                                          (has_attributes ? attributes[i] : 0), (has_datas ? datas[i] : 0), animfps));
        if (m_tileset.tiles[ids[i]] == 0) {
          m_tileset.tiles[ids[i]] = tile.release();
        } else {
          log_warning << "Tile with ID " << ids[i] << " redefined" << std::endl;
        }
      }
    } else if(iter.item() == "properties") {
      // deprecated
    } else {
      log_warning << "Unknown symbol '" << iter.item() << "' in tileset file" << std::endl;
    }
  }
}

/* EOF */
