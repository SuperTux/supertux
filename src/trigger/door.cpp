//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include <config.h>

#include "door.h"
#include "utils/lispreader.h"
#include "utils/lispwriter.h"
#include "gameloop.h"
#include "resources.h"
#include "special/sprite.h"
#include "special/sprite_manager.h"
#include "video/drawing_context.h"
#include "app/globals.h"

using namespace SuperTux;

Door::Door(LispReader& reader)
{
  reader.read_float("x", bbox.p1.x);
  reader.read_float("y", bbox.p1.y);
  bbox.set_size(32, 64);

  reader.read_string("sector", target_sector);
  reader.read_string("spawnpoint", target_spawnpoint);

  sprite = sprite_manager->create("door");
}

Door::Door(int x, int y, std::string sector, std::string spawnpoint)
{
  bbox.set_pos(Vector(x, y));
  bbox.set_size(32, 64);
  target_sector = sector;
  target_spawnpoint = spawnpoint;

  sprite = sprite_manager->create("door");
}

Door::~Door()
{
  delete sprite;
}

void
Door::write(LispWriter& writer)
{
  writer.start_list("door");

  writer.write_float("x", bbox.p1.x);
  writer.write_float("y", bbox.p1.y);
  writer.write_float("width", bbox.get_width());
  writer.write_float("height", bbox.get_height());
  
  writer.write_string("sector", target_sector);
  writer.write_string("spawnpoint", target_spawnpoint);

  writer.end_list("door");
}

void
Door::action(float )
{
  //Check if door animation is complete
  if (!sprite->check_animation()) {
    GameSession::current()->respawn(target_sector, target_spawnpoint);
  }
}

void
Door::draw(DrawingContext& context)
{
  sprite->draw(context, bbox.p1, LAYER_TILES);
}

void
Door::event(Player& , EventType type)
{
  if(type == EVENT_ACTIVATE) {
    sprite->set_action("open", 1);
  }
}

