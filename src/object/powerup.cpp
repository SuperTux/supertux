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
#include "powerup.h"
#include "resources.h"
#include "player.h"
#include "sprite/sprite_manager.h"
#include "audio/sound_manager.h"
#include "object_factory.h"
#include "sector.h"
#include "scripting/script_interpreter.h"

PowerUp::PowerUp(const lisp::Lisp& lisp)
{
  std::string sprite_name;
  lisp.get("x", bbox.p1.x);
  lisp.get("y", bbox.p1.y);
  lisp.get("sprite", sprite_name);
  lisp.get("script", script);
  bbox.set_size(32, 32);   
  sprite = sprite_manager->create(sprite_name);
  physic.enable_gravity(true);
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
  if (sprite->get_name() == "egg") {
    player->set_bonus(GROWUP_BONUS, true);
    sound_manager->play("grow");
  } else if (sprite->get_name() == "fireflower") {
    player->set_bonus(FIRE_BONUS, true);
    sound_manager->play("fire-flower");
  } else if (sprite->get_name() == "star") {
    player->make_invincible();
  } else if (sprite->get_name() == "1up") {
    player->get_status()->incLives();
  }
  return ABORT_MOVE;
}

void
PowerUp::update(float elapsed_time)
{
  movement = physic.get_movement(elapsed_time);
}

void
PowerUp::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

IMPLEMENT_FACTORY(PowerUp, "powerup");

