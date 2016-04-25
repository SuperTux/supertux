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

#include "editor/tile_selection.hpp"

TileSelection::TileSelection() :
  tiles(),
  width(1),
  height(1)
{
  tiles.push_back(1);
}

uint32_t TileSelection::pos(int x, int y) const {
  x = x % width;
  y = y % height;
  if (x < 0) {
    x += width;
  }
  if (y < 0) {
    y += height;
  }

  return tiles[x + y*width];
}

void TileSelection::set_tile(uint32_t tile) {
  tiles.clear();
  width = 1;
  height = 1;
  tiles.push_back(tile);
}

bool TileSelection::empty() const {
  for (auto it = tiles.begin(); it != tiles.end(); ++it) {
    if (*it != 0) {
      return false;
    }
  }
  return true;
}
/* EOF */
