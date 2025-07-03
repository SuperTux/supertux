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

#include "object/floating_platform.hpp"
#include "badguy/crusher.hpp"
#include "object/player.hpp"
#include "object/rock.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "object/rainsplash.hpp"
#include "audio/sound_manager.hpp"

constexpr float GROUND_FRICTION = 0.1f;
const float sink_speed = 40.0f;
static const std::string SPLASH_SOUND = "sounds/splash.ogg";
FloatingPlatform::FloatingPlatform(const ReaderMapping& reader) :
  MovingSprite(reader, "images/objects/platforms/icefloe_small.sprite", LAYER_OBJECTS, COLGROUP_MOVING_STATIC),
  m_physic(),
  m_on_ground(false),
  m_at_ceiling(false),
  m_floating(false),
  m_last_sector_gravity(1.f),
  m_sink_offset(0),
  m_player_offset(0),
  m_player_on_platform(false)
{
  set_group(COLGROUP_MOVING_STATIC);
  SoundManager::current()->preload(SPLASH_SOUND);
}

void
FloatingPlatform::update(float dt_sec)
{
  if (get_bbox().get_top() > Sector::get().get_height())
    remove_me();

  Rectf water_check_bbox = get_bbox();
  water_check_bbox.set_bottom(water_check_bbox.get_bottom() - 30.0f); // Expand downward
  bool in_water = !Sector::get().is_free_of_tiles(water_check_bbox, true, Tile::WATER);
  if (in_water)
    m_floating = true;
  
  Vector movement;
  if (!m_floating) {
    movement = m_physic.get_movement(dt_sec) *
                  Vector(1.f, 1.f);
  } else{
    movement = m_physic.get_movement(dt_sec) *
                  Vector(1.f, in_water ? -0.05f : 0.01f);
  }
  
  if (m_sink_offset > 0) {
        set_pos(get_pos() + Vector(0, m_sink_offset * dt_sec));
  }
 
  const float sector_gravity = Sector::get().get_gravity();
  if (m_last_sector_gravity != sector_gravity) {
    if ((sector_gravity < 0.0f && m_last_sector_gravity >= 0.0f) ||
        (sector_gravity >= 0.0f && m_last_sector_gravity < 0.0f)) {
      m_on_ground = false;
      m_at_ceiling = false;
    }
    m_last_sector_gravity = sector_gravity;
  }
  m_col.set_movement(movement);
}

void
FloatingPlatform::collision_solid(const CollisionHit& hit)
{
  if (hit.top || hit.bottom) {
    m_physic.set_velocity_y(0);
  }

  if (hit.left || hit.right) {
    const float vel_x = m_physic.get_velocity_x();
    m_physic.set_velocity_x(-0.1f * vel_x);
  }

  if (hit.bottom) {
    if (!m_on_ground) {
      m_physic.set_velocity_x(0);
    }
    m_on_ground = true;
  }

  if (hit.top) {
    m_at_ceiling = true;
  }
}

HitResponse 
FloatingPlatform::collision(MovingObject& other, const CollisionHit& hit)
{
  auto crusher = dynamic_cast<Crusher*>(&other);
  if (crusher) {
    return FORCE_MOVE;
  }

  auto badguy = dynamic_cast<BadGuy*>(&other);
  if (badguy && hit.bottom && m_physic.get_velocity_y() > 200) {
    if (badguy->get_group() != COLGROUP_TOUCHABLE) {
      badguy->kill_fall();
      m_physic.set_velocity_y(0);
    }
  }

  Rectf rock_check_zone = get_bbox();
  rock_check_zone.set_bottom(rock_check_zone.get_top());
  rock_check_zone.set_top(rock_check_zone.get_top() - 100.0f);

  auto rock = dynamic_cast<Rock*>(&other);
  if (rock) {
    m_number_rocks = Sector::get().get_object_count<Rock>([this](const Rock& r) {
        Rectf rock_zone = get_bbox();
        rock_zone.set_bottom(rock_zone.get_top());
        rock_zone.set_top(rock_zone.get_top() - 100.0f);
        return rock_zone.overlaps(r.get_bbox());
    });
    const float max_sink_depth = 15.0f * static_cast<float>(m_number_rocks);
    
    m_sink_offset = std::min(m_sink_offset + sink_speed, max_sink_depth);
    m_physic.set_velocity_y(sink_speed * static_cast<float>(m_number_rocks)); 
  }

  Rectf player_check_zone = get_bbox();
  player_check_zone.set_bottom(player_check_zone.get_top());
  player_check_zone.set_top(player_check_zone.get_top() - 10.0f);

  auto player = dynamic_cast<Player*>(&other);
  if (player && player_check_zone.overlaps(other.get_bbox()) && !m_player_on_platform){
    m_player_on_platform = true;
    SoundManager::current()->play(SPLASH_SOUND, get_pos());
    const float sink_speed_player = 40.0f;
    const float max_sink_depth_player = 200.0f;
    const int num_splashes = 20;
    for (int i = 0; i < num_splashes; i++) {
        // Distribute evenly from left to right
        float x_pos = get_bbox().get_left() + (get_bbox().get_right() -get_bbox().get_left()) * 
                     (static_cast<float>(i) / (num_splashes - 1));
        
        Sector::get().add<RainSplash>(
            Vector(x_pos, get_bbox().get_bottom()-40.f),
            true
        );
    }
    m_player_offset = std::min(m_player_offset + sink_speed_player, max_sink_depth_player);
    m_physic.set_velocity_y(sink_speed_player); 
  } else if (!player_check_zone.overlaps(other.get_bbox())) {
      m_player_on_platform = false;
  }
  return FORCE_MOVE;
}

void FloatingPlatform::on_flip(float height)
{
  MovingSprite::on_flip(height);
}

ObjectSettings FloatingPlatform::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();
  return result;
}

/* EOF */
