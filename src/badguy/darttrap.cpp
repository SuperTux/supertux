//  DartTrap - Shoots a Dart at regular intervals
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "badguy/dart.hpp"
#include "badguy/darttrap.hpp"

#include "audio/sound_manager.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

namespace {
const float MUZZLE_Y = 25; /**< [px] muzzle y-offset from top */
}

DartTrap::DartTrap(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/darttrap/darttrap.sprite", LAYER_TILES-1),
  enabled(true),
  initial_delay(),
  fire_delay(),
  ammo(),
  state(IDLE),
  fire_timer()
{
  reader.get("enabled", enabled, true);
  reader.get("initial-delay", initial_delay, 0);
  reader.get("fire-delay", fire_delay, 2);
  reader.get("ammo", ammo, -1);
  countMe = false;
  SoundManager::current()->preload("sounds/dartfire.wav");
  if (start_dir == AUTO) { log_warning << "Setting a DartTrap's direction to AUTO is no good idea" << std::endl; }
  state = IDLE;
  set_colgroup_active(COLGROUP_DISABLED);
  if (initial_delay == 0) initial_delay = 0.1f;
}

void
DartTrap::initialize()
{
  sprite->set_action(dir == LEFT ? "idle-left" : "idle-right");
}

void
DartTrap::activate()
{
  fire_timer.start(initial_delay);
}

HitResponse
DartTrap::collision_player(Player& , const CollisionHit& )
{
  return ABORT_MOVE;
}

void
DartTrap::active_update(float )
{
  if(!enabled) {
    return;
  }
  switch (state) {
    case IDLE:
      if ((ammo != 0) && (fire_timer.check())) {
        if (ammo > 0) ammo--;
        load();
        fire_timer.start(fire_delay);
      }
      break;

    case LOADING:
      if (sprite->animation_done()) {
        fire();
      }
      break;

    default:
      break;
  }
}

void
DartTrap::load()
{
  state = LOADING;
  sprite->set_action(dir == LEFT ? "loading-left" : "loading-right", 1);
}

void
DartTrap::fire()
{
  float px = get_pos().x;
  if (dir == RIGHT) px += 5;
  float py = get_pos().y;
  py += MUZZLE_Y;

  SoundManager::current()->play("sounds/dartfire.wav", get_pos());
  Sector::current()->add_object(std::make_shared<Dart>(Vector(px, py), dir, this));
  state = IDLE;
  sprite->set_action(dir == LEFT ? "idle-left" : "idle-right");
}


ObjectSettings
DartTrap::get_settings() {
  ObjectSettings result = BadGuy::get_settings();
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Enabled"), &enabled,
                                         "enabled"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Initial delay"), &initial_delay,
                                         "initial-delay"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Fire delay"), &fire_delay,
                                         "fire-delay"));
  result.options.push_back( ObjectOption(MN_INTFIELD, _("Ammo"), &ammo,
                                         "ammo"));

  return result;
}

void
DartTrap::after_editor_set()
{
  BadGuy::after_editor_set();
  sprite->set_action(dir == LEFT ? "idle-left" : "idle-right");
}

/* EOF */
