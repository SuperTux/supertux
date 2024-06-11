//  SuperTux - Boss "Yeti"
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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

#include "badguy/yeti.hpp"

#include <math.h>

#include "audio/sound_manager.hpp"
#include "badguy/yeti_stalactite.hpp"
#include "math/random.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "video/surface.hpp"

namespace
{
const float JUMP_DOWN_VX = 250; /**< Horizontal speed while jumping off the dais. */
const float JUMP_DOWN_VY = -250; /**< Vertical speed while jumping off the dais. */

const float RUN_VX = 350; /**< Horizontal speed while running. */

const float JUMP_UP_VX = 350; /**< Horizontal speed while jumping on the dais. */
const float JUMP_UP_VY = -700; /**< Vertical speed while jumping on the dais. */

const float STOMP_VY = -300; /**< Vertical speed while stomping on the dais. */

const float RUN_DISTANCE = 1060; /**< Distance between the x-coordinates of left and right end positions. */
const float JUMP_SPACE = 448; /**< Distance between the jump position and the stand position. */
const float STOMP_WAIT = 0.5; /**< Time we stay on the dais before jumping again. */
const float SAFE_TIME = 0.5; /**< The time we are safe when Tux just hit us. */

const float YETI_SQUISH_TIME = 3;

const float SNOW_EXPLOSIONS_FREQUENCY = 8; /**< Number of snowball explosions per second. */
const int SNOW_EXPLOSIONS_COUNT = 5; /**< Number of snowballs per explosion. */
const float SNOW_EXPLOSIONS_VX = 150; /**< Speed of snowballs. */
const float SNOW_EXPLOSIONS_VY = -200; /**< Speed of snowballs. */
}

Yeti::Yeti(const ReaderMapping& reader) :
  Boss(reader, "images/creatures/yeti/yeti.sprite"),
  m_state(),
  m_state_timer(),
  m_safe_timer(),
  m_stomp_count(),
  m_left_stand_x(),
  m_right_stand_x(),
  m_left_jump_x(),
  m_right_jump_x(),
  m_fixed_pos()
{
  reader.get("hud-icon", m_hud_icon, "images/creatures/yeti/hudlife.png");
  m_hud_head = Surface::from_file(m_hud_icon);
  SoundManager::current()->preload("sounds/yeti_gna.wav");
  SoundManager::current()->preload("sounds/yeti_roar.wav");

  initialize();

  reader.get("fixed-pos", m_fixed_pos, false);
  if (m_fixed_pos) {
    m_left_stand_x = 80;
    m_right_stand_x = 1140;
    m_left_jump_x = 528;
    m_right_jump_x = 692;
  } else {
    recalculate_pos();
  }
}

void
Yeti::initialize()
{
  m_dir = Direction::RIGHT;
  jump_down();
}

void
Yeti::recalculate_pos()
{
  if (m_dir == Direction::RIGHT) {
    m_left_stand_x = m_col.m_bbox.get_left();
    m_right_stand_x = m_left_stand_x + RUN_DISTANCE;
  } else {
    m_right_stand_x = m_col.m_bbox.get_left();
    m_left_stand_x = m_right_stand_x - RUN_DISTANCE;
  }

  m_left_jump_x = m_left_stand_x + JUMP_SPACE;
  m_right_jump_x = m_right_stand_x - JUMP_SPACE;
}

void
Yeti::draw(DrawingContext& context)
{
  // We blink when we are safe.
  if (m_safe_timer.started() && size_t(g_game_time * 40) % 2)
    return;

  Boss::draw(context);
}

void
Yeti::active_update(float dt_sec)
{
  Boss::boss_update(dt_sec);

  switch (m_state) {
    case JUMP_DOWN:
      m_physic.set_velocity_x((m_dir==Direction::RIGHT)?+JUMP_DOWN_VX:-JUMP_DOWN_VX);
      break;
    case RUN:
      m_physic.set_velocity_x((m_dir==Direction::RIGHT)?+RUN_VX:-RUN_VX);
      if (((m_dir == Direction::RIGHT) && (get_pos().x >= m_right_jump_x)) || ((m_dir == Direction::LEFT) && (get_pos().x <= m_left_jump_x))) jump_up();
      break;
    case JUMP_UP:
      m_physic.set_velocity_x((m_dir==Direction::RIGHT)?+JUMP_UP_VX:-JUMP_UP_VX);
      if (((m_dir == Direction::RIGHT) && (get_pos().x >= m_right_stand_x)) || ((m_dir == Direction::LEFT) && (get_pos().x <= m_left_stand_x))) be_angry();
      break;
    case BE_ANGRY:
      if (m_state_timer.check() && on_ground()) {
        m_physic.set_velocity_y(STOMP_VY);
        set_action("stomp", m_dir);
        SoundManager::current()->play("sounds/yeti_gna.wav", get_pos());
      }
      break;
    case SQUISHED:
      {
        Direction newdir = (int(m_state_timer.get_timeleft() * SNOW_EXPLOSIONS_FREQUENCY) % 2) ? Direction::LEFT : Direction::RIGHT;
        if (m_dir != newdir && m_dir == Direction::RIGHT) {
          SoundManager::current()->play("sounds/stomp.wav", get_pos());
          add_snow_explosions();
          Sector::get().get_camera().shake(.05f, 0, 5);
        }
        m_dir = newdir;
        set_action("jump", m_dir);
      }
      if (m_state_timer.check()) {
        BadGuy::kill_fall();
        m_state = FALLING;
        m_physic.set_velocity_y(JUMP_UP_VY / 2); // Move up a bit before falling
        // Add some extra explosions.
        for (int i = 0; i < 10; i++) {
          add_snow_explosions();
        }
        run_dead_script();
      }
      break;
    case FALLING:
      break;
  }

  m_col.set_movement(m_physic.get_movement(dt_sec));
}

void
Yeti::jump_down()
{
  set_action("jump", m_dir);
  m_physic.set_velocity(m_dir == Direction::RIGHT ? JUMP_DOWN_VX : -JUMP_DOWN_VX, JUMP_DOWN_VY);
  m_state = JUMP_DOWN;
}

void
Yeti::run()
{
  set_action("walking", m_dir);
  m_physic.set_velocity(m_dir == Direction::RIGHT ? RUN_VX : -RUN_VX, 0);
  m_state = RUN;
}

void
Yeti::jump_up()
{
  set_action("jump", m_dir);
  m_physic.set_velocity(m_dir == Direction::RIGHT ? JUMP_UP_VX : -JUMP_UP_VX, JUMP_UP_VY);
  m_state = JUMP_UP;
}

void
Yeti::be_angry()
{
  // Turn around.
  m_dir = (m_dir==Direction::RIGHT) ? Direction::LEFT : Direction::RIGHT;

  set_action("stand", m_dir);
  m_physic.set_velocity_x(0);
  m_stomp_count = 0;
  m_state = BE_ANGRY;
  m_state_timer.start(STOMP_WAIT);
}

bool
Yeti::collision_squished(GameObject& object)
{
  kill_squished(object);

  return true;
}

void
Yeti::kill_squished(GameObject& object)
{
  auto player = dynamic_cast<Player*>(&object);
  if (player) {
    player->bounce(*this);
    take_hit(*player);
  }
}

void Yeti::take_hit(Player& )
{
  if (m_safe_timer.started())
    return;

  SoundManager::current()->play("sounds/yeti_roar.wav", get_pos());
  m_lives--;

  if (m_lives <= 0) {
    // We're dead.
    m_physic.set_velocity((m_dir == Direction::RIGHT ? RUN_VX : -RUN_VX) / 5, 0);

    // Set the badguy layer to be above the foremost, so that
    // this does not reveal secret tilemaps:
    m_layer = Sector::get().get_foremost_opaque_layer() + 1;
    m_state = SQUISHED;
    m_state_timer.start(YETI_SQUISH_TIME);
    set_colgroup_active(COLGROUP_MOVING_ONLY_STATIC);
    // sprite->setAction("dead");
  }
  else {
    m_safe_timer.start(SAFE_TIME);
  }
}

void
Yeti::kill_fall()
{
  // Shooting bullets or being invincible won't prevent this action.
}

void
Yeti::drop_stalactite()
{
  // Make a stalactite fall down and shake the camera a bit.
  Sector::get().get_camera().shake(.1f, 0, 20.f);

  auto player = get_nearest_player();
  if (!player) return;

  for (auto& stalactite : Sector::get().get_objects_by_type<YetiStalactite>())
  {
    if (stalactite.is_hanging()) {
      if (!m_pinch_mode) {
        // Drop stalactites within 3 units of player, going out with each jump.
        float distancex = fabsf(stalactite.get_bbox().get_middle().x - player->get_bbox().get_middle().x);
        if (distancex < static_cast<float>(m_stomp_count) * 32.0f) {
          stalactite.start_shaking();
        }
      }
      else { /* if (hitpoints < 3) */
        // Drop every 3rd pair of stalactites.
        if ((((static_cast<int>(stalactite.get_pos().x) + 16) / 64) % 3) == (m_stomp_count % 3))
          stalactite.start_shaking();
      }
    }
  }
}

void
Yeti::collision_solid(const CollisionHit& hit)
{
  update_on_ground_flag(hit);
  if (hit.top || hit.bottom) {
    // Hit floor or roof.
    m_physic.set_velocity_y(0);
    switch (m_state) {
      case JUMP_DOWN:
        run();
        break;
      case RUN:
        break;
      case JUMP_UP:
        break;
      case BE_ANGRY:
        // We just landed.
        if (!m_state_timer.started())
        {
          set_action("stand", m_dir);
          m_stomp_count++;
          drop_stalactite();

          // Go to the other side after 3 jumps.
          if (m_stomp_count == 3)
          {
            jump_down();
          }
          else
          {
            // Jump again.
            m_state_timer.start(STOMP_WAIT);
          }
        }
        break;
      case SQUISHED:
        break;
      case FALLING:
        break;
    }
  } else if (hit.left || hit.right) {
    // Hit wall.
    if(m_state != SQUISHED && m_state != FALLING)
      jump_up();
  }
}

ObjectSettings
Yeti::get_settings()
{
  ObjectSettings result = Boss::get_settings();

  result.add_bool(_("Fixed position"), &m_fixed_pos, "fixed-pos", false);

  return result;
}

void
Yeti::add_snow_explosions()
{
  for (int i = 0; i < SNOW_EXPLOSIONS_COUNT; i++) {
    Vector pos = get_pos();
    Vector velocity(SNOW_EXPLOSIONS_VX * graphicsRandom.randf(0.5f, 2.0f) * (graphicsRandom.rand(2) ? 1.0f : -1.0f),
                    SNOW_EXPLOSIONS_VY * graphicsRandom.randf(0.5f, 2.0f));
    pos.x += static_cast<float>(m_sprite->get_width()) / 2.0f;
    pos.x += static_cast<float>(m_sprite->get_width()) * graphicsRandom.randf(0.3f, 0.5f) * ((velocity.x > 0) ? 1.0f : -1.0f);
    pos.y += static_cast<float>(m_sprite->get_height()) * graphicsRandom.randf(-0.3f, 0.3f);
    velocity.x += m_physic.get_velocity_x();
    Sector::get().add<SnowExplosionParticle>(pos, velocity);
  }
}

Yeti::SnowExplosionParticle::SnowExplosionParticle(const Vector& pos, const Vector& velocity)
  : BadGuy(pos, (velocity.x > 0) ? Direction::RIGHT : Direction::LEFT, "images/objects/bullets/icebullet.sprite")
{
  m_physic.set_velocity(velocity);
  m_physic.enable_gravity(true);
  set_state(STATE_FALLING);
  m_layer = Sector::get().get_foremost_opaque_layer() + 1;
}

std::vector<Direction>
Yeti::get_allowed_directions() const
{
  return {};
}

/* EOF */
