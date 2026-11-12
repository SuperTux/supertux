//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#include <algorithm>
#include <stdint.h>
#include <string>
#include <vector>

class TileSelection
{
public:
  TileSelection();

  /** Returns the tile id at specific coordinates. The coordinates
      might be bigger than the selection size or negative. */
  uint32_t pos(int x, int y);

  /** Sets the tile selection to a single tile. */
  void set_tile(uint32_t tile);

  /** Clears the selection. */
  void clear();

  /** Returns true when has no tiles or is full of zeros */
  bool empty() const;

  /**
   * Returns true if the tile selection is supposed to erase
   * existing tiles
   * TODO: This only checks if the first tile is 0, is this what we want?
   */
  inline bool erases_tiles() const { return m_tiles.size() > 0 && m_tiles[0] == 0; }

public:
  std::vector<uint32_t> m_tiles;
  int m_width;
  int m_height;

public:
  bool operator==(const TileSelection& other)
  {
    return m_width == other.m_width &&
           m_height == other.m_height &&
           m_tiles == other.m_tiles;
  };

  TileSelection& operator=(const TileSelection& other)
  {
    m_width = other.m_width;
    m_height = other.m_height;
    m_tiles = other.m_tiles;
    return *this;
  };

private:
  TileSelection(const TileSelection&) = delete;
};

class EditorBrush final : public TileSelection
{
public:
  EditorBrush() :
    TileSelection(),
    m_tilegroup_id(-1),
    m_name()
  {
  }

public:
  int get_tilegroup_id() const { return m_tilegroup_id; }
  void set_tilegroup_id(int id) { m_tilegroup_id = id; }

  const std::string& get_name() const { return m_name; }
  void set_name(const std::string& name) { m_name = name; }

public:
  int m_tilegroup_id;
  std::string m_name;
};