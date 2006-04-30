//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "mrtree.hpp"
#include "poisonivy.hpp"

static const float WALKSPEED = 100;
static const float WALKSPEED_SMALL = 120;
static const float INVINCIBLE_TIME = 1;

static const float POISONIVY_WIDTH = 32;
static const float POISONIVY_HEIGHT = 32;
static const float POISONIVY_Y_OFFSET = 24;


MrTree::MrTree(const lisp::Lisp& reader)
  : mystate(STATE_BIG)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  sprite = sprite_manager->create("images/creatures/mr_tree/mr_tree.sprite");
  sprite->set_action(dir == LEFT ? "large-left" : "large-right");
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
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
  if (mystate == STATE_INVINCIBLE) {
    physic.set_velocity_x(0);
    sprite->set_action(dir == LEFT ? "small-left" : "small-right");
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
  if ((mystate == STATE_INVINCIBLE) && (invincible_timer.check())) {
    mystate = STATE_NORMAL;
    activate();
  }

  if (might_fall())
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
    mystate = STATE_INVINCIBLE;
    invincible_timer.start(INVINCIBLE_TIME);

    float old_x_offset = sprite->get_current_hitbox_x_offset();
    float old_y_offset = sprite->get_current_hitbox_y_offset();
    activate();

    // shrink bounding box and adjust sprite position to where the stump once was
    bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
    Vector pos = get_pos();
    pos.x += sprite->get_current_hitbox_x_offset() - old_x_offset;
    pos.y += sprite->get_current_hitbox_y_offset() - old_y_offset;
    set_pos(pos);

    sound_manager->play("sounds/mr_tree.ogg", get_pos());
    player.bounce(*this);

    Vector leaf1_pos = Vector(pos.x - POISONIVY_WIDTH - 1, pos.y - POISONIVY_Y_OFFSET);
    Rect leaf1_bbox = Rect(leaf1_pos.x, leaf1_pos.y, leaf1_pos.x + POISONIVY_WIDTH, leaf1_pos.y + POISONIVY_HEIGHT);
    if (Sector::current()->is_free_space(leaf1_bbox)) {
      PoisonIvy* leaf1 = new PoisonIvy(leaf1_bbox.p1.x, leaf1_bbox.p1.y, LEFT);
      Sector::current()->add_object(leaf1);
    }
    Vector leaf2_pos = Vector(pos.x + sprite->get_current_hitbox_width() + 1, pos.y - POISONIVY_Y_OFFSET);
    Rect leaf2_bbox = Rect(leaf2_pos.x, leaf2_pos.y, leaf2_pos.x + POISONIVY_WIDTH, leaf2_pos.y + POISONIVY_HEIGHT);
    if (Sector::current()->is_free_space(leaf2_bbox)) {
      PoisonIvy* leaf2 = new PoisonIvy(leaf2_bbox.p1.x, leaf2_bbox.p1.y, RIGHT);
      Sector::current()->add_object(leaf2);
    }

    return true;
  }

  // if we're still invincible, we ignore the hit
  if (mystate == STATE_INVINCIBLE) {
    sound_manager->play("sounds/mr_treehit.ogg", get_pos());
    player.bounce(*this);
    return true;
  }

  // if we're small, we die
  if (mystate == STATE_NORMAL) {
    sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
    bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
    kill_squished(player);
    return true;
  }

  //TODO: exception?
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
  
