//  $Id: growup.cpp 2458 2005-05-10 11:29:58Z matzebraun $
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

#include <math.h>
#include "powerup.hpp"
#include "resources.hpp"
#include "player.hpp"
#include "sprite/sprite_manager.hpp"
#include "audio/sound_manager.hpp"
#include "object_factory.hpp"
#include "sector.hpp"
#include "scripting/script_interpreter.hpp"

PowerUp::PowerUp(const lisp::Lisp& lisp)
{
  lisp.get("x", bbox.p1.x);
  lisp.get("y", bbox.p1.y);
  lisp.get("sprite", sprite_name);
  lisp.get("script", script);
  no_physics = false;
  lisp.get("disable-physics", no_physics);
  bbox.set_size(32, 32);   
  sprite = sprite_manager->create(sprite_name);
  physic.enable_gravity(true);

  set_group(COLGROUP_MOVING);
}

PowerUp::~PowerUp()
{
  delete sprite;
}

HitResponse
PowerUp::collision(GameObject& other, const CollisionHit& hit)
{
  if(other.get_flags() & FLAG_SOLID) {
    if(fabsf(hit.normal.y) > .5) { // roof or ground
      physic.set_velocity_y(0);
    } else { // bumped left or right
      printf("Normal: %f %f\n", hit.normal.x, hit.normal.y);
      printf("LRbounce, new speed. %f\n", physic.get_velocity_x());
      physic.set_velocity_x(-physic.get_velocity_x());
    }

    return CONTINUE;
  }
  
  Player* player = dynamic_cast<Player*>(&other);
  if(player == 0)
    return FORCE_MOVE;

  remove_me();

  if (script != "") {
    ScriptInterpreter::add_script_object(Sector::current(), "powerup-script",
        script);
    return ABORT_MOVE;
  }

  // some defaults if no script has been set
  if (sprite_name == "images/powerups/egg/egg.sprite") {
    player->set_bonus(GROWUP_BONUS, true);
    sound_manager->play("sounds/grow.wav");
  } else if (sprite_name == "images/powerups/fireflower/fireflower.sprite") {
    player->set_bonus(FIRE_BONUS, true);
    sound_manager->play("sounds/fire-flower.wav");
  } else if (sprite_name == "images/powerups/star/star.sprite") {
    player->make_invincible();
  } else if (sprite_name == "images/powerups/1up/1up.sprite") {
    player->get_status()->incLives();
  }
  return ABORT_MOVE;
}

void
PowerUp::update(float elapsed_time)
{
  if (!no_physics)
    movement = physic.get_movement(elapsed_time);
}

void
PowerUp::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

IMPLEMENT_FACTORY(PowerUp, "powerup");

