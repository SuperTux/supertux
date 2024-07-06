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
#include "object/trampoline.hpp"
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
  physic(),
  on_ground(false),
  on_ice(false),
  last_movement(0.0f, 0.0f),
  on_grab_script(),
  on_ungrab_script(),
  running_grab_script(),
  running_ungrab_script()
{
  parse_type(reader);
  reader.get("on-grab-script", on_grab_script, "");
  reader.get("on-ungrab-script", on_ungrab_script, "");

  SoundManager::current()->preload(ROCK_SOUND);
  set_group(COLGROUP_MOVING_STATIC);
}

Rock::Rock(const Vector& pos, const std::string& spritename) :
  MovingSprite(pos, spritename),
  physic(),
  on_ground(false),
  on_ice(false),
  last_movement(0.0f, 0.0f),
  on_grab_script(),
  on_ungrab_script(),
  running_grab_script(),
  running_ungrab_script()
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
  if (!is_grabbed()) {

    if (get_bbox().get_top() > Sector::get().get_height()) {
      remove_me();
    }

    Rectf icebox = get_bbox().grown(-1.f);
    icebox.set_bottom(get_bbox().get_bottom() + 8.f);
    on_ice = !Sector::get().is_free_of_tiles(icebox, true, Tile::ICE);

    bool in_water = !Sector::get().is_free_of_tiles(get_bbox(), true, Tile::WATER);
    physic.set_gravity_modifier(in_water ? 0.2f : 1.f);

    Rectf trampolinebox = get_bbox().grown(-1.f);
    trampolinebox.set_bottom(get_bbox().get_bottom() + 8.f);

    for (auto& trampoline : Sector::get().get_objects_by_type<Trampoline>()) {
      if (trampolinebox.overlaps(trampoline.get_bbox()) && !trampoline.is_grabbed() &&
        (glm::length((get_bbox().get_middle() - trampoline.get_bbox().get_middle())) >= 10.f) &&
        is_portable()) {
        trampoline.bounce();
        physic.set_velocity_y(-500.f);
      }
    }

    Rectf playerbox = get_bbox().grown(-2.f);
    playerbox.set_bottom(get_bbox().get_bottom() + 7.f);
    for (auto& player : Sector::get().get_objects_by_type<Player>()) {
      if (playerbox.overlaps(player.get_bbox()) && physic.get_velocity_y() > 0.f && is_portable()) {
        physic.set_velocity_y(-250.f);
      }
    }

    m_col.set_movement(physic.get_movement(dt_sec) *
      Vector(in_water ? 0.4f : 1.f, in_water ? 0.6f : 1.f));
  }
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

  if (hit.bottom  && !on_ground && !is_grabbed() && !on_ice) {
    SoundManager::current()->play(ROCK_SOUND, get_pos());
    physic.set_velocity_x(0);
    on_ground = true;
  }

  if (on_ground || (hit.bottom && on_ice)) {
    // Full friction!
    physic.set_velocity_x(physic.get_velocity_x() * (1.f - (GROUND_FRICTION * (on_ice ? 0.5f : 1.f))));
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
      physic.set_velocity_y(-250.f);
    }
  }

  // Don't fall further if we are on a rock which is on the ground.
  // This is to avoid jittering.
  auto rock = dynamic_cast<Rock*> (&other);
  if (rock && rock->on_ground && hit.bottom) {
    physic.set_velocity_y(rock->get_physic().get_velocity_y());
    return CONTINUE;
  }

  if (!on_ground) {
    if (hit.bottom && physic.get_velocity_y() > 200) {
      auto badguy = dynamic_cast<BadGuy*> (&other);
      if (badguy && badguy->get_group() != COLGROUP_TOUCHABLE) {
        //Getting a rock on the head hurts. A lot.
        badguy->kill_fall();
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

  running_ungrab_script = false;
  if (!on_grab_script.empty() && !running_grab_script)
  {
    running_grab_script = true;
    Sector::get().run_script(on_grab_script, "Rock::on_grab");
  }
}

void
Rock::ungrab(MovingObject& object, Direction dir)
{
  auto player = dynamic_cast<Player*> (&object);
  set_group(COLGROUP_MOVING_STATIC);
  on_ground = false;
  if (player)
  {
    if (player->is_swimming() || player->is_water_jumping())
    {
      float swimangle = player->get_swimming_angle();
      physic.set_velocity(player->get_velocity() + Vector(std::cos(swimangle), std::sin(swimangle)));
    }
    else
    {
      physic.set_velocity_x(fabsf(player->get_physic().get_velocity_x()) < 1.f ? 0.f :
        player->m_dir == Direction::LEFT ? -200.f : 200.f);
      physic.set_velocity_y((dir == Direction::UP) ? -500.f : (dir == Direction::DOWN) ? 500.f :
        (glm::length(last_movement) > 1) ? -200.f : 0.f);
    }
  }

  running_grab_script = false;
  if (!on_ungrab_script.empty() && !running_ungrab_script)
  {
    running_ungrab_script = true;
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
