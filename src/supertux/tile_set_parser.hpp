//  SuperTux
//  Copyright (C) 2008 Matthias Braun <matze@braunis.de>,
//                     Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_TILE_SET_PARSER_HPP
#define HEADER_SUPERTUX_SUPERTUX_TILE_SET_PARSER_HPP

#include <string>
#include <vector>

#include "supertux/tile.hpp"

class ReaderMapping;
class TileSet;

class TileSetParser
{
private:
  TileSet&    m_tileset;
  std::string m_filename;
  std::string m_tiles_path;

public:
  TileSetParser(TileSet& tileset, const std::string& filename);

  void parse();

private:
  void parse_tile(const ReaderMapping& reader);
  void parse_tiles(const ReaderMapping& reader);
  std::vector<Tile::ImageSpec> parse_imagespecs(const ReaderMapping& cur) const;

private:
  TileSetParser(const TileSetParser&);
  TileSetParser& operator=(const TileSetParser&);
};

#endif

/* EOF */
