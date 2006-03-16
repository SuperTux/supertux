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

#include "mrtree.hpp"

static const float WALKSPEED = 100;
static const float WALKSPEED_SMALL = 120;
static const float INVINCIBLE_TIME = 1;

MrTree::MrTree(const lisp::Lisp& reader)
  : mystate(STATE_BIG)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  stay_on_platform = false;
  reader.get("stay-on-platform", stay_on_platform);
  bbox.set_size(84.8, 84.8);
  sprite = sprite_manager->create("images/creatures/mr_tree/mr_tree.sprite");
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
  if (mystate == STATE_BIG) {
    physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
    sprite->set_action(dir == LEFT ? "large-left" : "large-right");
    return;
  }
  if (mystate == STATE_NORMAL) {
    physic.set_velocity_x(dir == LEFT ? -WALKSPEED_SMALL : WALKSPEED_SMALL);
    sprite->set_action(dir == LEFT ? "small-left" : "small-right");
    return;
  }
}

void
MrTree::active_update(float elapsed_time)
{
  if (stay_on_platform && may_fall_off_platform())
  {
    dir = (dir == LEFT ? RIGHT : LEFT);
    activate();
  }

  BadGuy::active_update(elapsed_time);
}

bool
MrTree::collision_squished(Player& player)
{
  // if we're big, we shrink
  if(mystate == STATE_BIG) {
    mystate = STATE_NORMAL;
    activate();

    // shrink bounding box and adjust sprite position to where the stump once was
    bbox.set_size(42, 62);
    Vector pos = get_pos();
    pos.x += 20;
    pos.y += 23;
    set_pos(pos);

    sound_manager->play("sounds/mr_tree.ogg", get_pos());
    player.bounce(*this);

    invincible_timer.start(INVINCIBLE_TIME);

    return true;
  }

  // if we're small, but still invincible, we ignore the hit
  if (!invincible_timer.check()) {
    sound_manager->play("sounds/mr_treehit.ogg", get_pos());
    player.bounce(*this);
    return true;
  }

  // if we're small and no longer invincible, we die
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  bbox.set_size(42, 42);
  kill_squished(player);
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
  
