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

#include "rock.hpp"
#include "lisp/writer.hpp"
#include "object_factory.hpp"
#include "audio/sound_manager.hpp"

namespace {
  const std::string ROCK_SOUND = "sounds/brick.wav"; //TODO use own sound.
}

Rock::Rock(const lisp::Lisp& reader)
  : MovingSprite(reader, "images/objects/rock/rock.sprite")
{
  sound_manager->preload( ROCK_SOUND );
  on_ground = false;
  grabbed = false;
}

void
Rock::write(lisp::Writer& writer)
{
  writer.start_list("rock");

  writer.write_float("x", bbox.p1.x);
  writer.write_float("y", bbox.p1.y);

  writer.end_list("rock");
}

void
Rock::update(float elapsed_time)
{
  if( grabbed )
    return;

  movement = physic.get_movement(elapsed_time);
}

void
Rock::collision_solid(const CollisionHit& hit)
{
  if( hit.top || hit.bottom )
    physic.set_velocity_y( 0 );
  if( hit.left || hit.right )
    physic.set_velocity_x( 0 );
  if( hit.crush )
    physic.set_velocity(0, 0);

  if( hit.bottom  && !on_ground ){
    sound_manager->play( ROCK_SOUND, get_pos() );
    on_ground = true;
  }
}

HitResponse
Rock::collision(GameObject& other, const CollisionHit& hit)
{
  if( !on_ground ){
      return FORCE_MOVE;
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
Rock::grab(MovingObject& , const Vector& pos, Direction)
{
  movement = pos - get_pos();
  set_group( COLGROUP_DISABLED );
  on_ground = true;
  grabbed = true;
}

void
Rock::ungrab(MovingObject& , Direction ){
  set_group( COLGROUP_MOVING );
  on_ground = false;
  physic.set_velocity(0, 0);
  grabbed = false;
}

IMPLEMENT_FACTORY(Rock, "rock");
