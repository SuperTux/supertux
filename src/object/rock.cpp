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
#include "tile.hpp"

namespace {
  const std::string ROCK_SOUND = "sounds/brick.wav"; //TODO use own sound.
}

Rock::Rock(const Vector& pos, std::string spritename)
  : MovingSprite(pos, spritename)
{
  sound_manager->preload(ROCK_SOUND);
  on_ground = false;
  grabbed = false;
  set_group(COLGROUP_MOVING_STATIC);
}

Rock::Rock(const lisp::Lisp& reader)
  : MovingSprite(reader, "images/objects/rock/rock.sprite")
{
  sound_manager->preload(ROCK_SOUND);
  on_ground = false;
  grabbed = false;
  set_group(COLGROUP_MOVING_STATIC);
}

Rock::Rock(const lisp::Lisp& reader, std::string spritename)
  : MovingSprite(reader, spritename)
{
  sound_manager->preload(ROCK_SOUND);
  on_ground = false;
  grabbed = false;
  set_group(COLGROUP_MOVING_STATIC);
}

void
Rock::write(lisp::Writer& writer)
{
  writer.start_list("rock");

  writer.write("x", bbox.p1.x);
  writer.write("y", bbox.p1.y);

  writer.end_list("rock");
}

void
Rock::update(float elapsed_time)
{
  if( grabbed )
    return;

  if (on_ground) physic.set_velocity_x(0);

  movement = physic.get_movement(elapsed_time);
}

void
Rock::collision_solid(const CollisionHit& hit)
{
  if(grabbed) {
    return;
  }
  if(hit.top || hit.bottom)
    physic.set_velocity_y(0);
  if(hit.left || hit.right)
    physic.set_velocity_x(0);
  if(hit.crush)
    physic.set_velocity(0, 0);

  if(hit.bottom  && !on_ground && !grabbed) {
    sound_manager->play(ROCK_SOUND, get_pos());
    on_ground = true;
  }
}

HitResponse
Rock::collision(GameObject& other, const CollisionHit& hit)
{
  if(grabbed) {
    return PASSTHROUGH;
  }
  if(!on_ground) {
    if(hit.bottom && physic.get_velocity_y() > 200) {
      MovingObject* moving_object = dynamic_cast<MovingObject*> (&other);
      if(moving_object) {
        //Getting a rock on the head hurts. A lot.
        moving_object->collision_tile(Tile::HURTS);
      }
    }
    return FORCE_MOVE;
  }

  return FORCE_MOVE;
}

void
Rock::grab(MovingObject& , const Vector& pos, Direction)
{
  movement = pos - get_pos();
  last_movement = movement;
  set_group(COLGROUP_TOUCHABLE);
  on_ground = false;
  grabbed = true;
}

void
Rock::ungrab(MovingObject& , Direction dir)
{
  set_group(COLGROUP_MOVING_STATIC);
  on_ground = false;
  if(dir == UP) {
    physic.set_velocity(0, -500);
  } else if (last_movement.norm() > 1) {
    physic.set_velocity((dir == RIGHT) ? 200 : -200, -200);
  } else {
    physic.set_velocity(0, 0);
  }
  grabbed = false;
}

IMPLEMENT_FACTORY(Rock, "rock");
