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
#include "editor/editor.hpp"
#include "math/random_generator.hpp"
#include "object/bullet.hpp"
#include "object/player.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

#include <stdexcept>

Dispenser::Dispenser(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/dispenser/dispenser.sprite"),
  colgroup_active(COLGROUP_MOVING_STATIC),
  cycle(),
  badguys(),
  next_badguy(0),
  dispense_timer(),
  autotarget(false),
  swivel(false),
  broken(false),
  random(),
  type(),
  type_str()
{
  SoundManager::current()->preload("sounds/squish.wav");
  if ( !reader.get("cycle", cycle)) cycle = 5;
  if ( !reader.get("badguy", badguys)) badguys.clear();
  if ( !reader.get("random", random)) random = false;
  std::string type_s = "dropper"; //default
  if ( !reader.get("type", type_s)) type_s = "";
  if (type_s == "dropper") {
    type = DT_DROPPER;
  } else if (type_s == "rocketlauncher") {
    type = DT_ROCKETLAUNCHER;
  } else if (type_s == "cannon") {
    type = DT_CANNON;
  } else if (type_s == "point") {
    type = DT_POINT;
  } else {
    if(type_s.empty()) {
      log_warning << "No dispenser type set, setting to dropper." << std::endl;
    }
    else {
      log_warning << "Unknown type of dispenser:" << type_s << ", setting to dropper." << std::endl;
    }
    type = DT_DROPPER;
  }
  type_str = get_type_string();

//  if (badguys.size() <= 0)
//    throw std::runtime_error("No badguys in dispenser.");

  switch (type) {
    case DT_DROPPER:
      sprite->set_action("dropper");
      break;
    case DT_ROCKETLAUNCHER:
      sprite->set_action(dir == LEFT ? "working-left" : "working-right");
      set_colgroup_active(COLGROUP_MOVING); //if this were COLGROUP_MOVING_STATIC MrRocket would explode on launch.

      if (start_dir == AUTO) {
        autotarget = true;
      }
      break;
    case DT_CANNON:
      sprite->set_action("working");
      break;
    case DT_POINT:
      sprite->set_action("invisible");
      set_colgroup_active(COLGROUP_DISABLED);
    default:
      break;
  }

  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  countMe = false;
}

void
Dispenser::draw(DrawingContext& context) {
  if (type != DT_POINT || Editor::is_active()) {
    BadGuy::draw(context);
  }
}

void
Dispenser::activate()
{
  if( broken ){
    return;
  }
  if( autotarget && !swivel ){ // auto cannon sprite might be wrong
    auto player = get_nearest_player();
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
  if (broken || type != DT_ROCKETLAUNCHER) {
    return false;
  }

  if (frozen) {
    unfreeze();
  }

  sprite->set_action(dir == LEFT ? "broken-left" : "broken-right");
  dispense_timer.start(0);
  set_colgroup_active(COLGROUP_MOVING_STATIC); // Tux can stand on broken cannon.
  auto player = dynamic_cast<Player*>(&object);
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
  auto player = dynamic_cast<Player*> (&other);
  if(player) {
    // hit from above?
    if (player->get_bbox().p2.y < (bbox.p1.y + 16)) {
      collision_squished(*player);
      return FORCE_MOVE;
    }
    if(frozen && type != DT_CANNON){
      unfreeze();
    }
    return FORCE_MOVE;
  }

  auto bullet = dynamic_cast<Bullet*> (&other);
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

      auto player = get_nearest_player();
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
  if (badguys.empty()) return;
  if (frozen) return;

  //FIXME: Does is_offscreen() work right here?
  if (!is_offscreen() && !Editor::is_active()) {
    Direction launchdir = dir;
    if( !autotarget && start_dir == AUTO ){
      Player* player = get_nearest_player();
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

      auto& bad_guy = dynamic_cast<BadGuy&>(*game_object);

      object_bbox = bad_guy.get_bbox();

      switch (type) {
        case DT_DROPPER:
          spawnpoint = get_anchor_pos (bbox, ANCHOR_BOTTOM);
          spawnpoint.x -= 0.5 * object_bbox.get_width();
          break;
        case DT_ROCKETLAUNCHER:
        case DT_CANNON:
          spawnpoint = get_pos(); /* top-left corner of the cannon */
          if (launchdir == LEFT)
            spawnpoint.x -= object_bbox.get_width() + 1;
          else
            spawnpoint.x += bbox.get_width() + 1;
          break;
        case DT_POINT:
          spawnpoint = bbox.p1;
        default:
          break;
      }

      /* Now we set the real spawn position */
      bad_guy.set_pos(spawnpoint);

      /* We don't want to count dispensed badguys in level stats */
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
  if (broken) {
    return;
  }

  set_group(COLGROUP_MOVING_STATIC);
  frozen = true;

    if(type == DT_ROCKETLAUNCHER && sprite->has_action("iced-left"))
    // Only swivel dispensers can use their left/right iced actions.
    sprite->set_action(dir == LEFT ? "iced-left" : "iced-right", 1);
    // when the sprite doesn't have separate actions for left and right or isn't a rocketlauncher,
    // it tries to use an universal one.
  else
  {
    if(type == DT_CANNON && sprite->has_action("iced"))
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

  sprite->set_color(Color(1.00, 1.00, 1.00f));
  set_correct_action();
  activate();
}

bool
Dispenser::is_freezable() const
{
  return true;
}

bool
Dispenser::is_flammable() const
{
  return false;
}

void
Dispenser::set_correct_action()
{
  switch (type) {
    case DT_DROPPER:
      sprite->set_action("dropper");
      break;
    case DT_ROCKETLAUNCHER:
      sprite->set_action(dir == LEFT ? "working-left" : "working-right");
      break;
    case DT_CANNON:
      sprite->set_action("working");
      break;
    case DT_POINT:
      sprite->set_action("invisible");
      break;
    default:
      break;
  }
}

ObjectSettings
Dispenser::get_settings()
{
  ObjectSettings result = BadGuy::get_settings();
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Interval (seconds)"), &cycle,
                                         "cycle"));
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Random"), &random,
                                         "random"));
  result.options.push_back( ObjectOption(MN_BADGUYSELECT, _("Enemies"), &badguys,
                                         "badguy"));

  ObjectOption seq(MN_STRINGSELECT, _("Type"), &type);
  seq.select.push_back(_("dropper"));
  seq.select.push_back(_("rocket launcher"));
  seq.select.push_back(_("cannon"));
  seq.select.push_back(_("invisible"));

  result.options.push_back( seq );

  type_str = get_type_string();
  result.options.push_back( ObjectOption(MN_TEXTFIELD, "type", &type_str, "type", false));
  return result;
}

void
Dispenser::after_editor_set()
{
  BadGuy::after_editor_set();
  set_correct_action();
}

/* EOF */
