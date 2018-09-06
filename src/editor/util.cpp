//  SuperTux
//  Copyright (C) 2016 Ingo Ruhnke <grumbel@gmail.com>
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

#include "editor/util.hpp"

#include "supertux/tile.hpp"
#include "supertux/tile_set.hpp"
#include "supertux/resources.hpp"
#include "video/canvas.hpp"
#include "video/renderer.hpp"

void
draw_tile(Canvas& canvas, const TileSet& tileset, uint32_t id, const Vector& pos,
          int z_pos, const Color& color)
{
  const Tile& tile = tileset.get(id);
  tile.draw(canvas, pos, z_pos, color);
}

/* EOF */
