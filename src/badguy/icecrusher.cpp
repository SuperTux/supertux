//  IceCrusher - A block to stand on, which can drop down to crush the player
//  Copyright (C) 2008 Christoph Sommer <christoph.sommer@2008.expires.deltadevelopment.de>
//  Copyright (C) 2010 Florian Forster <supertux at octo.it>
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

#include "badguy/icecrusher.hpp"

#include <algorithm>
#include <math.h>
#include <string>

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "object/coin.hpp"
#include "object/camera.hpp"
#include "object/particles.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"

namespace {
/* Maximum movement speed in pixels per LOGICAL_FPS */
const float MAX_DROP_SPEED = 10.0f;
const float RECOVER_SPEED_NORMAL = -3.125f;
const float RECOVER_SPEED_LARGE  = -2.0f;
const float DROP_ACTIVATION_DISTANCE = 4.0f;
const float PAUSE_TIME_NORMAL = 0.5f;
const float PAUSE_TIME_LARGE  = 1.0f;
}

IceCrusher::IceCrusher(const ReaderMapping& reader) :
  MovingSprite(reader, "images/creatures/icecrusher/icecrusher.sprite", LAYER_OBJECTS, COLGROUP_STATIC),
  state(IDLE),
  start_position(m_col.m_bbox.p1()),
  physic(),
  cooldown_timer(0.0),
  lefteye(),
  righteye(),
  whites(),
  ic_size(NORMAL),
  sideways()
{
  reader.get("sideways", sideways);
  // TODO: icecrusher hitting deserves its own sounds-
  // one for hitting the ground, one for hitting Tux
  if ( m_sprite_name.find("rock_crusher") != std::string::npos ||
      m_sprite_name.find("moss_crusher") != std::string::npos )
  {
    SoundManager::current()->preload("sounds/thud.ogg");
  }
  else
  {
    SoundManager::current()->preload("sounds/brick.wav");
  }

  set_state(state, true);
  after_sprite_set();
}

bool
IceCrusher::is_sideways() const
{
  return sideways;
}

ObjectSettings
IceCrusher::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();

  result.add_bool(_("Sideways"), &sideways, "sideways", false);

  result.reorder({"sideways", "sprite", "x", "y"});

  return result;
}

void
IceCrusher::set_state(IceCrusherState state_, bool force)
{
  if ((state == state_) && (!force)) return;
  switch (state_) {
    case IDLE:
      set_group(COLGROUP_STATIC);
      physic.enable_gravity (false);
      m_sprite->set_action("idle");
      break;
    case CRUSHING:
      set_group(COLGROUP_MOVING_STATIC);
      physic.reset ();
      physic.enable_gravity (true);
      m_sprite->set_action("crushing");
      break;
    case CRUSHING_UP:
      set_group(COLGROUP_MOVING_STATIC);
      physic.reset ();
      physic.enable_gravity (false);
      m_sprite->set_action("crushing");
      break;
    case CRUSHING_RIGHT:
      set_group(COLGROUP_MOVING_STATIC);
      physic.reset ();
      physic.enable_gravity (false);
      m_sprite->set_action("idle");
      break;
    case CRUSHING_LEFT:
      set_group(COLGROUP_MOVING_STATIC);
      physic.reset ();
      physic.enable_gravity (false);
      m_sprite->set_action("idle");
      break;
    case RECOVERING:
    case RECOVERING_UP:
      set_group(COLGROUP_MOVING_STATIC);
      physic.enable_gravity (false);
      m_sprite->set_action("recovering");
      break;
    case RECOVERING_RIGHT:
      set_group(COLGROUP_MOVING_STATIC);
      physic.enable_gravity (false);
      m_sprite->set_action("recovering");
      break;
    case RECOVERING_LEFT:
      set_group(COLGROUP_MOVING_STATIC);
      physic.enable_gravity (false);
      m_sprite->set_action("recovering");
      break;
    default:
      log_debug << "IceCrusher in invalid state" << std::endl;
      break;
  }
  state = state_;
}

HitResponse
IceCrusher::collision(GameObject& other, const CollisionHit& hit)
{
  auto player = dynamic_cast<Player*>(&other);

  // If the other object is the player, and the collision is at the
  // bottom of the ice crusher, hurt the player.
  if (player && hit.bottom && state == CRUSHING) {
    SoundManager::current()->play("sounds/brick.wav", get_pos());
    set_state(RECOVERING);
    if (player->is_invincible()) {
      return ABORT_MOVE;
    }
    player->kill(false);
    return FORCE_MOVE;
  }
  if (player && hit.top && state == CRUSHING_UP)
    return FORCE_MOVE;
  auto badguy = dynamic_cast<BadGuy*>(&other);
  if (badguy && (state == CRUSHING || state == CRUSHING_LEFT || state == CRUSHING_RIGHT)) {
    badguy->kill_fall();
  }

  auto heavy_coin = dynamic_cast<HeavyCoin*>(&other);
  if(heavy_coin) {
    return ABORT_MOVE;
  }
  return FORCE_MOVE;
}

void
IceCrusher::collision_solid(const CollisionHit& hit)
{
  switch (state) {
    case RECOVERING:
    case RECOVERING_UP:
    case IDLE:
      break;
    case CRUSHING:
    case CRUSHING_UP:
      if ((state == CRUSHING && hit.bottom) || (state == CRUSHING_UP && hit.top)) {
        if (ic_size == LARGE) {
          cooldown_timer = PAUSE_TIME_LARGE;
          Sector::get().get_camera().shake (0.125f, 0.0f, 16.0f);
          SoundManager::current()->play("sounds/brick.wav", get_pos());
          // throw some particles, bigger and more for large icecrusher
          for (int j = 0; j < 9; j++)
          {
            Sector::get().add<Particles>(
              Vector(m_col.m_bbox.get_right() - static_cast<float>(j) * 8.0f - 4.0f,
              (state == CRUSHING ? m_col.m_bbox.get_bottom() : m_col.m_bbox.get_top())),
              0, 90-5*j, 140.0f, 380.0f, Vector(0.0f, 300.0f),
              1, Color(.6f, .6f, .6f), 5, 1.8f, LAYER_OBJECTS+1);
            Sector::get().add<Particles>(
              Vector(m_col.m_bbox.get_left() + static_cast<float>(j) * 8.0f + 4.0f,
              (state == CRUSHING ? m_col.m_bbox.get_bottom() : m_col.m_bbox.get_top())),
              270+5*j, 360, 140.0f, 380.0f, Vector(0.0f, 300.0f),
              1, Color(.6f, .6f, .6f), 5, 1.8f, LAYER_OBJECTS+1);
          }
        }
        else {
          cooldown_timer = PAUSE_TIME_NORMAL;
          Sector::get().get_camera().shake (0.1f, 0.0, 8.0);
          if ( m_sprite_name.find("rock_crusher") != std::string::npos ||
              m_sprite_name.find("moss_crusher") != std::string::npos )
          {
            SoundManager::current()->play("sounds/thud.ogg", get_pos());
          }
          else
          {
            SoundManager::current()->play("sounds/brick.wav", get_pos());
          }
          // throw some particles
          for (int j = 0; j < 5; j++)
          {
            Sector::get().add<Particles>(
              Vector(m_col.m_bbox.get_right() - static_cast<float>(j) * 8.0f - 4.0f,
                     (state == CRUSHING ? m_col.m_bbox.get_bottom() : m_col.m_bbox.get_top())),
              0, 90+10*j, 140, 260, Vector(0, 300),
              1, Color(.6f, .6f, .6f), 4, 1.6f, LAYER_OBJECTS+1);
            Sector::get().add<Particles>(
              Vector(m_col.m_bbox.get_left() + static_cast<float>(j) * 8.0f + 4.0f,
                     m_col.m_bbox.get_bottom()),
              270+10*j, 360, 140, 260, Vector(0, 300),
              1, Color(.6f, .6f, .6f), 4, 1.6f, LAYER_OBJECTS+1);
          }
        }
        set_state(state == CRUSHING ? RECOVERING : RECOVERING_UP);
        spawn_roots(Direction::DOWN);
      }
      break;
    case CRUSHING_RIGHT:
      if (hit.right)
      {
        if (ic_size == LARGE)
        {
          cooldown_timer = PAUSE_TIME_LARGE;
          Sector::get().get_camera().shake (0.125f, 0.0f, 16.0f);
          SoundManager::current()->play("sounds/brick.wav", get_pos());
        }
        else
        {
          cooldown_timer = PAUSE_TIME_NORMAL;
          Sector::get().get_camera().shake (0.1f, 0.0, 8.0);
          if ( m_sprite_name.find("rock_crusher") != std::string::npos ||
              m_sprite_name.find("moss_crusher") != std::string::npos )
          {
            SoundManager::current()->play("sounds/thud.ogg", get_pos());
          }
          else
          {
            SoundManager::current()->play("sounds/brick.wav", get_pos());
          }
        }
        spawn_roots(Direction::RIGHT);
        set_state(RECOVERING_RIGHT);
      }
      break;
    case CRUSHING_LEFT:
      if (hit.left)
      {
        if (ic_size == LARGE)
        {
          cooldown_timer = PAUSE_TIME_LARGE;
          Sector::get().get_camera().shake (0.125f, 0.0f, 16.0f);
          SoundManager::current()->play("sounds/brick.wav", get_pos());
        }
        else
        {
          cooldown_timer = PAUSE_TIME_NORMAL;
          Sector::get().get_camera().shake (0.1f, 0.0, 8.0);
          if ( m_sprite_name.find("rock_crusher") != std::string::npos ||
              m_sprite_name.find("moss_crusher") != std::string::npos )
          {
            SoundManager::current()->play("sounds/thud.ogg", get_pos());
          }
          else
          {
            SoundManager::current()->play("sounds/brick.wav", get_pos());
          }
        }
        set_state(RECOVERING_LEFT);
        spawn_roots(Direction::LEFT);
      }
      break;
    default:
      log_debug << "IceCrusher in invalid state" << std::endl;
      break;
  }
}

void
IceCrusher::update(float dt_sec)
{
  
  if (cooldown_timer >= dt_sec)
  {
    cooldown_timer -= dt_sec;
    return;
  }
  else if (cooldown_timer != 0.0f)
  {
    dt_sec -= cooldown_timer;
    cooldown_timer = 0.0;
  }

  switch (state) {
    case IDLE:
      m_col.set_movement(Vector (0, 0));
      if (found_victim_up() && !sideways && m_flip != NO_FLIP)
        set_state(CRUSHING_UP);
      else if (found_victim_down() && !sideways && m_flip == NO_FLIP)
        set_state(CRUSHING);
      else if (found_victim_right() && sideways)
        set_state(CRUSHING_RIGHT);
      else if (found_victim_left() && sideways)
        set_state(CRUSHING_LEFT);
      break;
    case CRUSHING_UP:
      {
        Vector movement = physic.get_movement(dt_sec);
        m_col.set_movement(movement);
        m_col.propagate_movement(movement);
        physic.set_velocity_y((physic.get_velocity_y() - 10.f));
      }
      break;
    case CRUSHING:
      {
        Vector movement = physic.get_movement(dt_sec);
        if (movement.y > MAX_DROP_SPEED)
          movement.y = MAX_DROP_SPEED;
        m_col.set_movement(movement);
        m_col.propagate_movement(movement);
      }
      break;
    case CRUSHING_RIGHT:
      {
        Vector movement = physic.get_movement(dt_sec);
        m_col.set_movement(movement);
        m_col.propagate_movement(movement);
        physic.set_velocity_x((physic.get_velocity_x() + 10.f));
      }
      break;
    case CRUSHING_LEFT:
      {
        Vector movement = physic.get_movement(dt_sec);
        m_col.set_movement(movement);
        m_col.propagate_movement(movement);
        physic.set_velocity_x((physic.get_velocity_x() - 10.f));
      }
      break;
    case RECOVERING:
      if (m_col.m_bbox.get_top() <= start_position.y+1)
      {
        set_pos(start_position);
        m_col.set_movement(Vector (0, 0));
        if (ic_size == LARGE)
          cooldown_timer = PAUSE_TIME_LARGE;
        else
          cooldown_timer = PAUSE_TIME_NORMAL;
        set_state(IDLE);
      }
      else
      {
        Vector movement(0, (ic_size == LARGE ? RECOVER_SPEED_LARGE : RECOVER_SPEED_NORMAL));
        m_col.set_movement(movement);
        m_col.propagate_movement(movement);
      }
      break;
    case RECOVERING_UP:
      if (m_col.m_bbox.get_top() >= start_position.y-1)
      {
        set_pos(start_position);
        m_col.set_movement(Vector (0, 0));
        if (ic_size == LARGE)
          cooldown_timer = PAUSE_TIME_LARGE;
        else
          cooldown_timer = PAUSE_TIME_NORMAL;
        set_state(IDLE);
      }
      else
      {
        Vector movement(0, (ic_size == LARGE ? -RECOVER_SPEED_LARGE : -RECOVER_SPEED_NORMAL));
        m_col.set_movement(movement);
        m_col.propagate_movement(movement);
      }
      break;
    case RECOVERING_RIGHT:
      if (m_col.m_bbox.get_left() <= start_position.x+1)
      {
        set_pos(start_position);
        m_col.set_movement(Vector (0, 0));
        if (ic_size == LARGE)
          cooldown_timer = PAUSE_TIME_LARGE;
        else
          cooldown_timer = PAUSE_TIME_NORMAL;
        set_state(IDLE);
      }
      else
      {
        m_col.set_movement(Vector(RECOVER_SPEED_LARGE, 0));
        m_col.propagate_movement(Vector(RECOVER_SPEED_LARGE, 0));
      }
      break;
    case RECOVERING_LEFT:
      if (m_col.m_bbox.get_left() >= start_position.x-1)
      {
        set_pos(start_position);
        m_col.set_movement(Vector (0, 0));
        if (ic_size == LARGE)
          cooldown_timer = PAUSE_TIME_LARGE;
        else
          cooldown_timer = PAUSE_TIME_NORMAL;
        set_state(IDLE);
      }
      else
      {
        m_col.set_movement(Vector(-RECOVER_SPEED_LARGE, 0));
        m_col.propagate_movement(Vector(-RECOVER_SPEED_LARGE, 0));
      }
      break;
    default:
      log_debug << "IceCrusher in invalid state" << std::endl;
      break;
  }
}

void
IceCrusher::spawn_roots(Direction direction)
{
  if (m_sprite_name.find("root_crusher") == std::string::npos)
    return;

  Vector origin;
  Rectf test_solid_offset_1, test_solid_offset_2, test_empty_offset;
  bool vertical = false;

  switch(direction)
  {
    case Direction::DOWN:
      vertical = true;
      origin.x = m_col.m_bbox.get_middle().x - 16.f;
      origin.y = m_col.m_bbox.get_bottom();
      test_empty_offset = Rectf(Vector(4, -4), Size(16, 1));
      test_solid_offset_1 = Rectf(Vector(6, 8), Size(1, 1));
      test_solid_offset_2 = Rectf(Vector(16, 8), Size(1, 1));
      break;

    case Direction::LEFT:
      origin.x = m_col.m_bbox.get_left() - 6.f;
      origin.y = m_col.m_bbox.get_middle().y - 16.f;
      test_empty_offset = Rectf(Vector(8, 0), Size(1, 16));
      test_solid_offset_1 = Rectf(Vector(0, 4), Size(1, 1));
      test_solid_offset_2 = Rectf(Vector(0, 12), Size(1, 1));
      break;

    case Direction::RIGHT:
      origin.x = m_col.m_bbox.get_right() + 12.f;
      origin.y = m_col.m_bbox.get_middle().y - 16.f;
      test_empty_offset = Rectf(Vector(-16, 0), Size(1, 16));
      test_solid_offset_1 = Rectf(Vector(0, 4), Size(1, 1));
      test_solid_offset_2 = Rectf(Vector(0, 12), Size(1, 1));
      break;
  }

  for (float dir = -1.f; dir <= 1.f; dir += 2.f)
  {
    for (float step = 0.f; step < 3.f; step++)
    {
      Vector pos = origin;
      float dist = 32.f * step - 15.f;
      (vertical ? pos.x : pos.y) += dir * (dist + (vertical ? m_col.m_bbox.get_width() : m_col.m_bbox.get_height()));

      bool solid_1 = Sector::current()->is_free_of_tiles(test_solid_offset_1.moved(pos));
      bool solid_2 = Sector::current()->is_free_of_tiles(test_solid_offset_2.moved(pos));
      bool empty = Sector::current()->is_free_of_tiles(test_empty_offset.moved(pos));

      printf("Empty %d, solid1 %d, solid2 %d\n", empty, solid_1, solid_2);
      if (!empty || solid_1 || solid_2)
        break;

      Sector::current()->add<CrusherRoot>(pos, direction, step * .1f, m_layer);
    }
  }
}

void
IceCrusher::draw(DrawingContext& context)
{
  m_sprite->draw(context.color(), get_pos(), m_layer+2, m_flip);
  if (!(state == CRUSHING) && m_sprite->has_action("whites"))
  {
    // draw icecrusher's eyes slightly behind
    lefteye->draw(context.color(), get_pos()+eye_position(false), m_layer+1, m_flip);
    righteye->draw(context.color(), get_pos()+eye_position(true), m_layer+1, m_flip);

    // draw the whites of icecrusher's eyes even further behind
    whites->draw(context.color(), get_pos(), m_layer, m_flip);
  }
}

void
IceCrusher::after_editor_set() {
  MovingSprite::after_editor_set();
  after_sprite_set();
}

bool
IceCrusher::found_victim_down() const
{
  for (auto* player : Sector::get().get_players())
  {
    const Rectf& player_bbox = player->get_bbox();
    Rectf crush_area_down = Rectf(m_col.m_bbox.get_left()+1, m_col.m_bbox.get_bottom(),
                             m_col.m_bbox.get_right()-1, std::max(m_col.m_bbox.get_bottom(),player_bbox.get_top()-1));
    if ((player_bbox.get_top() >= m_col.m_bbox.get_bottom()) /* player is below crusher */
        && (player_bbox.get_right() > (m_col.m_bbox.get_left() - DROP_ACTIVATION_DISTANCE))
        && (player_bbox.get_left() < (m_col.m_bbox.get_right() + DROP_ACTIVATION_DISTANCE))
        && (Sector::get().is_free_of_statics(crush_area_down, this, false)) /* and area to player is free of objects */) {
      return true;
    }
  }

  return false;
}

bool
IceCrusher::found_victim_up() const
{
  for (auto* player : Sector::get().get_players())
  {
    const Rectf& player_bbox = player->get_bbox();
    Rectf crush_area_up = Rectf(m_col.m_bbox.get_left()+1, m_col.m_bbox.get_top(),
                             m_col.m_bbox.get_right()-1, std::max(m_col.m_bbox.get_top(),player_bbox.get_bottom()+1));
    if ((player_bbox.get_bottom() <= m_col.m_bbox.get_top()) /* player is above crusher */
        && (player_bbox.get_right() > (m_col.m_bbox.get_left() - DROP_ACTIVATION_DISTANCE))
        && (player_bbox.get_left() < (m_col.m_bbox.get_right() + DROP_ACTIVATION_DISTANCE))
        && (Sector::get().is_free_of_statics(crush_area_up, this, false)) /* and area to player is free of objects */) {
      return true;
    }
  }

  return false;
}

bool
IceCrusher::found_victim_right() const
{
  for (auto* player : Sector::get().get_players())
  {
    const Rectf& player_bbox = player->get_bbox();
    Rectf crush_area_right = get_bbox();
    crush_area_right.set_right(player_bbox.get_left() - 1);
    if (((player_bbox.get_left() + 64) >= m_col.m_bbox.get_right())
        && (player_bbox.get_bottom() + 5 > (m_col.m_bbox.get_top() - DROP_ACTIVATION_DISTANCE))
        && (player_bbox.get_top() < (m_col.m_bbox.get_bottom() + DROP_ACTIVATION_DISTANCE))
        && (Sector::get().is_free_of_statics(crush_area_right, this, false)) /* and area to player is free of objects */) {
      return true;
    }
  }

  return false;
}

bool
IceCrusher::found_victim_left() const
{
  for (auto* player : Sector::get().get_players())
  {
    const Rectf& player_bbox = player->get_bbox();
    Rectf crush_area_left = get_bbox();
    crush_area_left.set_left(player_bbox.get_right() + 1);
    if (((player_bbox.get_right() - 64) <= m_col.m_bbox.get_left())
        && (player_bbox.get_bottom() + 5 > (m_col.m_bbox.get_top() - DROP_ACTIVATION_DISTANCE))
        && (player_bbox.get_top() < (m_col.m_bbox.get_bottom() + DROP_ACTIVATION_DISTANCE))
        && (Sector::get().is_free_of_statics(crush_area_left, this, false)) /* and area to player is free of objects */) {
      return true;
    }
  }

  return false;
}

Vector
IceCrusher::eye_position(bool right) const
{
  if (state == IDLE || state == CRUSHING_RIGHT || state == CRUSHING_LEFT)
  {
    if (auto* player = Sector::get().get_nearest_player(m_col.m_bbox))
    {
      // Icecrusher focuses on approximate position of player's head
      const float player_focus_x = (player->get_bbox().get_right() + player->get_bbox().get_left()) * 0.5f;
      const float player_focus_y = player->get_bbox().get_bottom() * 0.25f + player->get_bbox().get_top() * 0.75f;
      // Icecrusher's approximate origin of line-of-sight
      const float crusher_origin_x = m_col.m_bbox.get_middle().x;
      const float crusher_origin_y = m_col.m_bbox.get_middle().y;
      // Line-of-sight displacement from icecrusher to player
      const float displacement_x = player_focus_x - crusher_origin_x;
      const float displacement_y = player_focus_y - crusher_origin_y;
      const float displacement_mag = powf(powf(displacement_x, 2.0f) + powf(displacement_y, 2.0f), 0.5f);
      // Determine weighting for eye displacement along x given icecrusher eye shape
      int weight_x = m_sprite->get_width()/64 * (((displacement_x > 0) == right) ? 1 : 4);
      int weight_y = m_sprite->get_width()/64 * 2;

      return Vector(displacement_x / displacement_mag * static_cast<float>(weight_x),
                    displacement_y / displacement_mag * static_cast<float>(weight_y) - static_cast<float>(weight_y));
    }
  }
  else if (state != IDLE && state != CRUSHING_RIGHT && state != CRUSHING_LEFT)
  {
    // Eyes spin while icecrusher is recovering, giving a dazed impression
    return Vector(sinf((right ? 1 : -1) * // X motion of each eye is opposite of the other
                  (((state == RECOVERING || state == RECOVERING_UP) ? get_pos().y / 13 : get_pos().x / 13) - // Phase factor due to y position
                       (ic_size==NORMAL ? RECOVER_SPEED_NORMAL : RECOVER_SPEED_LARGE) + cooldown_timer * 13.0f)) * //Phase factor due to cooldown timer
                  static_cast<float>(m_sprite->get_width()) / 64.0f * 2.0f - (right ? 1 : -1) * // Amplitude dependent on size
                  static_cast<float>(m_sprite->get_width()) / 64.0f * 2.0f, // Offset to keep eyes visible

                  cosf((right ? 3.1415f : 0.0f) + // Eyes spin out of phase of eachother
                  ((state == RECOVERING || state == RECOVERING_UP) ? get_pos().y / 13 : get_pos().x / 13) - // Phase factor due to y position
                       (ic_size==NORMAL ? RECOVER_SPEED_NORMAL : RECOVER_SPEED_LARGE) + cooldown_timer * 13.0f) * //Phase factor due to cooldown timer
                  static_cast<float>(m_sprite->get_width()) / 64.0f * 2.0f -  // Amplitude dependent on size
                  static_cast<float>(m_sprite->get_width()) / 64.0f * 2.0f); // Offset to keep eyes visible
  }

  return Vector(0,0);
}

void
IceCrusher::after_sprite_set()
{
  float sprite_width = static_cast<float>(m_sprite->get_width());
  if (sprite_width >= 128.0f)
    ic_size = LARGE;

  if (!m_sprite->has_action("whites"))
  {
    lefteye.reset();
    righteye.reset();
    whites.reset();
  }
  else
  {
    lefteye = m_sprite->clone();
    lefteye->set_action("lefteye");
    righteye = m_sprite->clone();
    righteye->set_action("righteye");
    whites = m_sprite->clone();
    whites->set_action("whites");
  }
}

void
IceCrusher::on_flip(float height)
{
  MovingSprite::on_flip(height);
  start_position.y = height - m_col.m_bbox.get_height() - start_position.y;
  FlipLevelTransformer::transform_flip(m_flip);
}

CrusherRoot::CrusherRoot(Vector position, IceCrusher::Direction direction, float delay, int layer) :
  MovingSprite(position, direction == IceCrusher::Direction::DOWN ?
                  "images/creatures/icecrusher/roots/crusher_root.sprite" :
                  "images/creatures/icecrusher/roots/crusher_root_side.sprite"),
  m_original_pos(position),
  m_direction(direction),
  m_delay_remaining(delay)
{
  m_layer = layer;

  if (delay_gone())
  {
    start_animation();
  }
  else
  {
    m_col.m_group = COLGROUP_DISABLED;
  }
}

HitResponse
CrusherRoot::collision(GameObject& other, const CollisionHit& hit)
{
  if (delay_gone())
  {
    auto player = dynamic_cast<Player*>(&other);

    if (player)
      player->kill(false);
  }

  return ABORT_MOVE;
}

void
CrusherRoot::update(float dt_sec)
{
  if (m_delay_remaining > 0.f)
  {
    m_delay_remaining -= dt_sec;

    if (delay_gone())
    {
      start_animation();
    }
    else
    {
      return;
    }
  }
  else if (m_sprite->animation_done())
  {
    remove_me();
    return;
  }

  switch(m_direction)
  {
    case IceCrusher::Direction::DOWN:
      m_col.move_to(m_original_pos + Vector(0, -m_sprite->get_current_hitbox_height()));
      break;

    case IceCrusher::Direction::LEFT:
      m_col.move_to(m_original_pos);
      break;

    case IceCrusher::Direction::RIGHT:
      m_col.move_to(m_original_pos + Vector(-m_sprite->get_current_hitbox_width(), 0));
      break;
  }
}

void
CrusherRoot::start_animation()
{
  m_col.m_group = COLGROUP_TOUCHABLE;

  switch(m_direction)
  {
    case IceCrusher::Direction::DOWN:
      m_sprite->set_action("downwards");
      m_sprite->set_animation_loops(1);
      break;

    case IceCrusher::Direction::LEFT:
      m_sprite->set_action("sideways-left");
      m_sprite->set_animation_loops(1);
      break;

    case IceCrusher::Direction::RIGHT:
      m_sprite->set_action("sideways-right");
      m_sprite->set_animation_loops(1);
      break;
  }
}

/* EOF */
