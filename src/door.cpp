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

#include "door.h"
#include "lispreader.h"
#include "lispwriter.h"
#include "gameloop.h"
#include "resources.h"
#include "sprite.h"
#include "sprite_manager.h"
#include "screen/drawing_context.h"

Door::Door(LispReader& reader)
{
  reader.read_float("x", area.x);
  reader.read_float("y", area.y);
  area.width = 32;
  area.height = 64;

  reader.read_string("sector", target_sector);
  reader.read_string("spawnpoint", target_spawnpoint);

  sprite = sprite_manager->load("door");
}

void
Door::write(LispWriter& writer)
{
  writer.start_list("door");

  writer.write_float("x", area.x);
  writer.write_float("y", area.y);
  writer.write_float("width", area.width);
  writer.write_float("height", area.height);
  
  writer.write_string("sector", target_sector);
  writer.write_string("spawnpoint", target_spawnpoint);

  writer.end_list("door");
}

Door::~Door()
{
}

void
Door::action(float )
{
}

void
Door::draw(DrawingContext& context)
{
  sprite->draw(context, Vector(area.x, area.y), LAYER_TILES);
}

void
Door::interaction(InteractionType type)
{
  if(type == INTERACTION_ACTIVATE) {
    GameSession::current()->respawn(target_sector, target_spawnpoint);
  }
}

