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

#include "mrbomb.hpp"
#include "bomb.hpp"
#include "object/explosion.hpp"
#include "sprite/sprite_manager.hpp"
#include "audio/sound_manager.hpp"
#include "lisp/writer.hpp"
#include "object_factory.hpp"
#include "sector.hpp"
#include "lisp/lisp.hpp"
#include "sprite/sprite.hpp"

MrBomb::MrBomb(const lisp::Lisp& reader)
  : WalkingBadguy(reader, "images/creatures/mr_bomb/mr_bomb.sprite", "left", "right")
{
  walk_speed = 80;
  max_drop_height = 16;
  grabbed = false;

  //Prevent stutter when Tux jumps on Mr Bomb
  sound_manager->preload("sounds/explosion.wav");

  //Check if we need another sprite
  if( !reader.get( "sprite", sprite_name ) ){
    return;
  }
  if( sprite_name == "" ){
    sprite_name = "images/creatures/mr_bomb/mr_bomb.sprite";
    return;
  }
  //Replace sprite
  sprite = sprite_manager->create( sprite_name );
}

/* MrBomb created by a dispenser always gets default sprite atm.*/
MrBomb::MrBomb(const Vector& pos, Direction d)
  : WalkingBadguy(pos, d, "images/creatures/mr_bomb/mr_bomb.sprite", "left", "right")
{
  walk_speed = 80;
  max_drop_height = 16;
  grabbed = false;
  sound_manager->preload("sounds/explosion.wav");
}

void
MrBomb::write(lisp::Writer& writer)
{
  writer.start_list("mrbomb");
  WalkingBadguy::write(writer);
  writer.end_list("mrbomb");
}

HitResponse
MrBomb::collision(GameObject& object, const CollisionHit& hit)
{
  if(grabbed)
    return FORCE_MOVE;
  return WalkingBadguy::collision(object, hit);
}

HitResponse
MrBomb::collision_player(Player& player, const CollisionHit& hit)
{
  if(grabbed)
    return FORCE_MOVE;
  return WalkingBadguy::collision_player(player, hit);
}

bool
MrBomb::collision_squished(GameObject& object)
{
  if(is_valid()) {
    remove_me();
    Sector::current()->add_object(new Bomb(get_pos(), dir, sprite_name ));
  }
  kill_squished(object);
  return true;
}

void
MrBomb::active_update(float elapsed_time)
{
  if(grabbed)
    return;
  WalkingBadguy::active_update(elapsed_time);
}

void
MrBomb::kill_fall()
{
  if(is_valid()) {
    remove_me();
    Explosion* explosion = new Explosion(get_bbox().get_middle());
    Sector::current()->add_object(explosion);
  }

  run_dead_script();
}

void
MrBomb::grab(MovingObject&, const Vector& pos, Direction dir)
{
  assert(frozen);
  movement = pos - get_pos();
  this->dir = dir;
  sprite->set_action(dir == LEFT ? "iced-left" : "iced-right");
  set_colgroup_active(COLGROUP_DISABLED);
  grabbed = true;
}

void
MrBomb::ungrab(MovingObject& , Direction dir)
{
  this->dir = dir;
  set_colgroup_active(COLGROUP_MOVING);
  grabbed = false;
}

void
MrBomb::freeze()
{
  WalkingBadguy::freeze();
  sprite->set_action(dir == LEFT ? "iced-left" : "iced-right");
}

bool
MrBomb::is_freezable() const
{
  return true;
}

bool
MrBomb::is_portable() const
{
  return frozen;
}

IMPLEMENT_FACTORY(MrBomb, "mrbomb")
