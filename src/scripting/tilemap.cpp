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

#define NOIMPL      log_fatal << __PRETTY_FUNCTION__ << " not implemented."

namespace Scripting {

TileMap::TileMap(::TileMap* tilemap)
  : tilemap(tilemap)
{ }

TileMap::~TileMap()
{ }

void TileMap::goto_node(int node_no)
{
  tilemap->goto_node(node_no);
}

void TileMap::start_moving()
{
  tilemap->start_moving();
}

void TileMap::stop_moving()
{
  tilemap->stop_moving();
}

void TileMap::fade(float alpha, float seconds)
{
  tilemap->fade(alpha, seconds);
}

void TileMap::set_alpha(float alpha)
{
  tilemap->set_alpha(alpha);
}

float TileMap::get_alpha()
{
  return tilemap->get_alpha();
}

}

/* EOF */
