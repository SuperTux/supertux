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
#include "math/util.hpp"
#include "object/brick.hpp"
#include "object/coin.hpp"
#include "object/camera.hpp"
#include "object/particles.hpp"
#include "object/player.hpp"
#include "object/rock.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "video/surface.hpp"

namespace {
  /* Maximum movement speed in pixels per LOGICAL_FPS. */
  const float RECOVER_SPEED_NORMAL = -3.125f;
  const float RECOVER_SPEED_LARGE = -2.0f;
  const float DROP_ACTIVATION_DISTANCE = 4.0f;
  const float PAUSE_TIME_NORMAL = 0.5f;
  const float PAUSE_TIME_LARGE = 1.0f;
  const float DETECT_RANGE = 400.f;
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
  m_righteye()
{
  parse_type(reader);
  after_sprite_set();

  m_physic.enable_gravity(false);

  std::string dir = "";
  reader.get("direction", dir);
  m_dir = CrusherDirection_from_string(dir);

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
  return {
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
Crusher::should_finish_crushing(const CollisionHit& hit)
{
  if (m_dir == CrusherDirection::ALL)
    return hit.bottom || hit.top || hit.left || hit.right;

  return ((m_dir == CrusherDirection::VERTICAL   || m_dir == CrusherDirection::DOWN ) && hit.bottom) ||
         ((m_dir == CrusherDirection::VERTICAL   || m_dir == CrusherDirection::UP   ) && hit.top   ) ||
         ((m_dir == CrusherDirection::HORIZONTAL || m_dir == CrusherDirection::LEFT ) && hit.left  ) ||
         ((m_dir == CrusherDirection::HORIZONTAL || m_dir == CrusherDirection::RIGHT) && hit.right );
}

bool
Crusher::has_recovered()
{
  switch (m_dir)
  {
    case CrusherDirection::DOWN:  return get_bbox().get_top() <= m_start_position.y;
    case CrusherDirection::UP:    return get_bbox().get_bottom() >= m_start_position.y;
    case CrusherDirection::LEFT:  return get_bbox().get_right() >= m_start_position.x;
    case CrusherDirection::RIGHT: return get_bbox().get_left() <= m_start_position.x;

    case CrusherDirection::VERTICAL: return math::in_bounds(get_bbox().get_top(),
                                                            m_start_position.y - 1.5f,
                                                            m_start_position.y + 1.5f);

    case CrusherDirection::HORIZONTAL: return math::in_bounds(get_bbox().get_left(),
                                                              m_start_position.x - 1.5f,
                                                              m_start_position.x + 1.5f);

    case CrusherDirection::ALL:
      return Rectf(m_start_position - Vector(1.5f, 1.5f), Sizef(1.5f, 1.5f) * 2).contains(get_pos());
  }

  return false;
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
    case CrusherDirection::DOWN:  return Vector(0.f, 1.f);
    case CrusherDirection::UP:    return Vector(0.f, -1.f);
    case CrusherDirection::LEFT:  return Vector(-1.f, 0.f);
    case CrusherDirection::RIGHT: return Vector(1.f, 0.f);

    case CrusherDirection::HORIZONTAL:
    {
      if (!m_target)
        return Vector(0.f, 0.f);

      Vector mid = get_bbox().get_middle();
      return mid.x <= m_target->get_bbox().get_left() ? Vector(1.f, 0.f) : Vector(-1.f, 0.f);
    }

    case CrusherDirection::VERTICAL:
    {
      if (!m_target)
        return Vector(0.f, 0.f);

      Vector mid = get_bbox().get_middle();
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
}

void
Crusher::crushed()
{
  m_state = DELAY;
  m_state_timer.start(m_ic_size == NORMAL ? PAUSE_TIME_NORMAL : PAUSE_TIME_LARGE, true);
  m_physic.set_acceleration(Vector(0.f, 0.f));

  SoundManager::current()->play(get_crush_sound(), get_pos());

  const float shake_time = m_ic_size == LARGE ? 0.125f : 0.1f;
  const float shake_intensity = m_ic_size == LARGE ? 32.f : 16.f;
  const Vector shake = Vector(shake_intensity, shake_intensity) * m_dir_vector;
  Sector::get().get_camera().shake(shake_time, shake.x, shake.y);
}

void
Crusher::recover()
{
  m_state = RECOVERING;
  m_physic.set_acceleration(Vector(0.f, 0.f));
  m_physic.set_velocity(m_dir_vector * -160.f);
}

void
Crusher::idle()
{
  m_state = IDLE;
  m_physic.set_velocity(Vector(0.f, 0.f));

  // Force start position. Current position shouldn't
  // stray away from this value much so the effect of
  // doing this shouldn't be noticeable.
  set_pos(m_start_position);
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

      // Determine weighting for eye displacement along x given crusher eye shape.
      int weight_x = m_sprite->get_width() / 64 * (((displacement.x > 0) == right) ? 1 : 4);
      int weight_y = m_sprite->get_width() / 64 * 2;

      return Vector(displacement.x / displacement_mag * static_cast<float>(weight_x),
        displacement.y / displacement_mag * static_cast<float>(weight_y) - static_cast<float>(weight_y));
    }

    case CRUSHING:
    {
      float step = m_sprite->get_width() / 64.f * 2.f;
      float x = (right == (m_dir_vector.x < 0) ? 2 : 1) * step;

      return Vector(x * m_dir_vector.x,
                    -step + (step * m_dir_vector.y));
    }

    case DELAY: [[fallthrough]];
    case RECOVERING:
    {
      // Amplitude dependent on size.
      float amplitude = static_cast<float>(m_sprite->get_width()) / 64.0f * 2.0f;

      // Phase factor due to cooldown timer.
      float spin_speed = m_ic_size == NORMAL ? RECOVER_SPEED_NORMAL : RECOVER_SPEED_LARGE;
      float cooldown_phase_factor = spin_speed + m_state_timer.get_progress() * 13.0f;

      // Phase factor due to y position.
      //auto y_position_phase_factor = !m_sideways ? get_pos().y / 13 : get_pos().x / 13;
      float y_position_phase_factor = m_dir_vector.y;

      float phase_factor = y_position_phase_factor - cooldown_phase_factor;

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

  return Vector(0, 0);
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
  return FORCE_MOVE;
}

void
Crusher::collision_solid(const CollisionHit& hit)
{
  if (m_state == CRUSHING && should_finish_crushing(hit))
  {
    crushed();
  }
}

void
Crusher::update(float dt_sec)
{
  MovingSprite::update(dt_sec);

  switch (m_state)
  {
    case IDLE:
      if (should_crush())
        crush();

      break;

    case DELAY:
      if (m_state_timer.check())
        recover();

      break;

    case RECOVERING:
      if (has_recovered())
        idle();

      break;

    default:
      break;
  }

  m_col.set_movement(m_physic.get_movement(dt_sec));
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
                  {_("Down"), _("Up"), _("Left"), _("Right"), _("Horizontal"), _("Vertical"), _("All")},
                  {"down", "up", "left", "right", "horizontal", "vertical", "all"},
                  static_cast<int>(CrusherDirection::DOWN),
                  "direction");
  result.reorder({ "direction", "sprite", "x", "y" });

  return result;
}

/* EOF */
