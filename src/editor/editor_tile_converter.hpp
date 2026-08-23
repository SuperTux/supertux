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

class EditorTileConverter
{
public:
  EditorTileConverter() :
    m_has_deprecated_tiles(false)
  {
  }

  /** Convert tiles on every tilemap in the level, according to a tile conversion file. */
  void convert_tiles_by_file(const std::string& file);

  void check_deprecated_tiles(bool focus = false);

  inline bool has_deprecated_tiles() const { return m_has_deprecated_tiles; }

private:
    bool m_has_deprecated_tiles;
};