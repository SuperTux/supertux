//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <config.h>

#include <math.h>
#include <stdexcept>
#include <stdexcept>

#include "audio/sound_manager.hpp"
#include "object/player.hpp"
#include "object/powerup.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/resources.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"

PowerUp::PowerUp(const lisp::Lisp& lisp)
  : MovingSprite(lisp, LAYER_OBJECTS, COLGROUP_MOVING)
{
  lisp.get("script", script);
  no_physics = false;
  lisp.get("disable-physics", no_physics);
  physic.enable_gravity(true);
  sound_manager->preload("sounds/grow.ogg");
  sound_manager->preload("sounds/fire-flower.wav");
}

void
PowerUp::collision_solid(const CollisionHit& hit)
{
  if(hit.bottom) {
    physic.set_velocity_y(0);
  }
  if(hit.right || hit.left) {
    physic.set_velocity_x(-physic.get_velocity_x());
  }
}

HitResponse
PowerUp::collision(GameObject& other, const CollisionHit&)
{
  Player* player = dynamic_cast<Player*>(&other);
  if(player == 0)
    return FORCE_MOVE;

  if (script != "") {
    std::istringstream stream(script);
    Sector::current()->run_script(stream, "powerup-script");
    remove_me();
    return ABORT_MOVE;
  }

  // some defaults if no script has been set
  if (sprite_name == "images/powerups/egg/egg.sprite") {
    if(!player->add_bonus(GROWUP_BONUS, true))
      return FORCE_MOVE;
    sound_manager->play("sounds/grow.ogg");
  } else if (sprite_name == "images/powerups/fireflower/fireflower.sprite") {
    if(!player->add_bonus(FIRE_BONUS, true))
      return FORCE_MOVE;
    sound_manager->play("sounds/fire-flower.wav");
  } else if (sprite_name == "images/powerups/star/star.sprite") {
    player->make_invincible();
  } else if (sprite_name == "images/powerups/1up/1up.sprite") {
    player->get_status()->add_coins(100);
  }

  remove_me();
  return ABORT_MOVE;
}

void
PowerUp::update(float elapsed_time)
{
  if (!no_physics)
    movement = physic.get_movement(elapsed_time);
}

IMPLEMENT_FACTORY(PowerUp, "powerup");
