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
  m_target(nullptr)
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
  SoundManager::current()->preload(m_ic_type != ICE ? "sounds/thud.ogg" : "sounds/brick.wav");
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

  Rectf detectbox = get_detect_box();

  for (Player* player : Sector::get().get_players())
  {
    const Rectf& playerbbox = player->get_bbox();
    if (!playerbbox.overlaps(detectbox))
      continue;

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
  }

  return false;
}

Rectf
Crusher::get_detect_box()
{
  Vector pos = get_pos();
  switch (m_dir)
  {
    case CrusherDirection::VERTICAL: [[fallthrough]];
    case CrusherDirection::UP:
      pos.y -= DETECT_RANGE;
      break;

    case CrusherDirection::HORIZONTAL: [[fallthrough]];
    case CrusherDirection::LEFT:
      pos.x -= DETECT_RANGE;
      break;
  }

  Sizef size = get_bbox().get_size();
  switch (m_dir)
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

    default:
      return Vector(0.f, 0.f);
  }
}

void
Crusher::crush()
{
  m_state = CRUSHING;
  m_dir_vector = get_direction_vector();
  m_physic.set_acceleration(m_dir_vector * 750.f);
}

void
Crusher::recover()
{
  m_state = RECOVERING;
  m_physic.set_acceleration(Vector(0.f, 0.f));
  m_physic.set_velocity(m_dir_vector * -100.f);
}

void
Crusher::idle()
{
  m_state = IDLE;
  m_physic.set_velocity(Vector(0.f, 0.f));

  // Force start position.
  set_pos(m_start_position);
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
    m_state = DELAY;
    m_physic.set_acceleration_y(0.f);

    m_state_timer.start(1.75f);
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
  Vector draw_pos = get_pos() + m_physic.get_velocity() * context.get_time_offset();
  m_sprite->draw(context.color(), draw_pos, m_layer + 2, m_flip);
  if (m_sprite->has_action("whites"))
  {
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
}

ObjectSettings
Crusher::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();
  result.add_enum(_("Direction"), reinterpret_cast<int*>(&m_dir),
                  {_("Down"), _("Up"), _("Left"), _("Right"), _("Horizontal"), _("Vertical")},
                  {"down", "up", "left", "right", "horizontal", "vertical"},
                  static_cast<int>(CrusherDirection::DOWN),
                  "direction");
  result.reorder({ "direction", "sprite", "x", "y" });

  return result;
}

/* EOF */
