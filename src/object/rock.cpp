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
#include "util/reader_mapping.hpp"

namespace {
const std::string ROCK_SOUND = "sounds/brick.wav"; //TODO use own sound.
}

Rock::Rock(const Vector& pos, const std::string& spritename) :
  MovingSprite(pos, spritename),
  ExposedObject<Rock, scripting::Rock>(this),
  physic(),
  on_ground(false),
  grabbed(false),
  last_movement(),
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
  grabbed(false),
  last_movement(),
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
  grabbed(false),
  last_movement(),
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
  if ( grabbed )
    return;

  if (on_ground) physic.set_velocity_x(0);

  m_col.m_movement = physic.get_movement(dt_sec);
}

void
Rock::collision_solid(const CollisionHit& hit)
{
  if (grabbed) {
    return;
  }
  if (hit.top || hit.bottom)
    physic.set_velocity_y(0);
  if (hit.left || hit.right)
    physic.set_velocity_x(0);
  if (hit.crush)
    physic.set_velocity(0, 0);

  if (hit.bottom  && !on_ground && !grabbed) {
    SoundManager::current()->play(ROCK_SOUND, get_pos());
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

  if (grabbed) {
    return ABORT_MOVE;
  }
  if (!on_ground) {
    if (hit.bottom && physic.get_velocity_y() > 200) {
      auto moving_object = dynamic_cast<MovingObject*> (&other);
      if (moving_object) {
        //Getting a rock on the head hurts. A lot.
        moving_object->collision_tile(Tile::HURTS);
      }
    }
    return FORCE_MOVE;
  }

  return FORCE_MOVE;
}

void
Rock::grab(MovingObject& , const Vector& pos, Direction)
{
  m_col.m_movement = pos - get_pos();
  last_movement = m_col.m_movement;
  set_group(COLGROUP_TOUCHABLE); //needed for lanterns catching willowisps
  on_ground = false;
  grabbed = true;

  if (!on_grab_script.empty()) {
    Sector::get().run_script(on_grab_script, "Rock::on_grab");
  }
}

void
Rock::ungrab(MovingObject& , Direction dir)
{
  set_group(COLGROUP_MOVING_STATIC);
  on_ground = false;
  if (dir == Direction::UP) {
    physic.set_velocity(0, -500);
  } else if (last_movement.norm() > 1) {
    physic.set_velocity((dir == Direction::RIGHT) ? 200.0f : -200.0f, -200.0f);
  } else {
    physic.set_velocity(0, 0);
  }
  grabbed = false;

  if (!on_ungrab_script.empty()) {
    Sector::get().run_script(on_ungrab_script, "Rock::on_ungrab");
  }
}

ObjectSettings
Rock::get_settings()
{
  auto result = MovingSprite::get_settings();
  result.add_script(_("On-grab script"), &on_grab_script, "on-grab-script");
  result.add_script(_("On-ungrab script"), &on_ungrab_script, "on-ungrab-script");
  return result;
}


/* EOF */
