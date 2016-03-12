//  MoleRock - Rock thrown by "Mole" Badguy
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

#include "badguy/mole_rock.hpp"

#include "audio/sound_manager.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"

MoleRock::MoleRock(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/mole/mole_rock.sprite", LAYER_TILES - 2),
  parent(0),
  initial_velocity(Vector(0, -400))
{
  physic.enable_gravity(true);
  countMe = false;
  SoundManager::current()->preload("sounds/darthit.wav");
  SoundManager::current()->preload("sounds/stomp.wav");
}

MoleRock::MoleRock(const Vector& pos, const Vector& velocity, const BadGuy* parent_ = 0) :
  BadGuy(pos, LEFT, "images/creatures/mole/mole_rock.sprite", LAYER_TILES - 2),
  parent(parent_),
  initial_velocity(velocity)
{
  physic.enable_gravity(true);
  countMe = false;
  SoundManager::current()->preload("sounds/darthit.wav");
  SoundManager::current()->preload("sounds/stomp.wav");
}

MoleRock::~MoleRock()
{
}

bool
MoleRock::updatePointers(const GameObject* from_object, GameObject* to_object)
{
  if (from_object == parent) {
    parent = dynamic_cast<MoleRock*>(to_object);
    return true;
  }
  return false;
}

void
MoleRock::initialize()
{
  physic.set_velocity(initial_velocity);
  sprite->set_action("default");
}

void
MoleRock::deactivate()
{
  remove_me();
}

void
MoleRock::active_update(float elapsed_time)
{
  BadGuy::active_update(elapsed_time);
}

void
MoleRock::collision_solid(const CollisionHit& )
{
  SoundManager::current()->play("sounds/darthit.wav", get_pos());
  remove_me();
}

HitResponse
MoleRock::collision_badguy(BadGuy& badguy, const CollisionHit& )
{
  // ignore collisions with parent
  if (&badguy == parent) {
    return FORCE_MOVE;
  }
  SoundManager::current()->play("sounds/stomp.wav", get_pos());
  remove_me();
  badguy.kill_fall();
  return ABORT_MOVE;
}

HitResponse
MoleRock::collision_player(Player& player, const CollisionHit& hit)
{
  SoundManager::current()->play("sounds/stomp.wav", get_pos());
  remove_me();
  return BadGuy::collision_player(player, hit);
}

bool
MoleRock::is_flammable() const
{
  return false;
}

/* EOF */
