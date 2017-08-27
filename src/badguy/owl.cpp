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

#include "editor/editor.hpp"
#include "audio/sound_manager.hpp"
#include "object/anchor_point.hpp"
#include "object/player.hpp"
#include "object/rock.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "util/log.hpp"

#define FLYING_SPEED 120.0
#define ACTIVATION_DISTANCE 128.0

Owl::Owl(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/owl/owl.sprite", LAYER_OBJECTS + 1),
  carried_obj_name(),
  carried_object(NULL)
{
  reader.get("carry", carried_obj_name, "skydive");
  set_action (dir == LEFT ? "left" : "right", /* loops = */ -1);
}

Owl::Owl(const Vector& pos, Direction d) :
  BadGuy(pos, d, "images/creatures/owl/owl.sprite", LAYER_OBJECTS + 1),
  carried_obj_name("skydive"),
  carried_object(NULL)
{
  set_action (dir == LEFT ? "left" : "right", /* loops = */ -1);
}

void
Owl::save(Writer& writer) {
  BadGuy::save(writer);
  writer.write("carry", carried_obj_name);
}

void
Owl::initialize()
{
  physic.set_velocity_x(dir == LEFT ? -FLYING_SPEED : FLYING_SPEED);
  physic.enable_gravity(false);
  sprite->set_action(dir == LEFT ? "left" : "right");

  // If we add the carried object to the sector while we're editing 
  // a level with the editor, it gets written to the level file,
  // resulting in two carried objects. Returning early is much better.
  if(Editor::is_active())
  {
    return;
  }

  auto game_object = ObjectFactory::instance().create(carried_obj_name, get_pos(), dir);
  if (game_object == NULL)
  {
    log_fatal << "Creating \"" << carried_obj_name << "\" object failed." << std::endl;
  }
  else
  {
    carried_object = dynamic_cast<Portable*>(game_object.get());
    if (carried_object == NULL)
    {
      log_warning << "Object is not portable: " << carried_obj_name << std::endl;
    }
    else
    {
      Sector::current()->add_object(game_object);
    }
  }
}

bool
Owl::is_above_player() const
{
  auto player = Sector::current()->get_nearest_player (bbox);
  if (!player)
    return false;

  /* Let go of carried objects a short while *before* Tux is below us. This
   * makes it more likely that we'll hit him. */
  float x_offset = (dir == LEFT) ? ACTIVATION_DISTANCE : -ACTIVATION_DISTANCE;

  const Rectf& player_bbox = player->get_bbox();

  return ((player_bbox.p1.y >= bbox.p2.y) /* player is below us */
          && ((player_bbox.p2.x + x_offset) > bbox.p1.x)
          && ((player_bbox.p1.x + x_offset) < bbox.p2.x));
}

void
Owl::active_update (float elapsed_time)
{
  BadGuy::active_update (elapsed_time);

  if(frozen)
    return;

  if (carried_object != NULL) {
    if (!is_above_player ()) {
      Vector obj_pos = get_anchor_pos (bbox, ANCHOR_BOTTOM);
      obj_pos.x -= 16.0; /* FIXME: Actually do use the half width of the carried object here. */
      obj_pos.y += 3.0; /* Move a little away from the hitbox (the body). Looks nicer. */

      //To drop enemie before leave the screen
      if (obj_pos.x<=16 || obj_pos.x+16>=Sector::current()->get_width()){
        carried_object->ungrab (*this, dir);
        carried_object = NULL;
      }

     else
        carried_object->grab (*this, obj_pos, dir);
    }
    else { /* if (is_above_player) */
      carried_object->ungrab (*this, dir);
      carried_object = NULL;
    }
  }
}

bool
Owl::collision_squished(GameObject&)
{
  auto player = Sector::current()->get_nearest_player (bbox);
  if (player)
    player->bounce (*this);

  if (carried_object != NULL) {
    carried_object->ungrab (*this, dir);
    carried_object = NULL;
  }

  kill_fall ();
  return true;
}

void
Owl::kill_fall()
{
  SoundManager::current()->play("sounds/fall.wav", get_pos());
  physic.set_velocity_y(0);
  physic.set_acceleration_y(0);
  physic.enable_gravity(true);
  set_state(STATE_FALLING);

  if (carried_object != NULL) {
    carried_object->ungrab (*this, dir);
    carried_object = NULL;
  }

  // start dead-script
  run_dead_script();
}

void
Owl::freeze()
{
  if (carried_object != NULL) {
    carried_object->ungrab (*this, dir);
    carried_object = NULL;
  }
  physic.enable_gravity(true);
  BadGuy::freeze();
}

void
Owl::unfreeze()
{
  BadGuy::unfreeze();
  physic.set_velocity_x(dir == LEFT ? -FLYING_SPEED : FLYING_SPEED);
  physic.enable_gravity(false);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

bool
Owl::is_freezable() const
{
  return true;
}

void
Owl::collision_solid(const CollisionHit& hit)
{
  if(frozen)
  {
    BadGuy::collision_solid(hit);
    return;
  }
  if(hit.top || hit.bottom) {
    physic.set_velocity_y(0);
  } else if(hit.left || hit.right) {
    if (dir == LEFT) {
      set_action ("right", /* loops = */ -1);
      dir = RIGHT;
      physic.set_velocity_x (FLYING_SPEED);
    }
    else {
      set_action ("left", /* loops = */ -1);
      dir = LEFT;
      physic.set_velocity_x (-FLYING_SPEED);
    }
  }
} /* void Owl::collision_solid */

void
Owl::ignite() {
  if (carried_object != NULL) {
    carried_object->ungrab (*this, dir);
    carried_object = NULL;
  }
  BadGuy::ignite();
}

void
Owl::after_editor_set()
{
  BadGuy::after_editor_set();
  sprite->set_action(dir == LEFT ? "left" : "right");
}

/* vim: set sw=2 sts=2 et fdm=marker : */
/* EOF */
