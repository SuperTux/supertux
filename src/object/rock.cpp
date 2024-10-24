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
#include "badguy/crusher.hpp"
#include "badguy/badguy.hpp"
#include "object/coin.hpp"
#include "object/explosion.hpp"
#include "object/lit_object.hpp"
#include "object/pushbutton.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "object/player.hpp"
#include "util/reader_mapping.hpp"

namespace {
  const std::string ROCK_SOUND = "sounds/brick.wav"; //TODO use own sound.
  const float GROUND_FRICTION = 0.1f; // Amount of friction to apply while on ground.
} // namespace


Rock::Rock(const ReaderMapping& reader, const std::string& spritename) :
  MovingSprite(reader, spritename),
  m_physic(),
  m_on_ground(false),
  m_on_ice(false),
  m_last_movement(0.0f, 0.0f),
  m_on_grab_script(),
  m_on_ungrab_script(),
  m_running_grab_script(),
  m_running_ungrab_script()
{
  parse_type(reader);
  reader.get("on-grab-script", m_on_grab_script, "");
  reader.get("on-ungrab-script", m_on_ungrab_script, "");

  SoundManager::current()->preload(ROCK_SOUND);
  set_group(COLGROUP_MOVING_STATIC);
}

Rock::Rock(const Vector& pos, const std::string& spritename) :
  MovingSprite(pos, spritename),
  m_physic(),
  m_on_ground(false),
  m_on_ice(false),
  m_last_movement(0.0f, 0.0f),
  m_on_grab_script(),
  m_on_ungrab_script(),
  m_running_grab_script(),
  m_running_ungrab_script()
{
  SoundManager::current()->preload(ROCK_SOUND);
  set_group(COLGROUP_MOVING_STATIC);
}

GameObjectTypes
Rock::get_types() const
{
  return {
    { "small", _("Small") },
    { "large", _("Large") }
  };
}

std::string
Rock::get_default_sprite_name() const
{
  switch (m_type)
  {
    case LARGE:
      return "images/objects/rock/rock-b.png";
    default:
      return m_default_sprite_name;
  }
}

void
Rock::update(float dt_sec)
{
  if (!is_grabbed())
    m_col.set_movement(m_physic.get_movement(dt_sec));
}

void
Rock::handle_wind()
{
  if (!m_col.m_colliding_wind.empty())
  {
    if (m_on_ground && m_physic.get_wind_velocity_y() > 0.f)
      m_physic.set_wind_velocity_y(0.f);
  }
  else {
    m_physic.set_wind_velocity(Vector(0.f));
    m_physic.set_wind_acceleration(0.0);
  }
}

void
Rock::collision_solid(const CollisionHit& hit)
{
  if (is_grabbed()) {
    return;
  }
  if (hit.top || hit.bottom)
    m_physic.set_velocity_y(0);
  if (hit.left || hit.right) {
    // Bounce back slightly when hitting a wall
    float velx = m_physic.get_velocity_x();
    m_physic.set_velocity_x(-0.1f * velx);
  }
  if (hit.crush)
    m_physic.set_velocity(0, 0);

  if (hit.bottom  && !m_on_ground && !is_grabbed() && !m_on_ice) {
    SoundManager::current()->play(ROCK_SOUND, get_pos());
    m_physic.set_velocity_x(0);
    m_on_ground = true;
  }

  if (m_on_ground || (hit.bottom && m_on_ice)) {
    // Full friction!
    m_physic.set_velocity_x(m_physic.get_velocity_x() * (1.f - (GROUND_FRICTION * (m_on_ice ? 0.5f : 1.f))));
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

  // Why is it necessary to list exceptions here? Why doesn't the rock just not
  // affect object that have ABORT_MOVE on all collisions?
  auto litobject = dynamic_cast<LitObject*> (&other);
  if (litobject) {
    return ABORT_MOVE;
  }

  auto pushbutton = dynamic_cast<PushButton*> (&other);
  if (pushbutton) {
    return ABORT_MOVE;
  }

  if (is_grabbed()) {
    return ABORT_MOVE;
  }

  auto crusher = dynamic_cast<Crusher*> (&other);
  if (crusher) {
    return FORCE_MOVE;
  }

  if (hit.bottom) {
    auto player = dynamic_cast<Player*> (&other);
    if (player) {
      m_physic.set_velocity_y(-250.f);
    }
  }

  // Don't fall further if we are on a rock which is on the ground.
  // This is to avoid jittering.
  auto rock = dynamic_cast<Rock*> (&other);
  if (rock && rock->m_on_ground && hit.bottom) {
    m_physic.set_velocity_y(rock->get_physic().get_velocity_y());
    return CONTINUE;
  }

  if (!m_on_ground) {
    if (hit.bottom && m_physic.get_velocity_y() > 200) {
      auto badguy = dynamic_cast<BadGuy*> (&other);
      auto player = dynamic_cast<Player*> (&other);
      if (badguy && badguy->get_group() != COLGROUP_TOUCHABLE) {
        //Getting a rock on the head hurts. A lot.
        badguy->kill_fall();
        m_physic.set_velocity_y(0);
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
  m_last_movement = movement;
  set_group(COLGROUP_TOUCHABLE); //needed for lanterns catching willowisps
  m_on_ground = false;

  m_running_ungrab_script = false;
  if (!m_on_grab_script.empty() && !m_running_grab_script)
  {
    m_running_grab_script = true;
    Sector::get().run_script(m_on_grab_script, "Rock::on_grab");
  }
}

void
Rock::ungrab(MovingObject& object, Direction dir)
{
  auto player = dynamic_cast<Player*> (&object);
  set_group(COLGROUP_MOVING_STATIC);
  m_on_ground = false;
  if (player)
  {
    if (player->is_swimming() || player->is_water_jumping())
    {
      float swimangle = player->get_swimming_angle();
      m_physic.set_velocity(player->get_velocity() + Vector(std::cos(swimangle), std::sin(swimangle)));
    }
    else
    {
      m_physic.set_velocity_x(fabsf(player->get_physic().get_velocity_x()) < 1.f ? 0.f :
        player->m_dir == Direction::LEFT ? -200.f : 200.f);
      m_physic.set_velocity_y((dir == Direction::UP) ? -500.f : (dir == Direction::DOWN) ? 500.f :
        (glm::length(m_last_movement) > 1) ? -200.f : 0.f);
    }
  }

  m_running_grab_script = false;
  if (!m_on_ungrab_script.empty() && !m_running_ungrab_script)
  {
    m_running_ungrab_script = true;
    Sector::get().run_script(m_on_ungrab_script, "Rock::on_ungrab");
  }
  Portable::ungrab(object, dir);
}

ObjectSettings
Rock::get_settings()
{
  auto result = MovingSprite::get_settings();
  result.add_script(_("On-grab script"), &m_on_grab_script, "on-grab-script");
  result.add_script(_("On-ungrab script"), &m_on_ungrab_script, "on-ungrab-script");
  return result;
}

/* EOF */
