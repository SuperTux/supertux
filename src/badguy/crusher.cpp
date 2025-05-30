//  Crusher - A block to stand on, which can drop down to crush the player
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

#include "badguy/crusher.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <optional>

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "badguy/root.hpp"
#include "math/util.hpp"
#include "object/brick.hpp"
#include "object/camera.hpp"
#include "object/coin.hpp"
#include "object/particles.hpp"
#include "object/player.hpp"
#include "object/rock.hpp"
#include "object/tilemap.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "video/surface.hpp"

namespace
{
  /* Maximum movement speed in pixels per LOGICAL_FPS. */
  constexpr float RECOVER_SPEED_NORMAL = -3.125f;
  constexpr float RECOVER_SPEED_LARGE = -2.0f;
  constexpr float PAUSE_TIME_NORMAL = 0.5f;
  constexpr float PAUSE_TIME_LARGE = 1.0f;
  constexpr float DETECT_RANGE = std::numeric_limits<float>::infinity();

  constexpr float MAX_CRUSH_SPEED = 700.f;
  constexpr float POSITION_RESET_OFFSET = 2.6f;

  constexpr float RECOVER_SPEED_MULTIPLIER_NORMAL = 1.125f;
  constexpr float RECOVER_SPEED_MULTIPLIER_LARGE = 1.0f;

  constexpr float BRICK_BREAK_PROBE_DISTANCE = 9.f;
}

Crusher::CrusherDirection
Crusher::CrusherDirection_from_string(std::string_view str)
{
  if (str == "down")
    return CrusherDirection::DOWN;
  if (str == "up")
    return CrusherDirection::UP;
  if (str == "left")
    return CrusherDirection::LEFT;
  if (str == "right")
    return CrusherDirection::RIGHT;
  if (str == "horizontal")
    return CrusherDirection::HORIZONTAL;
  if (str == "vertical")
    return CrusherDirection::VERTICAL;
  if (str == "all")
    return CrusherDirection::ALL;

  return CrusherDirection::DOWN;
}

Crusher::Crusher(const ReaderMapping& reader) :
  MovingSprite(reader, "images/creatures/crusher/krush_ice.sprite", LAYER_OBJECTS, COLGROUP_MOVING_STATIC),
  m_state(IDLE),
  m_ic_size(NORMAL),
  m_ic_type(ICE),
  m_start_position(get_bbox().p1()),
  m_physic(),
  m_dir(CrusherDirection::DOWN),
  m_dir_vector(get_direction_vector()),
  m_target(nullptr),
  m_whites(),
  m_lefteye(),
  m_righteye(),
  m_crush_script()
{
  parse_type(reader);
  after_sprite_set();

  m_physic.enable_gravity(false);

  std::string dir = "";
  reader.get("direction", dir);
  m_dir = CrusherDirection_from_string(dir);

  reader.get("crush-script", m_crush_script);

  // The sideways option no longer exists.
  // This is for compatibility.
  bool sideways = false;
  reader.get("sideways", sideways);
  if (sideways)
    m_dir = CrusherDirection::HORIZONTAL;

  // TODO: Add distinct sounds for crusher hitting the ground and hitting Tux.
  SoundManager::current()->preload(get_crush_sound());
}

GameObjectTypes
Crusher::get_types() const
{
  return
  {
    { "ice-krush", _("Ice (normal)") },
    { "ice-krosh", _("Ice (big)") },
    { "rock-krush", _("Rock (normal)") },
    { "rock-krosh", _("Rock (big)") },
    { "corrupted-krush", _("Corrupted (normal)") },
    { "corrupted-krosh", _("Corrupted (big)") }
  };
}

std::string
Crusher::get_default_sprite_name() const
{
  const std::string size_prefix = (m_ic_size == NORMAL ? "krush" : "krosh");
  switch (m_ic_type)
  {
    case ROCK:
      return "images/creatures/crusher/" + size_prefix + "_rock.sprite";
    case CORRUPTED:
      return "images/creatures/crusher/corrupted/" + size_prefix + "_corrupt.sprite";
    default:
      return "images/creatures/crusher/" + size_prefix + "_ice.sprite";
  }
}

void
Crusher::run_crush_script()
{
  if (!m_crush_script.empty())
    Sector::get().run_script(m_crush_script, "crush-script");
}

bool
Crusher::should_crush()
{
  using RaycastResult = CollisionSystem::RaycastResult;

  for (Player* player : Sector::get().get_players())
  {
    const Rectf& playerbbox = player->get_bbox();

    if (m_dir == CrusherDirection::ALL)
    {
      if (!(playerbbox.overlaps(get_detect_box(CrusherDirection::HORIZONTAL)) ||
        playerbbox.overlaps(get_detect_box(CrusherDirection::VERTICAL))))

        continue;
    }
    else
    {
      if (!playerbbox.overlaps(get_detect_box()))
        continue;
    }

    RaycastResult result = Sector::get().get_first_line_intersection(get_bbox().get_middle(),
                                                                     playerbbox.get_middle(),
                                                                     false,
                                                                     get_collision_object());

    auto obj_p = std::get_if<CollisionObject*>(&result.hit);
    if (!obj_p || *obj_p != player->get_collision_object())
      continue;

    m_target = *obj_p;
    return true;
  }

  return false;
}

bool
Crusher::should_finish_crushing(const CollisionHit& hit) const
{
  if (m_dir == CrusherDirection::ALL)
    return hit.bottom || hit.top || hit.left || hit.right;

  return ((m_dir == CrusherDirection::VERTICAL   || m_dir == CrusherDirection::DOWN) && hit.bottom) ||
         ((m_dir == CrusherDirection::VERTICAL   || m_dir == CrusherDirection::UP) && hit.top)      ||
         ((m_dir == CrusherDirection::HORIZONTAL || m_dir == CrusherDirection::LEFT) && hit.left)   ||
         ((m_dir == CrusherDirection::HORIZONTAL || m_dir == CrusherDirection::RIGHT) && hit.right);
}

bool
Crusher::has_recovered()
{
  const float current_top = get_bbox().get_top();
  const float current_bottom = get_bbox().get_bottom();
  const float current_left = get_bbox().get_left();
  const float current_right = get_bbox().get_right();
  Vector current_pos = get_pos();

  // Defines the zone around m_start_position to consider recovered.
  const float tolerance = 2.0f;
  bool recovered = false;

  switch (m_dir)
  {
    case CrusherDirection::DOWN:
      recovered = current_top <= (m_start_position.y + tolerance);
      break;
    case CrusherDirection::UP:
      recovered = current_top >= (m_start_position.y - tolerance);
      break;
    case CrusherDirection::LEFT:
      recovered = current_left >= (m_start_position.x - tolerance);
      break;
    case CrusherDirection::RIGHT:
      recovered = current_left <= (m_start_position.x + tolerance);
      break;
    case CrusherDirection::VERTICAL:
      recovered = math::in_bounds(current_top, m_start_position.y - tolerance, m_start_position.y + tolerance);
      break;
    case CrusherDirection::HORIZONTAL:
      recovered = math::in_bounds(current_left, m_start_position.x - tolerance, m_start_position.x + tolerance);
      break;
    case CrusherDirection::ALL:
      recovered = Rectf(m_start_position - Vector(tolerance, tolerance), Sizef(tolerance, tolerance) * 2.f).contains(current_pos);
      break;
    default:
      recovered = false;
  }

  return recovered;
}

Rectf
Crusher::get_detect_box(CrusherDirection dir)
{
  if (dir == CrusherDirection::ALL)
    dir = m_dir;

  Vector pos = get_pos();
  switch (dir)
  {
    case CrusherDirection::VERTICAL: [[fallthrough]];
    case CrusherDirection::UP:
      pos.y -= DETECT_RANGE;
      break;

    case CrusherDirection::HORIZONTAL: [[fallthrough]];
    case CrusherDirection::LEFT:
      pos.x -= DETECT_RANGE;
      break;

    default:
      break;
  }

  Sizef size = get_bbox().get_size();
  switch (dir)
  {
    case CrusherDirection::VERTICAL:
      size.height += DETECT_RANGE * 2;
      break;
    case CrusherDirection::UP: [[fallthrough]];
    case CrusherDirection::DOWN:
      size.height += DETECT_RANGE;
      break;

    case CrusherDirection::HORIZONTAL:
      size.width += DETECT_RANGE * 2;
      break;
    case CrusherDirection::LEFT: [[fallthrough]];
    case CrusherDirection::RIGHT:
      size.width += DETECT_RANGE;
      break;

    default:
      break;
  }

  Rectf detectbox;
  detectbox.set_p1(pos);
  detectbox.set_size(size.width, size.height);

  return detectbox.grown(-1.f);
}

Vector
Crusher::get_direction_vector()
{
  switch (m_dir)
  {
    case CrusherDirection::DOWN:
      return Vector(0.f, 1.f);
    case CrusherDirection::UP:
      return Vector(0.f, -1.f);
    case CrusherDirection::LEFT:
      return Vector(-1.f, 0.f);
    case CrusherDirection::RIGHT:
      return Vector(1.f, 0.f);

    case CrusherDirection::HORIZONTAL:
    {
      if (!m_target)
        return Vector(0.f, 0.f);

      const Vector mid = get_bbox().get_middle();
      return mid.x <= m_target->get_bbox().get_left() ? Vector(1.f, 0.f) : Vector(-1.f, 0.f);
    }

    case CrusherDirection::VERTICAL:
    {
      if (!m_target)
        return Vector(0.f, 0.f);

      const Vector mid = get_bbox().get_middle();
      return mid.y <= m_target->get_bbox().get_top() ? Vector(0.f, 1.f) : Vector(0.f, -1.f);
    }

    case CrusherDirection::ALL:
    {
      if (!m_target)
        return Vector(0.f, 0.f);

      const Vector a = get_bbox().get_middle();
      const Vector b = m_target->get_bbox().get_middle();
      const Vector diff = b - a;

      if (std::abs(diff.x) < std::abs(diff.y))
      {
        return Vector(0.f, (diff.y > 0 ? 1 : -1));
      }
      else
      {
        return Vector((diff.x > 0 ? 1 : -1), 0.f);
      }
    }

    default:
      return Vector(0.f, 0.f);
  }
}

void
Crusher::crush()
{
  m_state = CRUSHING;
  m_dir_vector = get_direction_vector();
  m_physic.set_acceleration(m_dir_vector * 700.f);

  if (m_ic_type != ICE)
    set_action("crushing");
}

void
Crusher::spawn_particles(const CollisionHit& hit_info)
{
  const Color particle_color(0.6f, 0.6f, 0.6f);
  const float particle_size = 4.0f;
  const float particle_lifetime = 1.6f;
  const int particle_layer = m_layer + 1;

  Vector impact_point;
  float base_angle_deg = 0; // Pointing away from impact surface.

  if (hit_info.bottom)
  {
    impact_point = Vector(get_bbox().get_middle().x, get_bbox().get_bottom());
    base_angle_deg = 90; // Up
  }
  else if (hit_info.top)
  {
    impact_point = Vector(get_bbox().get_middle().x, get_bbox().get_top());
    base_angle_deg = 270; // Down
  }
  else if (hit_info.left)
  {
    impact_point = Vector(get_bbox().get_left(), get_bbox().get_middle().y);
    base_angle_deg = 0; // Right
  }
  else if (hit_info.right)
  {
    impact_point = Vector(get_bbox().get_right(), get_bbox().get_middle().y);
    base_angle_deg = 180; // Left
  }
  else
  {
    return;
  }

  // Throw some particles.
  for (int j = 0; j < 5; ++j) {
    Vector spawn_pos = impact_point;
    const float offset_dist = (static_cast<float>(j) - 2.0f) * 8.0f;

    if (hit_info.bottom || hit_info.top)
      spawn_pos.x += offset_dist;
    else
      spawn_pos.y += offset_dist;

    // Vary the angle.
    const float angle_deg = base_angle_deg + (static_cast<float>(j) - 2.0f) * 15.0f;
    const float min_angle = angle_deg - 10.f;
    const float max_angle = angle_deg + 10.f;

    Sector::get().add<Particles>(
      spawn_pos,
      min_angle, max_angle,
      140, 260,
      Vector(0, 500),
      1, particle_color, particle_size, particle_lifetime, particle_layer);
  }
}

Direction
Crusher::direction_from_vector(const Vector& vec)
{
  if (!((vec.x == 0.f) ^ (vec.y == 0.f)))
    return Direction::NONE;

  if (vec.x > 0.f)
    return Direction::RIGHT;
  else if (vec.x < 0.f)
    return Direction::LEFT;
  else if (vec.y > 0.f)
    return Direction::DOWN;
  else if (vec.y < 0.f)
    return Direction::UP;

  return Direction::AUTO;
}

void
Crusher::spawn_roots()
{
  Vector pos(0.f, 0.f);
  const Direction dir = direction_from_vector(m_dir_vector);
  float* axis{};
  float origin{}, pos1{}, pos2{};

  switch (dir)
  {
    case Direction::UP:
      origin = get_bbox().get_top();
      axis = &pos.x;
      pos1 = get_bbox().get_left();
      pos2 = get_bbox().get_right();
      break;

    case Direction::DOWN:
      origin = get_bbox().get_bottom();
      axis = &pos.x;
      pos1 = get_bbox().get_left();
      pos2 = get_bbox().get_right();
      break;

    case Direction::LEFT:
      origin = get_bbox().get_left();
      axis = &pos.y;
      pos1 = get_bbox().get_top();
      pos2 = get_bbox().get_bottom();
      break;

    case Direction::RIGHT:
      origin = get_bbox().get_right();
      axis = &pos.y;
      pos1 = get_bbox().get_top();
      pos2 = get_bbox().get_bottom();
      break;
  }

  *(axis == &pos.y ? &pos.x : &pos.y) = origin;

  for (int i = 0; i < 3; ++i)
  {
    const float delay = (static_cast<float>(i) + 1.f) * 0.6f;
    Root& root = Sector::get().add<Root>(Vector(0.f, 0.f), invert_dir(dir),
                                         "images/creatures/mole/corrupted/root.sprite",
                                         delay, false, false);
    const float dimension = (axis == &pos.y ? root.get_height() : root.get_width());

    if (axis != nullptr)
      *axis = pos1 - 10.f - (((dimension / 2.f) + 50.f) * static_cast<float>(i));

    root.set_pos(pos);
    root.construct();
  }

  for (int i = 0; i < 3; ++i)
  {
    const float delay = (static_cast<float>(i) + 1.f) * 0.6f;
    Root& root = Sector::get().add<Root>(Vector(0.f, 0.f), invert_dir(dir),
                                         "images/creatures/mole/corrupted/root.sprite",
                                         delay, false, false);
    const float dimension = (axis == &pos.y ? root.get_height() : root.get_width());

    if (axis != nullptr)
      *axis = pos2 + 10.f + (((dimension / 2.f) + 50.f) * static_cast<float>(i));

    root.set_pos(pos);
    root.construct();
  }
}

void
Crusher::crushed(const CollisionHit& hit_info)
{
  m_state = DELAY;
  m_state_timer.start(m_ic_size == NORMAL ? PAUSE_TIME_NORMAL : PAUSE_TIME_LARGE, true);
  m_physic.set_velocity(Vector(0.f, 0.f));
  m_physic.set_acceleration(Vector(0.f, 0.f));

  SoundManager::current()->play(get_crush_sound(), get_pos());

  const float shake_time = m_ic_size == LARGE ? 0.125f : 0.1f;
  const float shake_intensity = m_ic_size == LARGE ? 32.f : 16.f;
  const Vector shake = Vector(shake_intensity, shake_intensity) * m_dir_vector;
  Sector::get().get_camera().shake(shake_time, shake.x, shake.y);

  spawn_particles(hit_info);

  if (m_ic_type == CORRUPTED)
    spawn_roots();

  run_crush_script();
}

void
Crusher::recover()
{
  m_state = RECOVERING;
  m_physic.set_acceleration(Vector(0.f, 0.f));

  const float base_recovery_speed = -160.f;
  const float speed_multiplier = (m_ic_size == NORMAL) ? RECOVER_SPEED_MULTIPLIER_NORMAL : RECOVER_SPEED_MULTIPLIER_LARGE;

  m_physic.set_velocity(m_dir_vector * base_recovery_speed * speed_multiplier);

  if (m_ic_type != ICE)
    set_action("recovering");
}

void
Crusher::idle()
{
  m_state = IDLE;
  m_physic.set_velocity(Vector(0.f, 0.f));
  m_physic.set_acceleration(Vector(0.f, 0.f));

  Vector final_pos = m_start_position;
  if (std::abs(m_dir_vector.x) > 0.1f) // Horizontal
  {
    final_pos.x = m_start_position.x - m_dir_vector.x * POSITION_RESET_OFFSET;
    final_pos.y = m_start_position.y;
  }
  else if (std::abs(m_dir_vector.y) > 0.1f) // Vertical
  {
    final_pos.y = m_start_position.y - m_dir_vector.y * POSITION_RESET_OFFSET;
    final_pos.x = m_start_position.x;
  }

  set_pos(final_pos);
  set_action("idle");

  m_state_timer.start(m_ic_size == NORMAL ? PAUSE_TIME_NORMAL : PAUSE_TIME_LARGE, true);
  m_target = nullptr;
}

std::string
Crusher::get_crush_sound() const
{
  return m_ic_type != ICE ? "sounds/thud.ogg" : "sounds/brick.wav";
}

Vector
Crusher::eye_position(bool right) const
{
  switch (m_state)
  {
    case IDLE:
    {
      Player* player = Sector::get().get_nearest_player(get_bbox());
      if (!player)
        break;

      // Crusher focuses on approximate position of player's head.
      const float player_focus_x = (player->get_bbox().get_right() + player->get_bbox().get_left()) * 0.5f;
      const float player_focus_y = player->get_bbox().get_bottom() * 0.25f + player->get_bbox().get_top() * 0.75f;
      const Vector player_focus(player_focus_x, player_focus_y);

      // Crusher's approximate origin of line-of-sight.
      const Vector crusher_origin = get_bbox().get_middle();

      // Line-of-sight displacement from crusher to player.
      const Vector displacement = player_focus - crusher_origin;
      const float displacement_mag = glm::length(displacement);
      if (displacement_mag < 0.01f)
        break;

      // Determine weighting for eye displacement along x given crusher eye shape.
      const int weight_x = m_sprite->get_width() / 64 * (((displacement.x > 0) == right) ? 1 : 4);
      const int weight_y = m_sprite->get_width() / 64 * 2;

      return Vector(displacement.x / displacement_mag * static_cast<float>(weight_x),
        displacement.y / displacement_mag * static_cast<float>(weight_y) - static_cast<float>(weight_y));
    }

    case CRUSHING:
    {
      const float step = m_sprite->get_width() / 64.f * 2.f;
      const float x = (right == (m_dir_vector.x < 0) ? 2 : 1) * step;

      return Vector(x * m_dir_vector.x,
                    -step + (step * m_dir_vector.y));
    }

    case DELAY: [[fallthrough]];
    case RECOVERING:
    {
      // Amplitude dependent on size.
      const float amplitude = static_cast<float>(m_sprite->get_width()) / 64.0f * 2.0f;

      // Phase factor due to cooldown timer.
      const float spin_speed = m_ic_size == NORMAL ? RECOVER_SPEED_NORMAL : RECOVER_SPEED_LARGE;
      const float cooldown_phase_factor = spin_speed + m_state_timer.get_progress() * 13.0f;

      // Phase factor due to y position.
      const float y_position_phase_factor = m_dir_vector.y;

      const float phase_factor = y_position_phase_factor - cooldown_phase_factor;

      // Eyes spin while crusher is recovering, giving a dazed impression.
      return Vector(std::sin((right ? 1 : -1) * // X motion of each eye is opposite of the other.
        phase_factor) * amplitude - (right ? 1 : -1) *
        amplitude, // Offset to keep eyes visible.

        std::cos((right ? math::PI : 0.0f) + // Eyes spin out of phase of eachother.
        phase_factor) * amplitude -
        amplitude); // Offset to keep eyes visible.
    }

    default:
      break;
  }

  return Vector(0.f, 0.f);
}

void
Crusher::on_flip(float height)
{
  MovingSprite::on_flip(height);
  m_start_position.y = height - m_col.m_bbox.get_height() - m_start_position.y;
  FlipLevelTransformer::transform_flip(m_flip);
}

void
Crusher::on_type_change(int old_type)
{
  m_ic_size = (m_type % 2 == 0 ? NORMAL : LARGE);
  switch (m_type)
  {
    case 0:
    case 1:
      m_ic_type = ICE;
      break;
    case 2:
    case 3:
      m_ic_type = ROCK;
      break;
    case 4:
    case 5:
      m_ic_type = CORRUPTED;
      break;
  }

  MovingSprite::on_type_change(old_type);
}

HitResponse
Crusher::collision(MovingObject& other, const CollisionHit& hit)
{
  if (m_state != CRUSHING)
    return FORCE_MOVE;

  bool is_crushing_hit = false;
  if (m_dir_vector.y > 0.5f && hit.bottom) is_crushing_hit = true; // Down, hit bottom
  else if (m_dir_vector.y < -0.5f && hit.top) is_crushing_hit = true;    // Up, hit top
  else if (m_dir_vector.x < -0.5f && hit.left) is_crushing_hit = true;   // Left, hit left
  else if (m_dir_vector.x > 0.5f && hit.right) is_crushing_hit = true;  // Right, hit right

  if (!is_crushing_hit)
    return FORCE_MOVE;

  auto* player = dynamic_cast<Player*>(&other);
  if (player)
  {
    bool player_vulnerable = true;
    if ((m_dir_vector.y > 0.5f && hit.bottom) && !player->on_ground())
    {
      player_vulnerable = false;
    }

    if (player_vulnerable)
    {
      SoundManager::current()->play("sounds/brick.wav", get_pos());
      crushed(hit);

      if (!player->is_invincible())
      {
        player->kill(false);
      }

      return ABORT_MOVE;
    }
  }

  auto* badguy = dynamic_cast<BadGuy*>(&other);
  if (badguy)
  {
    badguy->kill_fall();
    return FORCE_MOVE;
  }

  auto* rock = dynamic_cast<Rock*>(&other);
  if (rock && !rock->is_grabbed())
  {
    SoundManager::current()->play("sounds/brick.wav", get_pos());
    crushed(hit);
    return ABORT_MOVE;
  }

  const auto* heavy_coin = dynamic_cast<HeavyCoin*>(&other);
  if (heavy_coin)
  {
    return ABORT_MOVE;
  }

  return FORCE_MOVE;
}

void
Crusher::collision_solid(const CollisionHit& hit)
{
  if (m_state == CRUSHING && should_finish_crushing(hit))
  {
    crushed(hit);
  }
  else if (m_state == RECOVERING)
  {
    idle();
  }
}

void
Crusher::update(float dt_sec)
{
  MovingSprite::update(dt_sec);

  switch (m_state)
  {
    case IDLE:
      if (!m_state_timer.started() || m_state_timer.check())
      {
        if (should_crush())
        {
          crush();
        }
      }
      break;

    case CRUSHING:
    {
      const Rectf base_box = get_bbox().grown(-1.f);
      Rectf break_box = base_box;

      if (m_dir_vector.y > 0.5f) // Down
      { 
        break_box.set_top(base_box.get_bottom());
        break_box.set_bottom(base_box.get_bottom() + BRICK_BREAK_PROBE_DISTANCE);
      }
      else if (m_dir_vector.y < -0.5f) // Up
      { 
        break_box.set_top(base_box.get_top() - BRICK_BREAK_PROBE_DISTANCE);
        break_box.set_bottom(base_box.get_top());
      }
      else if (m_dir_vector.x < -0.5f) // Left
      {
        break_box.set_left(base_box.get_left() - BRICK_BREAK_PROBE_DISTANCE);
        break_box.set_right(base_box.get_left());
      }
      else if (m_dir_vector.x > 0.5f) // Right
      {
        break_box.set_left(base_box.get_right());
        break_box.set_right(base_box.get_right() + BRICK_BREAK_PROBE_DISTANCE);
      }

      for (auto& brick : Sector::get().get_objects_by_type<Brick>()) {
        if (break_box.overlaps(brick.get_bbox())) {
          if (brick.get_class_name() != "heavy-brick") {
            brick.break_for_crusher(this);
          }
          else {
            if (is_big()) {
              brick.break_for_crusher(this);
            }
          }
        }
      }
    }

    Vector current_velocity = m_physic.get_velocity();
    if (m_dir_vector.x != 0.f) // Horizontal
    { 
      if (std::abs(current_velocity.x) > MAX_CRUSH_SPEED)
      {
        current_velocity.x = std::copysign(MAX_CRUSH_SPEED, current_velocity.x);
        m_physic.set_velocity(current_velocity);
      }
    }
    if (m_dir_vector.y != 0.f) // Vertical
    { 
      if (std::abs(current_velocity.y) > MAX_CRUSH_SPEED)
      {
        current_velocity.y = std::copysign(MAX_CRUSH_SPEED, current_velocity.y);
        m_physic.set_velocity(current_velocity);
      }
    }
    break;

    case DELAY:
      if (m_state_timer.check())
      {
        recover();
      }
      break;

    case RECOVERING:
      if (has_recovered())
      {
        idle();
      }
      break;

    default:
      log_warning << "Crusher is in an invalid state." << std::endl;
      break;
  }

  Vector frame_movement = m_physic.get_movement(dt_sec);

  bool in_water = !Sector::get().is_free_of_tiles(get_bbox(), true, Tile::WATER);
  if (in_water)
    frame_movement *= 0.6f;

  m_col.set_movement(frame_movement);
}

void
Crusher::draw(DrawingContext& context)
{
  const Vector draw_pos = get_pos() + m_physic.get_velocity() * context.get_time_offset();
  m_sprite->draw(context.color(), draw_pos, m_layer + 2, m_flip);

  if (m_whites)
  {
    context.push_transform();
    context.set_flip(m_flip);

    const Vector offset_pos = draw_pos - Vector(m_sprite->get_current_hitbox().p1());
    context.color().draw_surface(m_whites, offset_pos, m_layer);

    context.color().draw_surface(m_lefteye, offset_pos + eye_position(false), m_layer + 1);
    context.color().draw_surface(m_righteye, offset_pos + eye_position(true), m_layer + 1);

    context.pop_transform();
  }
}

void
Crusher::after_editor_set()
{
  MovingSprite::after_editor_set();
  m_start_position = get_bbox().p1();
  after_sprite_set();
}

void
Crusher::after_sprite_set()
{
  set_action("idle");

  m_whites.reset();
  m_lefteye.reset();
  m_righteye.reset();

  if (m_sprite->has_action("whites"))
  {
    using Surfaces = const std::optional<std::vector<SurfacePtr>>;

    Surfaces esurfaces = m_sprite->get_action_surfaces("whites");
    if (!esurfaces.has_value())
      return;
    m_whites = esurfaces.value()[0];

    Surfaces lsurfaces = m_sprite->get_action_surfaces("lefteye");
    if (!lsurfaces.has_value())
      return;
    m_lefteye = lsurfaces.value()[0];

    Surfaces rsurfaces = m_sprite->get_action_surfaces("righteye");
    if (!rsurfaces.has_value())
      return;
    m_righteye = rsurfaces.value()[0];
  }
}

ObjectSettings
Crusher::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();
  result.add_enum(_("Direction"), reinterpret_cast<int*>(&m_dir),
                  { _("Down"), _("Up"), _("Left"), _("Right"), _("Horizontal"), _("Vertical"), _("All") },
                  { "down", "up", "left", "right", "horizontal", "vertical", "all" },
                  static_cast<int>(CrusherDirection::DOWN),
                  "direction");

  result.add_script(_("Crush script"), &m_crush_script, "crush-script");
  result.reorder({ "direction", "sprite", "crush-script", "x", "y" });

  return result;
}

/* EOF */
