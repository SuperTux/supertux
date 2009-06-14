//  $Id$
//
//  SuperTux - Trampoline
//  Copyright (C) 2006 Wolfgang Becker <uafr@gmx.de>
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

#include "trampoline.hpp"
#include "object_factory.hpp"
#include "player.hpp"
#include "audio/sound_manager.hpp"
#include "sprite/sprite_manager.hpp"
#include "sprite/sprite.hpp"
#include "badguy/walking_badguy.hpp"
#include "control/controller.hpp"

/* Trampoline will accelerate player to to VY_INITIAL, if
 * he jumps on it to VY_MIN. */
namespace {
  const std::string TRAMPOLINE_SOUND = "sounds/trampoline.wav";
  const float VY_MIN = -900; //negative, upwards
  const float VY_INITIAL = -500;
}

Trampoline::Trampoline(const lisp::Lisp& lisp)
        : Rock(lisp, "images/objects/trampoline/trampoline.sprite")
{
  sound_manager->preload(TRAMPOLINE_SOUND);

  portable = true;
  //Check if this trampoline is not portable
  if(lisp.get("portable", portable)) {
    if(!portable) {
        //we need another sprite
        sprite_name = "images/objects/trampoline/trampoline_fix.sprite";
        sprite = sprite_manager->create(sprite_name);
        sprite->set_action("normal");
    }
  }
}

void
Trampoline::update(float elapsed_time)
{
  if(sprite->animation_done()) {
    sprite->set_action("normal");
  }

  Rock::update(elapsed_time);
}

HitResponse
Trampoline::collision(GameObject& other, const CollisionHit& hit)
{

  //Tramponine has to be on ground to work.
  if(on_ground) {
    Player* player = dynamic_cast<Player*> (&other);
    //Trampoline works for player
    if(player) {
      float vy = player->physic.get_velocity_y();
      //player is falling down on trampoline
      if(hit.top && vy >= 0) {
        if(player->get_controller()->hold(Controller::JUMP)) {
          vy = VY_MIN;
        } else {
          vy = VY_INITIAL;
        }
        player->physic.set_velocity_y(vy);
        sound_manager->play(TRAMPOLINE_SOUND);
        sprite->set_action("swinging", 1);
        return FORCE_MOVE;
      }
    }
    WalkingBadguy* walking_badguy = dynamic_cast<WalkingBadguy*> (&other);
    //Trampoline also works for WalkingBadguy
    if(walking_badguy) {
      float vy = walking_badguy->get_velocity_y();
      //walking_badguy is falling down on trampoline
      if(hit.top && vy >= 0) {
        vy = VY_INITIAL;
        walking_badguy->set_velocity_y(vy);
        sound_manager->play(TRAMPOLINE_SOUND);
        sprite->set_action("swinging", 1);
        return FORCE_MOVE;
      }
    }
  }

  return Rock::collision(other, hit);
}

void
Trampoline::collision_solid(const CollisionHit& hit) {
  Rock::collision_solid(hit);
}

void
Trampoline::grab(MovingObject& object, const Vector& pos, Direction dir) {
  sprite->set_animation_loops(0);
  Rock::grab(object, pos, dir);
}

void
Trampoline::ungrab(MovingObject& object, Direction dir) {
  Rock::ungrab(object, dir);
}

bool
Trampoline::is_portable() const
{
  return Rock::is_portable() && portable;
}

IMPLEMENT_FACTORY(Trampoline, "trampoline");
