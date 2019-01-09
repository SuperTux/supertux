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

#include "object/icecrusher.hpp"

#include <algorithm>
#include <math.h>

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "object/camera.hpp"
#include "object/particles.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"

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
  ic_size(NORMAL)
{
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
    case RECOVERING:
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
  if (player && hit.bottom) {
    SoundManager::current()->play("sounds/brick.wav");
    if (state == CRUSHING)
      set_state(RECOVERING);
    if (player->is_invincible()) {
      return ABORT_MOVE;
    }
    player->kill(false);
    return FORCE_MOVE;
  }
  auto badguy = dynamic_cast<BadGuy*>(&other);
  if (badguy) {
    badguy->kill_fall();
  }
  return FORCE_MOVE;
}

void
IceCrusher::collision_solid(const CollisionHit& hit)
{
  switch (state) {
    case RECOVERING:
    case IDLE:
      break;
    case CRUSHING:
      if (hit.bottom) {
        if (ic_size == LARGE) {
          cooldown_timer = PAUSE_TIME_LARGE;
          Sector::get().get_camera().shake (0.125f, 0.0f, 16.0f);
          SoundManager::current()->play("sounds/brick.wav");
          // throw some particles, bigger and more for large icecrusher
          for (int j = 0; j < 9; j++)
          {
            Sector::get().add<Particles>(
              Vector(m_col.m_bbox.get_right() - static_cast<float>(j) * 8.0f - 4.0f, m_col.m_bbox.get_bottom()),
              0, 90-5*j, 140, 380, Vector(0.0f, 300.0f),
              1, Color(.6f, .6f, .6f), 5, 1.8f, LAYER_OBJECTS+1);
            Sector::get().add<Particles>(
              Vector(m_col.m_bbox.get_left() + static_cast<float>(j) * 8.0f + 4.0f, m_col.m_bbox.get_bottom()),
              270+5*j, 360, 140, 380, Vector(0.0f, 300.0f),
              1, Color(.6f, .6f, .6f), 5, 1.8f, LAYER_OBJECTS+1);
          }
        }
        else {
          cooldown_timer = PAUSE_TIME_NORMAL;
          Sector::get().get_camera().shake (0.1f, 0.0, 8.0);
          if ( m_sprite_name.find("rock_crusher") != std::string::npos ||
              m_sprite_name.find("moss_crusher") != std::string::npos )
          {
            SoundManager::current()->play("sounds/thud.ogg");
          }
          else
          {
            SoundManager::current()->play("sounds/brick.wav");
          }
          // throw some particles
          for (int j = 0; j < 5; j++)
          {
            Sector::get().add<Particles>(
              Vector(m_col.m_bbox.get_right() - static_cast<float>(j) * 8.0f - 4.0f,
                     m_col.m_bbox.get_bottom()),
              0, 90+10*j, 140, 260, Vector(0, 300),
              1, Color(.6f, .6f, .6f), 4, 1.6f, LAYER_OBJECTS+1);
            Sector::get().add<Particles>(
              Vector(m_col.m_bbox.get_left() + static_cast<float>(j) * 8.0f + 4.0f,
                     m_col.m_bbox.get_bottom()),
              270+10*j, 360, 140, 260, Vector(0, 300),
              1, Color(.6f, .6f, .6f), 4, 1.6f, LAYER_OBJECTS+1);
          }
        }
        set_state(RECOVERING);
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
      m_col.m_movement = Vector (0, 0);
      if (found_victim())
        set_state(CRUSHING);
      break;
    case CRUSHING:
      m_col.m_movement = physic.get_movement (dt_sec);
      if (m_col.m_movement.y > MAX_DROP_SPEED)
        m_col.m_movement.y = MAX_DROP_SPEED;
      break;
    case RECOVERING:
      if (m_col.m_bbox.get_top() <= start_position.y+1) {
        set_pos(start_position);
        m_col.m_movement = Vector (0, 0);
        if (ic_size == LARGE)
          cooldown_timer = PAUSE_TIME_LARGE;
        else
          cooldown_timer = PAUSE_TIME_NORMAL;
        set_state(IDLE);
      }
      else {
        if (ic_size == LARGE)
          m_col.m_movement = Vector (0, RECOVER_SPEED_LARGE);
        else
          m_col.m_movement = Vector (0, RECOVER_SPEED_NORMAL);
      }
      break;
    default:
      log_debug << "IceCrusher in invalid state" << std::endl;
      break;
  }
}

void
IceCrusher::draw(DrawingContext& context)
{
  m_sprite->draw(context.color(), get_pos(), m_layer+2);
  if (!(state == CRUSHING) && m_sprite->has_action("whites"))
  {
    // draw icecrusher's eyes slightly behind
    lefteye->draw(context.color(), get_pos()+eye_position(false), m_layer+1);
    righteye->draw(context.color(), get_pos()+eye_position(true), m_layer+1);
    // draw the whites of icecrusher's eyes even further behind
    whites->draw(context.color(), get_pos(), m_layer);
  }
}

void
IceCrusher::after_editor_set() {
  MovingSprite::after_editor_set();
  after_sprite_set();
}

bool
IceCrusher::found_victim() const
{
  if (auto* player = Sector::get().get_nearest_player(m_col.m_bbox))
  {
    const Rectf& player_bbox = player->get_bbox();
    Rectf crush_area = Rectf(m_col.m_bbox.get_left()+1, m_col.m_bbox.get_bottom(),
                             m_col.m_bbox.get_right()-1, std::max(m_col.m_bbox.get_bottom(),player_bbox.get_top()-1));
    if ((player_bbox.get_top() >= m_col.m_bbox.get_bottom()) /* player is below crusher */
        && (player_bbox.get_right() > (m_col.m_bbox.get_left() - DROP_ACTIVATION_DISTANCE))
        && (player_bbox.get_left() < (m_col.m_bbox.get_right() + DROP_ACTIVATION_DISTANCE))
        && (Sector::get().is_free_of_statics(crush_area, this, false)) /* and area to player is free of objects */) {
      return true;
    }
  }

  return false;
}

Vector
IceCrusher::eye_position(bool right) const
{
  if (state == IDLE)
  {
    if (auto* player = Sector::get().get_nearest_player (m_col.m_bbox))
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
  else if (state == RECOVERING)
  {
    // Eyes spin while icecrusher is recovering, giving a dazed impression
    return Vector(sinf((right ? 1 : -1) * // X motion of each eye is opposite of the other
                       (get_pos().y/13 - // Phase factor due to y position
                        (ic_size==NORMAL ? RECOVER_SPEED_NORMAL : RECOVER_SPEED_LARGE) + cooldown_timer * 13.0f)) * //Phase factor due to cooldown timer
                  static_cast<float>(m_sprite->get_width()) / 64.0f * 2.0f - (right ? 1 : -1) * // Amplitude dependent on size
                  static_cast<float>(m_sprite->get_width()) / 64.0f * 2.0f, // Offset to keep eyes visible

                  cosf((right ? 3.1415f : 0.0f) + // Eyes spin out of phase of eachother
                       get_pos().y / 13.0f - // Phase factor due to y position
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

/* EOF */
