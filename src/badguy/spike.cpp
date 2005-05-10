//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#include "spike.h"

Spike::Spike(const Vector& pos, Direction dir)
{
  sprite = sprite_manager->create("spike");
  start_position = pos;
  bbox.set_pos(Vector(0, 0));
  bbox.set_size(32, 32);
  set_direction(dir);
}

Spike::Spike(const lisp::Lisp& reader)
{
  sprite = sprite_manager->create("spike");
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(32, 32);
  int idir = 0;
  reader.get("direction", idir);
  set_direction((Direction) idir);
}

void
Spike::set_direction(Direction dir)
{
  spikedir = dir;
  switch(spikedir) {
    case NORTH:
      sprite->set_action("north");
      break;
    case SOUTH:
      sprite->set_action("south");
      break;
    case WEST:
      sprite->set_action("west");
      break;
    case EAST:
      sprite->set_action("east");
      break;
    default:
      break;
  }
}

void
Spike::write(lisp::Writer& writer)
{
  writer.start_list("spike");
  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);
  writer.write_int("direction", spikedir);
  writer.end_list("spike");
}

void
Spike::kill_fall()
{
  // you can't kill a spike
}

void
Spike::active_update(float )
{
}

IMPLEMENT_FACTORY(Spike, "spike")
