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
#include "lisp/writer.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"

namespace {
const float SPEED = 200;
}

static const std::string SOUNDFILE = "sounds/flame.wav";

Dart::Dart(const lisp::Lisp& reader) :
  BadGuy(reader, "images/creatures/dart/dart.sprite"), 
  parent(0),
  sound_source()
{
  physic.enable_gravity(false);
  countMe = false;
  sound_manager->preload(SOUNDFILE);
  sound_manager->preload("sounds/darthit.wav");
  sound_manager->preload("sounds/stomp.wav");
}

Dart::Dart(const Vector& pos, Direction d, const BadGuy* parent = 0) :
  BadGuy(pos, d, "images/creatures/dart/dart.sprite"), 
  parent(parent),
  sound_source()
{
  physic.enable_gravity(false);
  countMe = false;
  sound_manager->preload(SOUNDFILE);
  sound_manager->preload("sounds/darthit.wav");
  sound_manager->preload("sounds/stomp.wav");
}

Dart::~Dart()
{
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
Dart::write(lisp::Writer& writer)
{
  writer.start_list("dart");
  writer.write("x", start_position.x);
  writer.write("y", start_position.y);
  writer.end_list("dart");
}

void
Dart::initialize()
{
  physic.set_velocity_x(dir == LEFT ? -::SPEED : ::SPEED);
  sprite->set_action(dir == LEFT ? "flying-left" : "flying-right");
}

void
Dart::activate()
{
  sound_source.reset(sound_manager->create_sound_source(SOUNDFILE));
  sound_source->set_position(get_pos());
  sound_source->set_looping(true);
  sound_source->set_gain(1.0);
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
Dart::active_update(float elapsed_time)
{
  BadGuy::active_update(elapsed_time);
  sound_source->set_position(get_pos());
}

void
Dart::collision_solid(const CollisionHit& )
{
  sound_manager->play("sounds/darthit.wav", get_pos());
  remove_me();
}

HitResponse
Dart::collision_badguy(BadGuy& badguy, const CollisionHit& )
{
  // ignore collisions with parent
  if (&badguy == parent) {
    return FORCE_MOVE;
  }
  sound_manager->play("sounds/stomp.wav", get_pos());
  remove_me();
  badguy.kill_fall();
  return ABORT_MOVE;
}

HitResponse
Dart::collision_player(Player& player, const CollisionHit& hit)
{
  sound_manager->play("sounds/stomp.wav", get_pos());
  remove_me();
  return BadGuy::collision_player(player, hit);
}

IMPLEMENT_FACTORY(Dart, "dart");

/* EOF */
