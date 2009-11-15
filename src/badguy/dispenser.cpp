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

#include "badguy/dispenser.hpp"

#include "audio/sound_manager.hpp"
#include "lisp/writer.hpp"
#include "math/random_generator.hpp"
#include "object/bullet.hpp"
#include "object/player.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

Dispenser::Dispenser(const lisp::Lisp& reader) :
  BadGuy(reader, "images/creatures/dispenser/dispenser.sprite")
{
  set_colgroup_active(COLGROUP_MOVING_STATIC);
  sound_manager->preload("sounds/squish.wav");
  reader.get("cycle", cycle);
  reader.get("badguy", badguys);
  random = false; // default
  reader.get("random", random);
  type = "dropper"; //default
  reader.get("type", type);
  next_badguy = 0;
  autotarget = false;
  swivel = false;
  broken = false;

  if (badguys.size() <= 0)
    throw std::runtime_error("No badguys in dispenser.");

  if (type == "rocketlauncher") {
    sprite->set_action(dir == LEFT ? "working-left" : "working-right");
    set_colgroup_active(COLGROUP_MOVING); //if this were COLGROUP_MOVING_STATIC MrRocket would explode on launch.

    if (start_dir == AUTO) {
      autotarget = true;
    }
  } else if (type == "cannon") {
    sprite->set_action("working");
  } else {
    sprite->set_action("dropper");
  }

  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  countMe = false;
}

void
Dispenser::write(lisp::Writer& writer)
{
  writer.start_list("dispenser");

  writer.write("x", start_position.x);
  writer.write("y", start_position.y);
  writer.write("cycle", cycle);
  writer.write("random", random);
  writer.write("type", type);
  writer.write("badguy", badguys);

  writer.end_list("dispenser");
}

void
Dispenser::activate()
{
   if( broken ){
     return;
   }
   if( autotarget && !swivel ){ // auto cannon sprite might be wrong
      Player* player = this->get_nearest_player();
      if( player ){
        dir = (player->get_pos().x > get_pos().x) ? RIGHT : LEFT;
        sprite->set_action(dir == LEFT ? "working-left" : "working-right");
      }
   }
   dispense_timer.start(cycle, true);
   launch_badguy();
}

void
Dispenser::deactivate()
{
   dispense_timer.stop();
}

//TODO: Add launching velocity to certain badguys
bool
Dispenser::collision_squished(GameObject& object)
{
  //Cannon launching MrRocket can be broken by jumping on it
  //other dispensers are not that fragile.
  if (broken || type != "rocketlauncher") {
    return false;
  }

  sprite->set_action(dir == LEFT ? "broken-left" : "broken-right");
  dispense_timer.start(0);
  set_colgroup_active(COLGROUP_MOVING_STATIC); // Tux can stand on broken cannon.
  Player* player = dynamic_cast<Player*>(&object);
  if (player){
    player->bounce(*this);
  }
  sound_manager->play("sounds/squish.wav", get_pos());
  broken = true;
  return true;
}

HitResponse
Dispenser::collision(GameObject& other, const CollisionHit& hit)
{
  Player* player = dynamic_cast<Player*> (&other);
  if(player) {
    // hit from above?
    if (player->get_bbox().p2.y < (bbox.p1.y + 16)) {
      collision_squished(*player);
      return FORCE_MOVE;
    }
    if(frozen){
      unfreeze();
    }
    return FORCE_MOVE;
  }

  Bullet* bullet = dynamic_cast<Bullet*> (&other);
  if(bullet){
    return collision_bullet(*bullet, hit);
  }

  return FORCE_MOVE;
}

void
Dispenser::active_update(float )
{
  if (dispense_timer.check()) {
    // auto always shoots in Tux's direction
    if( autotarget ){ 
      if( sprite->animation_done()) {
        sprite->set_action(dir == LEFT ? "working-left" : "working-right");
        swivel = false;
      }

      Player* player = this->get_nearest_player();
      if( player && !swivel ){
        Direction targetdir = (player->get_pos().x > get_pos().x) ? RIGHT : LEFT;
        if( dir != targetdir ){ // no target: swivel cannon 
          swivel = true;
          dir = targetdir;
          sprite->set_action(dir == LEFT ? "swivel-left" : "swivel-right", 1);
        } else { // tux in sight: shoot
          launch_badguy();
        }
      }
    } else {
      launch_badguy();
    }
  }
}

void
Dispenser::launch_badguy()
{
  //FIXME: Does is_offscreen() work right here?
  if (!is_offscreen()) {
    Direction launchdir = dir;
    if( !autotarget && start_dir == AUTO ){
      Player* player = this->get_nearest_player();
      if( player ){
        launchdir = (player->get_pos().x > get_pos().x) ? RIGHT : LEFT;
      } 
    } 

    if (badguys.size() > 1) {
      if (random) {
        next_badguy = systemRandom.rand(badguys.size());
      }
      else {
        next_badguy++;

        if (next_badguy >= badguys.size())
          next_badguy = 0;
      }
    }

    std::string badguy = badguys[next_badguy];

    if(badguy == "random") {
      log_warning << "random is outdated; use a list of badguys to select from." << std::endl;
      return;
    }

    GameObject* badguy_object = NULL;

    try {
      Vector spawnpoint;

      if (type == "dropper")
        spawnpoint = Vector(get_pos().x, get_pos().y+32);
      else if (type == "cannon")
        spawnpoint = Vector(get_pos().x + (launchdir == LEFT ? -32 : 32), get_pos().y);
      else if (type == "rocketlauncher")
        spawnpoint = Vector(get_pos().x + (launchdir == LEFT ? -32 : 32), get_pos().y);

      badguy_object = create_object(badguy, Vector(get_pos().x, get_pos().y+32), launchdir);

      if (badguy_object)
        Sector::current()->add_object(badguy_object);
    } catch(std::exception& e) {
      log_warning << "Error dispensing badguy: " << e.what() << std::endl;
      return;
    }
  }
}

void
Dispenser::freeze()
{
  BadGuy::freeze();
  dispense_timer.stop();
}

void
Dispenser::unfreeze()
{
  BadGuy::unfreeze();
  activate();
}

bool
Dispenser::is_freezable() const
{
  return true;
}
IMPLEMENT_FACTORY(Dispenser, "dispenser");

/* EOF */
