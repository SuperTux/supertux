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

/* Trampoline will accelerate player to to VY_INITIAL, if 
 * he jumps on it to VY_MIN. */
namespace {
  const std::string TRAMPOLINE_SOUND = "sounds/trampoline.wav";
  const float VY_MIN = -900; //negative, upwards
  const float VY_INITIAL = -500;
}

Trampoline::Trampoline(const lisp::Lisp& lisp)
	: MovingSprite(lisp, "images/objects/trampoline/trampoline.sprite" )
{
  sound_manager->preload( TRAMPOLINE_SOUND );
  flags |= FLAG_PORTABLE;
  flags |= FLAG_SOLID;
  physic.set_velocity(0, 0);
  physic.enable_gravity(true);
  on_ground = false;

  bool portable = true;
  //Check if this trampoline is not portable
  if( lisp.get( "portable", portable ) ){
    if( !portable ){
        flags ^= FLAG_PORTABLE;
        //we need another sprite
        sprite_name = "images/objects/trampoline/trampoline_fix.sprite";
        sprite = sprite_manager->create( sprite_name );
        sprite->set_action("normal");
    }
  }
}

void
Trampoline::update( float elapsed_time ){
    if( !on_ground ){
        movement = physic.get_movement(elapsed_time);
    }
    if(sprite->animation_done()) {
      sprite->set_action("normal");
    }
}

HitResponse
Trampoline::collision(GameObject& other, const CollisionHit& hit )
{
  //Tramponine has to be on ground to work.
  if( !on_ground ){
      return FORCE_MOVE;
  }
  Player* player = dynamic_cast<Player*> (&other);
  if ( player ) {
    float vy = player->physic.get_velocity_y();
    //player is falling down on trampoline
    if(hit.top && vy > 0) {
      if(player->get_controller()->hold(Controller::JUMP)) { 
        vy = VY_MIN;
      } else {
        vy = VY_INITIAL;
      }
      player->physic.set_velocity_y( vy );
      sound_manager->play( TRAMPOLINE_SOUND );
      sprite->set_action("swinging", 1);
      return FORCE_MOVE;
    }
  }
  //Fake being solid for moving_object. 
  MovingObject* moving_object = dynamic_cast<MovingObject*> (&other);
  if( moving_object ){
      if( hit.top ){
        float inside = moving_object->get_bbox().get_bottom() - get_bbox().get_top();
        if( inside > 0 ){
          Vector pos = moving_object->get_pos();
          pos.y -= inside; 
          moving_object->set_pos( pos );    
        }
      }
      CollisionHit hit_other = hit;
      std::swap(hit_other.left, hit_other.right);
      std::swap(hit_other.top, hit_other.bottom);
      moving_object->collision_solid( hit_other );
  }
  return FORCE_MOVE;
}

void 
Trampoline::collision_solid( const CollisionHit& hit ){
  if( hit.bottom ){
     on_ground = true;
  }
} 

void
Trampoline::grab( MovingObject&, const Vector& pos, Direction ){
  movement = pos - get_pos();
  set_group( COLGROUP_DISABLED );
  on_ground = true;
  sprite->set_animation_loops( 0 );
}

void
Trampoline::ungrab(MovingObject& , Direction ){
  set_group( COLGROUP_MOVING );
  on_ground = false;
  physic.set_velocity(0, 0);
}


IMPLEMENT_FACTORY(Trampoline, "trampoline");
