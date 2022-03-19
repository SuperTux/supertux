//  Dart - Your average poison dart
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

#include "audio/sound_manager.hpp"
#include "audio/sound_source.hpp"
#include "sprite/sprite.hpp"
#include "supertux/flip_level_transformer.hpp"

namespace {
const float DART_SPEED = 200;
}

static const std::string DART_SOUND = "sounds/flame.wav";

Dart::Dart(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/dart/dart.sprite"),
  parent(nullptr),
  sound_source()
{
  m_physic.enable_gravity(false);
  m_countMe = false;
  SoundManager::current()->preload(DART_SOUND);
  SoundManager::current()->preload("sounds/darthit.wav");
  SoundManager::current()->preload("sounds/stomp.wav");
}

Dart::Dart(const Vector& pos, Direction d, const BadGuy* parent_ = nullptr) :
  BadGuy(pos, d, "images/creatures/dart/dart.sprite"),
  parent(parent_),
  sound_source()
{
  m_physic.enable_gravity(false);
  m_countMe = false;
  SoundManager::current()->preload(DART_SOUND);
  SoundManager::current()->preload("sounds/darthit.wav");
  SoundManager::current()->preload("sounds/stomp.wav");
}

bool
Dart::updatePointers(const GameObject* from_object, GameObject* to_object)
{
  if (from_object == parent) {
    parent = dynamic_cast<Dart*>(to_object);
    return true;
  }
  return false;
}

void
Dart::initialize()
{
  m_physic.set_velocity_x(m_dir == Direction::LEFT ? -::DART_SPEED : ::DART_SPEED);
  m_sprite->set_action(m_dir == Direction::LEFT ? "flying-left" : "flying-right");
}

void
Dart::activate()
{
  sound_source = SoundManager::current()->create_sound_source(DART_SOUND);
  sound_source->set_position(get_pos());
  sound_source->set_looping(true);
  sound_source->set_gain(0.5f);
  sound_source->set_reference_distance(32);
  sound_source->play();
}

void
Dart::deactivate()
{
  sound_source.reset();
  remove_me();
}

void
Dart::active_update(float dt_sec)
{
  BadGuy::active_update(dt_sec);
  sound_source->set_position(get_pos());
}

void
Dart::collision_solid(const CollisionHit& )
{
  SoundManager::current()->play("sounds/darthit.wav", get_pos());
  remove_me();
}

HitResponse
Dart::collision_badguy(BadGuy& badguy, const CollisionHit& )
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
Dart::collision_player(Player& player, const CollisionHit& hit)
{
  SoundManager::current()->play("sounds/stomp.wav", get_pos());
  remove_me();
  return BadGuy::collision_player(player, hit);
}

bool
Dart::is_flammable() const
{
  return false;
}

void
Dart::stop_looping_sounds()
{
  if (sound_source) {
    sound_source->stop();
  }
}

void
Dart::play_looping_sounds()
{
  if (sound_source) {
    sound_source->play();
  }
}

void
Dart::on_flip(float height)
{
  BadGuy::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
}

/* EOF */
