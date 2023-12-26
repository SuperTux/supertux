//  SuperTux
//  Copyright (C) 2023 MatusGuy
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

#include "badguy/granito.hpp"

#include "math/random.hpp"
#include "object/player.hpp"
#include "supertux/sector.hpp"

Granito::Granito(const ReaderMapping& reader, const std::string& sprite_name, int layer) :
  WalkingBadguy(reader, sprite_name, "left", "right", layer),
  m_walk_interval(),
  m_state(STATE_STAND),
  m_original_state(STATE_STAND),
  m_has_waved(false),
  m_stepped_on(false),
  m_airborne(false)
{
  parse_type(reader);

  walk_speed = 0;
  max_drop_height = 600;

  m_countMe = false;

  set_colgroup_active(COLGROUP_MOVING_STATIC);
  m_col.set_unisolid(true);
}

void
Granito::active_update(float dt_sec)
{
  if (m_type == SIT || m_type == WALK)
  {
    // Don't do any extra calculations
    WalkingBadguy::active_update(dt_sec);
    m_stepped_on = false;
    return;
  }

  Rectf airbornebox = get_bbox();
  airbornebox.set_bottom(get_bbox().get_bottom() + 8.f);
  bool airbornebefore = m_airborne;
  m_airborne = (Sector::get().is_free_of_statics(airbornebox));

  if (m_airborne && get_velocity_y() != 0)
  {
    // Choose if falling or jumping
    if (get_velocity_y() < 5)
      set_action("jump", m_dir);
    else if (get_velocity_y() > 5)
      set_action("fall", m_dir);
  }
  else if (!m_airborne && airbornebefore)
  {
    // Go back to normal action
    if (m_state == STATE_STAND)
    {
      if (m_type == SIT)
        set_action("sit", m_dir);
      else
        set_action("stand", m_dir);
    }
    else if (m_state == STATE_WALK)
    {
      set_action(m_dir);
    }
  }

  if ((m_state == STATE_LOOKUP && !m_stepped_on) ||
      (m_state == STATE_JUMPING && on_ground()))
  {
    restore_original_state();
  }

  if (m_state == STATE_LOOKUP || m_state == STATE_JUMPING)
  {
    // Don't do any extra calculations
    WalkingBadguy::active_update(dt_sec);
    m_stepped_on = false;
    return;
  }

  if (!m_has_waved)
  {
    if (m_state == STATE_WAVE)
    {
      if (!m_sprite->animation_done())
      {
        // Still waving
        WalkingBadguy::active_update(dt_sec);
        m_stepped_on = false;
        return;
      }
      else
      {
        // Finished waving
        restore_original_state();
        m_has_waved = true;
      }
    }
    else
    {
      try_wave();
    }
  }

  if (m_type == DEFAULT && try_jump())
  {
    WalkingBadguy::active_update(dt_sec);
    return;
  }

  // Only called, when timer has finished
  if (!m_walk_interval.started() && !m_walk_interval.check())
  {
    m_walk_interval.start(gameRandom.randf(1.f, 4.f));

    switch (m_type)
    {
      case STAND:
        if (gameRandom.rand(100) > 50)
        {
          // Turn around
          m_dir = m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT;
          set_action("stand", m_dir);
        }

        break;

      case DEFAULT:
      {
        if (gameRandom.rand(100) > 50 && walk_speed == 0)
        {
          // Turn around
          m_dir = m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT;
          set_action("stand", m_dir);
        }
        else
        {
          // Walk/stop
          if (walk_speed > 0)
          {
            walk_speed = 0;
            m_state = STATE_STAND;
            m_original_state = STATE_STAND;
            m_physic.set_velocity_x(0);
            set_action("stand", m_dir);
          }
          else
          {
            m_dir = (gameRandom.rand(2) == 0 ? Direction::LEFT : Direction::RIGHT);
            walk_speed = 80;
            m_state = STATE_WALK;
            m_original_state = STATE_WALK;
            m_physic.set_velocity_x(80 * (m_dir == Direction::LEFT ? -1 : 1));
            set_action(m_dir);
          }
        }

        break;
      }

      default:
        break;
    }
  }

  WalkingBadguy::active_update(dt_sec);

  m_stepped_on = false;
}

HitResponse
Granito::collision_player(Player& player, const CollisionHit& hit)
{
  if (m_type == SIT || m_type == WALK) return FORCE_MOVE;

  if (hit.top)
  {
    m_stepped_on = true;

    if (m_state != STATE_LOOKUP)
    {
      m_state = STATE_LOOKUP;
      walk_speed = 0;
      m_physic.set_velocity_x(0);
      set_action("lookup", m_dir);

      // Don't wave again because we've already spotted the player
      m_has_waved = true;
    }
  }

  return FORCE_MOVE;
}

HitResponse
Granito::collision(GameObject& other, const CollisionHit& hit)
{
  if (hit.top)
    m_col.propagate_movement(m_col.get_movement());

  return WalkingBadguy::collision(other, hit);
}

void
Granito::activate()
{
  WalkingBadguy::activate();
  m_has_waved = false;
}

GameObjectTypes
Granito::get_types() const
{
  return {
    // Big & small granito
    { "default", _("Default") },
    { "standing", _("Standing") },
    { "walking", _("Walking") },

    // Small granito only
    { "sitting", _("Sitting") }
  };
}

void
Granito::after_editor_set()
{
  WalkingBadguy::after_editor_set();

  switch (m_type)
  {
    case DEFAULT:
      set_action(m_dir);
      break;
    case SIT:
      set_action("sit", m_dir);
      break;
    case STAND:
      set_action("stand", m_dir);
      break;
  }
}

void
Granito::initialize()
{
  WalkingBadguy::initialize();

  if (m_type == WALK)
  {
    m_original_state = STATE_WALK;
    restore_original_state();
  }

  switch (m_type)
  {
    case DEFAULT:
      set_action(m_dir);
      break;

    case SIT:
      set_action("sit", m_dir);
      break;

    case STAND:
      set_action("stand", m_dir);
      break;
  }
}

void
Granito::update_hitbox()
{
  WalkingBadguy::update_hitbox();
  m_col.set_unisolid(true);
}

bool
Granito::try_wave()
{
  using RaycastResult = CollisionSystem::RaycastResult;

  if (!on_ground()) return false;

  Player* player = get_nearest_player();
  if (!player) return false;

  RaycastResult result = Sector::get().get_first_line_intersection(get_bbox().get_middle(),
                                                                   player->get_bbox().get_middle(),
                                                                   false,
                                                                   get_collision_object());

  CollisionObject** resultobj = std::get_if<CollisionObject*>(&result.hit);
  if (resultobj && *resultobj == player->get_collision_object())
  {
    float xdist = get_bbox().get_middle().x - result.box.get_middle().x;
    if (std::abs(xdist) < 32.f*4.f)
    {
      // Only wave if facing player.
      if (xdist == std::abs(xdist) * (m_dir == Direction::LEFT ? -1 : 1))
        return false;

      wave();
      return true;
    }
  }

  return false;
}

void
Granito::wave()
{
  walk_speed = 0;
  m_physic.set_velocity_x(0);

  m_state = STATE_WAVE;

  set_action("wave", m_dir, 1);
}

bool
Granito::try_jump()
{
  using RaycastResult = CollisionSystem::RaycastResult;

  if (walk_speed == 0 || m_airborne) return false;

  float eye = (m_dir == Direction::LEFT ? get_bbox().get_left() : get_bbox().get_right());
  float inc = (m_dir == Direction::LEFT ? -32.f : 32.f);

  RaycastResult result = Sector::get().get_first_line_intersection({eye, get_bbox().get_middle().y},
                                                                   {eye + inc, get_bbox().get_middle().y},
                                                                   false,
                                                                   get_collision_object());

  if (!result.is_valid) return false;

  auto result_tile = std::get_if<const Tile*>(&result.hit);
  if (result_tile)
  {
    if ((*result_tile)->is_slope())
      return false;
  }
  else
  {
    auto result_obj = std::get_if<CollisionObject*>(&result.hit);
    if (result_obj && !dynamic_cast<Granito*>(&(*result_obj)->get_listener()))
      return false;
  }

  const Rectf detect(Vector(eye + (m_dir == Direction::LEFT ? -48.f : 16.f),
                            get_bbox().get_top() - (32.f*2)),
                     get_bbox().get_size());

  if (!Sector::get().is_free_of_tiles(detect.grown(-1.f))) return false;

  jump();
  return true;
}

void
Granito::jump()
{
  m_state = STATE_JUMPING;
  m_physic.set_velocity_y(-420.f);
}

void
Granito::restore_original_state()
{
  if (m_state == m_original_state) return;

  m_state = m_original_state;

  if (m_state == STATE_WALK)
  {
    set_action(m_dir);
    walk_speed = 80;
    m_physic.set_velocity_x(80 * (m_dir == Direction::LEFT ? -1 : 1));
  }
  else
  {
    set_action("stand", m_dir);
    walk_speed = 0;
    m_physic.set_velocity_x(0);
  }
}

/* EOF */
