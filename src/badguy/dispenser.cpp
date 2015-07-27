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
#include "math/random_generator.hpp"
#include "object/bullet.hpp"
#include "object/player.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader.hpp"

#include <stdexcept>

Dispenser::Dispenser(const Reader& reader) :
  BadGuy(reader, "images/creatures/dispenser/dispenser.sprite"),
  cycle(),
  badguys(),
  next_badguy(),
  dispense_timer(),
  autotarget(),
  swivel(),
  broken(),
  random(),
  type()
{
  set_colgroup_active(COLGROUP_MOVING_STATIC);
  SoundManager::current()->preload("sounds/squish.wav");
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
  SoundManager::current()->play("sounds/squish.wav", get_pos());
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
        next_badguy = gameRandom.rand(badguys.size());
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
    if(badguy == "goldbomb") {
      log_warning << "goldbomb is not allowed to be dispensed" << std::endl;
      return;
    }

    try {
      GameObjectPtr game_object;
      Vector spawnpoint;
      Rectf object_bbox;

      /* Need to allocate the badguy first to figure out its bounding box. */
      game_object = ObjectFactory::instance().create(badguy, get_pos(), launchdir);
      if (game_object == NULL)
        throw std::runtime_error("Creating " + badguy + " object failed.");

      BadGuy& bad_guy = dynamic_cast<BadGuy&>(*game_object);

      object_bbox = bad_guy.get_bbox();

      if (type == "dropper")
      {
        spawnpoint = get_anchor_pos (get_bbox(), ANCHOR_BOTTOM);
        spawnpoint.x -= 0.5 * object_bbox.get_width();
      }
      else if ((type == "cannon") || (type == "rocketlauncher"))
      {
        spawnpoint = get_pos(); /* top-left corner of the cannon */
        if (launchdir == LEFT)
          spawnpoint.x -= object_bbox.get_width() + 1;
        else
          spawnpoint.x += get_bbox().get_width() + 1;
      }

      /* Now we set the real spawn position */
      bad_guy.set_pos(spawnpoint);

      /* We don't want to count dispensed badguys in level stats */
      if(bad_guy.countMe)
        bad_guy.countMe = false;

      Sector::current()->add_object(game_object);
    } catch(const std::exception& e) {
      log_warning << "Error dispensing badguy: " << e.what() << std::endl;
      return;
    }
  }
}

void
Dispenser::freeze()
{
  set_group(COLGROUP_MOVING_STATIC);
  frozen = true;

  if(type=="rocketlauncher" && sprite->has_action("iced-left"))
    // Only swivel dispensers can use their left/right iced actions.
    sprite->set_action(dir == LEFT ? "iced-left" : "iced-right", 1);
  // when the sprite doesn't have separate actions for left and right or isn't a rocketlauncher,
  // it tries to use an universal one.
  else
  {
    if(type=="cannon" && sprite->has_action("iced"))
      sprite->set_action("iced", 1);
      // When is the dispenser a cannon, it uses the "iced" action.
    else
    {
      if(sprite->has_action("dropper-iced"))
        sprite->set_action("dropper-iced", 1);
        // When is the dispenser a dropper, it uses the "dropper-iced".
      else
      {
        sprite->set_color(Color(0.60, 0.72, 0.88f));
        sprite->stop_animation();
        // When is the dispenser something else (unprobable), or has no matching iced sprite, it shades to blue.
      }
    }
  }
  dispense_timer.stop();
}

void
Dispenser::unfreeze()
{
  set_group(colgroup_active);
  frozen = false;

  // restore original color if needed
  if(((!sprite->has_action("iced-left") && type=="rocketlauncher")
      || (!sprite->has_action("iced") && type=="cannon"))
     && (!sprite->has_action("dropper-iced")) )
  {
    sprite->set_color(Color(1.00, 1.00, 1.00f));
    sprite->set_animation_loops();
  }
  activate();
}

bool
Dispenser::is_freezable() const
{
  return true;
}

/* EOF */
