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

#include "st_assert.hpp"
#include "editor/tile_replacement.hpp"

using editor::should_skip_tile_replacement;

int main(void)
{
  // --- Issue #3810: division-by-zero guard -------------------------------
  // A zero-sized selection (aligned the cursor exactly between two tiles)
  // previously caused a floating point exception when the fill pattern was
  // indexed with a zero width/height. The replacement must be skipped.
  ST_ASSERT("zero width selection is skipped",
            should_skip_tile_replacement(0, 4, 1, 2) == true);
  ST_ASSERT("zero height selection is skipped",
            should_skip_tile_replacement(4, 0, 1, 2) == true);
  ST_ASSERT("both zero selection is skipped",
            should_skip_tile_replacement(0, 0, 1, 2) == true);

  // --- Non-degenerate selections must NOT be skipped ---------------------
  ST_ASSERT("normal 1x1 differing tile is not skipped",
            should_skip_tile_replacement(1, 1, 1, 2) == false);
  ST_ASSERT("2x2 selection is not skipped",
            should_skip_tile_replacement(2, 2, 1, 2) == false);
  ST_ASSERT("1x3 selection is not skipped",
            should_skip_tile_replacement(1, 3, 1, 2) == false);

  // --- Redundant no-op guard ---------------------------------------------
  // Replacing a tile by the exact same single tile is a no-op and must skip.
  ST_ASSERT("1x1 identical tile is skipped (no-op)",
            should_skip_tile_replacement(1, 1, 7, 7) == true);
  // But a 1x1 selection with a DIFFERENT tile must run.
  ST_ASSERT("1x1 different tile is not skipped",
            should_skip_tile_replacement(1, 1, 7, 9) == false);

  // --- Large selections are unaffected by the guard ----------------------
  ST_ASSERT("big selection is not skipped",
            should_skip_tile_replacement(64, 64, 5, 5) == false);

  return 0;
}
