//  SuperTux
//  Copyright (C) 2026 SuperTux contributors
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

#include <cstdint>

namespace editor
{

/** Decide whether a tile-replacement (repeat/pattern fill) operation should be
 *  skipped before touching the tilemap.
 *
 *  Returns true when the replacement must NOT run, in order to avoid:
 *   - a division by zero when the selection has zero width or height
 *     (issue #3810), and
 *   - an unnecessary no-op rewrite when the selection is a single tile that is
 *     identical to the tile being replaced.
 *
 *  This is a pure function with no engine/SDL dependencies so it can be unit
 *  tested headlessly.
 *
 *  @param tiles_width      width of the selected tile pattern (in tiles)
 *  @param tiles_height     height of the selected tile pattern (in tiles)
 *  @param replace_tile     id of the tile currently under the cursor
 *  @param first_selected   id of the top-left tile of the selected pattern
 */
inline bool
should_skip_tile_replacement(int tiles_width, int tiles_height,
                             uint32_t replace_tile, uint32_t first_selected)
{
  if (tiles_width == 0 || tiles_height == 0)
    return true;

  if (tiles_width == 1 && tiles_height == 1 && replace_tile == first_selected)
    return true;

  return false;
}

} // namespace editor
