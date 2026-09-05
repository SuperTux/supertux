//  SuperTux
//  Copyright (C) 2026 Tobias Markus <tobbi.bugs@googlemail.com>
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

#pragma once

#include <string>

namespace {
  const std::string& DEFAULT_CONVERTERS_PATH = "images/converters/data.stcd";
}

/**
 * This class handles converting deprecated tiles in tilemaps
 * to updated tiles.
 */
class EditorTileConverter
{
public:
  /**
   * Class containing structure about a tile conversion definition
   */
  struct ConverterInfo
  {
    ConverterInfo() :
      title(), author(), description()
    {}

    std::string title;
    std::string author;
    std::string description;
  };

public:
  /**
   * Constructor
   */
  EditorTileConverter();

  /**
   * Loads available tile conversion definitions from the specific file path
   * @param filepath The path to load available converters from
   */
  void load_definitions(const std::string& filepath = DEFAULT_CONVERTERS_PATH);

  /**
   * Returns information about the specified tile converter
   * @param file_path Path to tile conversion file (with extension *.sttc)
   * @return `ConverterInfo` instance containing information about the tile converter
   */
  const ConverterInfo* get_tile_converter_info_for_file(const std::string& file_path) const;

  /**
   * Convert tiles on every tilemap in the level, according to a tile conversion file.
   * @param file The file to use for the basis of the conversion 
   */
  void convert_tiles_by_file(const std::string& file);

  /**
   * Check for any deprecated tiles, used throughout the entire level
   * @param focus If `true`, the first deprecated tile is focused in the level editor
   */
  void check_deprecated_tiles(bool focus = false);

  /**
   * Returns `true` if there are deprecated tiles present in the level, otherwise false
   */
  inline bool has_deprecated_tiles() const { return m_has_deprecated_tiles; }

private:
  /**
   * Centers the editor to a certain tile.
   * @param sector Sector the tile is on
   * @param tilemap Tilemap containing the tile
   * @param idx Index of the tile on the tilemap
   */
  void focus_on_tile(Sector* sector, TileMap* tilemap, int idx);

private:
  std::map<std::string, ConverterInfo> m_converters;
  bool m_has_deprecated_tiles;
};
