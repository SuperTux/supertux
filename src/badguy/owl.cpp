//  SuperTux
//  Copyright (C) 2008 Wolfgang Becker <uafr@gmx.de>
//  Copyright (C) 2010 Florian Forster <supertux at octo.it>
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

#include "badguy/owl.hpp"

#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "object/rock.hpp"
#include "util/reader.hpp"
#include "util/log.hpp"

#define FLYING_SPEED 120.0

Owl::Owl(const Reader& reader) :
  BadGuy(reader, "images/creatures/owl/owl.sprite"),
  carried_obj_name("rock"),
  carried_object(NULL)
{
  reader.get("carry", carried_obj_name);
  set_action (dir == LEFT ? "left" : "right", /* loops = */ -1);
}

Owl::Owl(const Vector& pos, Direction d) :
  BadGuy(pos, d, "images/creatures/owl/owl.sprite"),
  carried_obj_name("rock"),
  carried_object(NULL)
{
  set_action (dir == LEFT ? "left" : "right", /* loops = */ -1);
}

void
Owl::initialize()
{
  GameObject *game_object;

  physic.set_velocity_x(dir == LEFT ? -FLYING_SPEED : FLYING_SPEED);
  physic.enable_gravity(false);
  sprite->set_action(dir == LEFT ? "left" : "right");

  game_object = ObjectFactory::instance().create(carried_obj_name, get_pos(), dir);
  if (game_object == NULL) {
    log_fatal << "Creating \"" << carried_obj_name << "\" object failed." << std::endl;
    return;
  }

  carried_object = dynamic_cast<Portable *> (game_object);
  if (carried_object == NULL) {
    log_warning << "Object is not portable: " << carried_obj_name << std::endl;
    delete game_object;
    return;
  }

  Sector::current ()->add_object (game_object);
} /* void initialize */

void
Owl::active_update (float elapsed_time)
{
  BadGuy::active_update (elapsed_time);

  if (carried_object != NULL) {
    Vector obj_pos = get_pos ();
    
    obj_pos.y += bbox.get_height ();
    carried_object->grab (*this, obj_pos, dir);
  }
}

bool
Owl::collision_squished(GameObject&)
{
  if (carried_object != NULL) {
    carried_object->ungrab (*this, dir);
    carried_object = NULL;
  }
  kill_fall ();
  return true;
}

void
Owl::collision_solid(const CollisionHit& hit)
{
  if(hit.top || hit.bottom) {
    physic.set_velocity_y(0);
  } else if(hit.left || hit.right) {
    if (dir == LEFT) {
      set_action ("right", /* loops = */ -1);
      physic.set_velocity_x (FLYING_SPEED);
    }
    else {
      set_action ("left", /* loops = */ -1);
      physic.set_velocity_x (-FLYING_SPEED);
    }
  }
} /* void Owl::collision_solid */

HitResponse
Owl::collision_player(Player& player, const CollisionHit& hit)
{
  //Hack to tell if we should die
  HitResponse response = BadGuy::collision_player(player, hit);
  if(response == FORCE_MOVE) {
    if (carried_object != NULL) {
      carried_object->ungrab (*this, dir);
      carried_object = NULL;
    }
    kill_fall ();
  }

  return ABORT_MOVE;
}

/* vim: set sw=2 sts=2 et fdm=marker : */
/* EOF */
