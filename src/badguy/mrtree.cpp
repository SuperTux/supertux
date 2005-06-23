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

#include <config.h>

#include "mrtree.h"

static const float WALKSPEED = 50;
static const float WALKSPEED_SMALL = 30;

MrTree::MrTree(const lisp::Lisp& reader)
  : mystate(STATE_BIG)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(84.8, 95.8);
  sprite = sprite_manager->create("mrtree");
}

void
MrTree::write(lisp::Writer& writer)
{
  writer.start_list("mrtree");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("mrtree");
}

void
MrTree::activate()
{
  if(mystate == STATE_BIG) {
    physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
    sprite->set_action(dir == LEFT ? "left" : "right");
  } else {
    physic.set_velocity_x(dir == LEFT ? -WALKSPEED_SMALL : WALKSPEED_SMALL);
    sprite->set_action(dir == LEFT ? "small-left" : "small-right");
  }
}

bool
MrTree::collision_squished(Player& player)
{
  if(mystate == STATE_BIG) {
    mystate = STATE_NORMAL;
    activate();

    sound_manager->play("squish", get_pos());
    player.bounce(*this);
  } else {
    sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
    kill_squished(player);
  }
  
  return true;
}

HitResponse
MrTree::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5) {
    physic.set_velocity_y(0);
  } else {
    dir = dir == LEFT ? RIGHT : LEFT;
    activate();
  }

  return CONTINUE;
}

HitResponse
MrTree::collision_badguy(BadGuy& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.x) > .8) { // left or right hit
    dir = dir == LEFT ? RIGHT : LEFT;
    activate();
  }

  return CONTINUE;
}

IMPLEMENT_FACTORY(MrTree, "mrtree")
  
