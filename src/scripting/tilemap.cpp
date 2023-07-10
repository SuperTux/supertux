//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "object/tilemap.hpp"
#include "scripting/tilemap.hpp"

namespace scripting {

void
TileMap::goto_node(int node_no)
{
  SCRIPT_GUARD_VOID;
  object.goto_node(node_no);
}

void
TileMap::set_node(int node_no)
{
  SCRIPT_GUARD_VOID;
  object.jump_to_node(node_no);
}

void
TileMap::start_moving()
{
  SCRIPT_GUARD_VOID;
  object.start_moving();
}

void
TileMap::stop_moving()
{
  SCRIPT_GUARD_VOID;
  object.stop_moving();
}

int
TileMap::get_tile_id(int x, int y) const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_tile_id(x, y);
}

int
TileMap::get_tile_id_at(float x, float y) const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_tile_id_at( Vector(x, y) );
}

void
TileMap::change(int x, int y, int newtile)
{
  SCRIPT_GUARD_VOID;
  object.change(x, y, newtile);
}

void
TileMap::change_at(float x, float y, int newtile)
{
  SCRIPT_GUARD_VOID;
  object.change_at(Vector(x, y), newtile);
}

void
TileMap::fade(float alpha, float time)
{
  SCRIPT_GUARD_VOID;
  object.fade(alpha, time);
}

void
TileMap::tint_fade(float time, float red, float green, float blue, float alpha)
{
  SCRIPT_GUARD_VOID;
  object.tint_fade(Color(red, green, blue, alpha), time);
}

void
TileMap::set_alpha(float alpha)
{
  SCRIPT_GUARD_VOID;
  object.set_alpha(alpha);
}

float
TileMap::get_alpha() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_alpha();
}

void
TileMap::set_solid(bool solid)
{
  SCRIPT_GUARD_VOID;
  object.set_solid(solid);
}

} // namespace scripting

/* EOF */
