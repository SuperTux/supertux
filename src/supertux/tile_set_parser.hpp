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
#include <boost/optional.hpp>

#include "math/rect.hpp"
#include "supertux/tile.hpp"

class ReaderMapping;
class TileSet;

class TileSetParser final
{
private:
  TileSet&    m_tileset;
  std::string m_filename;
  std::string m_tiles_path;

public:
  TileSetParser(TileSet& tileset, const std::string& filename);

  void parse(uint32_t start = 0, uint32_t end = 0, int32_t offset = 0);

private:
  void parse_tile(const ReaderMapping& reader, uint32_t min, uint32_t max, int32_t offset);
  void parse_tiles(const ReaderMapping& reader, uint32_t min, uint32_t max, int32_t offset);
  std::vector<SurfacePtr> parse_imagespecs(const ReaderMapping& cur,
                                           const boost::optional<Rect>& region = boost::none) const;

private:
  TileSetParser(const TileSetParser&) = delete;
  TileSetParser& operator=(const TileSetParser&) = delete;
};

#endif

/* EOF */
