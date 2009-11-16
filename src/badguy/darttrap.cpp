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

namespace {
const float MUZZLE_Y = 25; /**< [px] muzzle y-offset from top */
}

DartTrap::DartTrap(const Reader& reader) :
  BadGuy(reader, "images/creatures/darttrap/darttrap.sprite", LAYER_TILES-1),
  initial_delay(0), 
  fire_delay(2), 
  ammo(-1), 
  state(IDLE),
  fire_timer()
{
  reader.get("initial-delay", initial_delay);
  reader.get("fire-delay", fire_delay);
  reader.get("ammo", ammo);
  countMe = false;
  sound_manager->preload("sounds/dartfire.wav");
  if (start_dir == AUTO) log_warning << "Setting a DartTrap's direction to AUTO is no good idea" << std::endl;
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
  if (state == IDLE) {
    if ((ammo != 0) && (fire_timer.check())) {
      if (ammo > 0) ammo--;
      load();
      fire_timer.start(fire_delay);
    }
  }
  if (state == LOADING) {
    if (sprite->animation_done()) {
      fire();
    }
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

  sound_manager->play("sounds/dartfire.wav", get_pos());
  Sector::current()->add_object(new Dart(Vector(px, py), dir, this));
  state = IDLE;
  sprite->set_action(dir == LEFT ? "idle-left" : "idle-right");
}

IMPLEMENT_FACTORY(DartTrap, "darttrap");

/* EOF */
