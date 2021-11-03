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

#include "object/rock.hpp"

#include "audio/sound_manager.hpp"
#include "object/explosion.hpp"
#include "object/coin.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "object/player.hpp"
#include "util/reader_mapping.hpp"

namespace {
const std::string ROCK_SOUND = "sounds/brick.wav"; //TODO use own sound.
}

Rock::Rock(const Vector& pos, const std::string& spritename) :
  MovingSprite(pos, spritename),
  ExposedObject<Rock, scripting::Rock>(this),
  physic(),
  on_ground(false),
  last_movement(0.0f, 0.0f),
  on_grab_script(),
  on_ungrab_script()
{
  SoundManager::current()->preload(ROCK_SOUND);
  set_group(COLGROUP_MOVING_STATIC);
}

Rock::Rock(const ReaderMapping& reader) :
  MovingSprite(reader, "images/objects/rock/rock.sprite"),
  ExposedObject<Rock, scripting::Rock>(this),
  physic(),
  on_ground(false),
  last_movement(0.0f, 0.0f),
  on_grab_script(),
  on_ungrab_script()
{
  reader.get("on-grab-script", on_grab_script, "");
  reader.get("on-ungrab-script", on_ungrab_script, "");
  SoundManager::current()->preload(ROCK_SOUND);
  set_group(COLGROUP_MOVING_STATIC);
}

Rock::Rock(const ReaderMapping& reader, const std::string& spritename) :
  MovingSprite(reader, spritename),
  ExposedObject<Rock, scripting::Rock>(this),
  physic(),
  on_ground(false),
  last_movement(0.0f, 0.0f),
  on_grab_script(),
  on_ungrab_script()
{
  if (!reader.get("on-grab-script", on_grab_script)) on_grab_script = "";
  if (!reader.get("on-ungrab-script", on_ungrab_script)) on_ungrab_script = "";
  SoundManager::current()->preload(ROCK_SOUND);
  set_group(COLGROUP_MOVING_STATIC);
}

void
Rock::update(float dt_sec)
{
  if (!is_grabbed())
    m_col.set_movement(physic.get_movement(dt_sec));
}

void
Rock::collision_solid(const CollisionHit& hit)
{
  if (is_grabbed()) {
    return;
  }
  if (hit.top || hit.bottom)
    physic.set_velocity_y(0);
  if (hit.left || hit.right) {
    // Bounce back slightly when hitting a wall
    float velx = physic.get_velocity_x();
    physic.set_velocity_x(-0.1f * velx);
  }
  if (hit.crush)
    physic.set_velocity(0, 0);

  if (hit.bottom  && !on_ground && !is_grabbed()) {
    SoundManager::current()->play(ROCK_SOUND, get_pos());
    physic.set_velocity_x(0);
    on_ground = true;
  }
}

HitResponse
Rock::collision(GameObject& other, const CollisionHit& hit)
{
  auto heavy_coin = dynamic_cast<HeavyCoin*> (&other);
  if (heavy_coin) {
    return ABORT_MOVE;
  }

  auto explosion = dynamic_cast<Explosion*> (&other);
  if (explosion) {
    return ABORT_MOVE;
  }

  if (is_grabbed()) {
    return ABORT_MOVE;
  }

  // Don't fall further if we are on a rock which is on the ground.
  // This is to avoid jittering.
  auto rock = dynamic_cast<Rock*> (&other);
  if (rock && rock->on_ground && hit.bottom) {
    physic.set_velocity_y(0);
    return CONTINUE;
  }

  if (!on_ground) {
    if (hit.bottom && physic.get_velocity_y() > 200) {
      auto moving_object = dynamic_cast<MovingObject*> (&other);
      if (moving_object && moving_object->get_group() != COLGROUP_TOUCHABLE) {
        //Getting a rock on the head hurts. A lot.
        moving_object->collision_tile(Tile::HURTS);
        physic.set_velocity_y(0);
      }
    }
    return FORCE_MOVE;
  }

  return FORCE_MOVE;
}

void
Rock::grab(MovingObject& object, const Vector& pos, Direction dir_)
{
  Portable::grab(object, pos, dir_);
  Vector movement = pos - get_pos();
  m_col.set_movement(movement);
  last_movement = movement;
  set_group(COLGROUP_TOUCHABLE); //needed for lanterns catching willowisps
  on_ground = false;

  if (!on_grab_script.empty()) {
    Sector::get().run_script(on_grab_script, "Rock::on_grab");
  }
}

void
Rock::ungrab(MovingObject& object, Direction dir)
{
  auto player = dynamic_cast<Player*> (&object);
  set_group(COLGROUP_MOVING_STATIC);
  on_ground = false;
  if (player != nullptr && (player->is_swimming() || player->is_water_jumping()))
  {
    float swimangle = player->get_swimming_angle();
    physic.set_velocity(player->get_velocity() + Vector(std::cos(swimangle), std::sin(swimangle)));
  }
  else
  {
    set_group(COLGROUP_MOVING_STATIC);
    on_ground = false;
    if (dir == Direction::UP) {
      physic.set_velocity(0, -500);
    }
    else if (dir == Direction::DOWN) {
      physic.set_velocity(0, 500);
    }
    else if (glm::length(last_movement) > 1) {
      physic.set_velocity((dir == Direction::RIGHT) ? 200.0f : -200.0f, -200.0f);
    }
    else {
      physic.set_velocity(0, 0);
    }
  }

  if (!on_ungrab_script.empty()) {
    Sector::get().run_script(on_ungrab_script, "Rock::on_ungrab");
  }
  Portable::ungrab(object, dir);
}

ObjectSettings
Rock::get_settings()
{
  auto result = MovingSprite::get_settings();
  result.add_script(_("On-grab script"), &on_grab_script, "on-grab-script");
  result.add_script(_("On-ungrab script"), &on_ungrab_script, "on-ungrab-script");
  return result;
}

void
Rock::add_wind_velocity(const Vector& velocity, const Vector& end_speed)
{
  // only add velocity in the same direction as the wind
  if (end_speed.x > 0 && physic.get_velocity_x() < end_speed.x)
    physic.set_velocity_x(std::min(physic.get_velocity_x() + velocity.x, end_speed.x));
  if (end_speed.x < 0 && physic.get_velocity_x() > end_speed.x)
    physic.set_velocity_x(std::max(physic.get_velocity_x() + velocity.x, end_speed.x));
  if (end_speed.y > 0 && physic.get_velocity_y() < end_speed.y)
    physic.set_velocity_y(std::min(physic.get_velocity_y() + velocity.y, end_speed.y));
  if (end_speed.y < 0 && physic.get_velocity_y() > end_speed.y)
    physic.set_velocity_y(std::max(physic.get_velocity_y() + velocity.y, end_speed.y));
}

/* EOF */
