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
#include "globals.h"

/** data images */
Sprite* door;
Surface* door_opening[DOOR_OPENING_FRAMES];

Door::Door(LispReader& reader)
{
  reader.read_float("x", area.x);
  reader.read_float("y", area.y);
  area.width = 32;
  area.height = 64;

  reader.read_string("sector", target_sector);
  reader.read_string("spawnpoint", target_spawnpoint);

  animation_timer.init(true);
  door_activated = false;

  animation_timer.init(true);
}

Door::Door(int x, int y)
{
area.x = x;
area.y = y;
area.width = 32;
area.height = 64;

animation_timer.init(true);
door_activated = false;

animation_timer.init(true);
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
  if(animation_timer.check())
    context.draw_surface(door_opening[(animation_timer.get_gone() * DOOR_OPENING_FRAMES) /
          DOOR_OPENING_TIME], Vector(area.x, area.y - (door_opening[0]->h/2)), LAYER_TILES);
  else
    door->draw(context, Vector(area.x, area.y), LAYER_TILES);
  
  //Check if door animation is complete
  //TODO: Move this out of the "draw" method as this is extremely dirty :)  
  if ((!animation_timer.check()) && (door_activated)) {    
    door_activated = false;
    GameSession::current()->respawn(target_sector, target_spawnpoint);
  }
}

void
Door::interaction(InteractionType type)
{
  //Animate the door on activation
  //TODO: Resetting the animation doesn't work correctly
  //      Tux and badguys should stop moving while the door is opening
  if(type == INTERACTION_ACTIVATE) {
    animation_timer.start(DOOR_OPENING_TIME);
    door_activated = true;
  }
}

