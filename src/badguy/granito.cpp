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

#include "granito.hpp"

#include "math/random.hpp"
#include "supertux/sector.hpp"
#include "object/player.hpp"

Granito::Granito(const ReaderMapping& reader):
  WalkingBadguy(reader, "images/creatures/granito/granito.sprite", "left", "right"),
  m_state(STATE_STAND),
  m_original_state(STATE_STAND),
  m_has_waved(false),
  m_stepped_on(false)
{
  parse_type(reader);

  walk_speed = 0;
  max_drop_height = 600;

  m_countMe = false;

}

void Granito::active_update(float dt_sec)
{
  if (get_velocity_y() != 0)
  {
    if (get_velocity_y() < 0)
      set_action("jump", m_dir);
    else if (get_velocity_y() > 0)
      set_action("fall", m_dir);
  }
  else if (on_ground() && m_state == m_original_state)
  {
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

  if (m_type == SIT || m_state == STATE_LOOKUP || m_state == STATE_JUMPING)
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

  if (m_type == WALK && try_jump())
  {
    WalkingBadguy::active_update(dt_sec);
    return;
  }

  // only called when timer is finished
  if ((!m_walk_interval.started() && !m_walk_interval.check()))
  {
    m_walk_interval.start(gameRandom.randf(1.f, 4.f));

    switch (m_type)
    {
      case STAND:
        if (gameRandom.rand(100) > 50)
        {
          // turn around
          m_dir = m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT;
          set_action("stand", m_dir);
        }

        break;

      case WALK:
      {
        if (gameRandom.rand(100) > 50 && walk_speed == 0)
        {
          // turn around
          m_dir = m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT;
          set_action("stand", m_dir);
        }
        else
        {
          // walk/stop
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
            // FIXME: Why do I need to add 1??? Grumbel, you...
            m_dir = (gameRandom.rand(1 + 1) == 0 ? Direction::LEFT : Direction::RIGHT);
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

HitResponse Granito::collision_player(Player& player, const CollisionHit &hit)
{
  if (m_type == SIT) return FORCE_MOVE;

  if (hit.top)
  {
    m_stepped_on = true;
    //m_col.propagate_movement(m_col.get_movement());
    if (m_state != STATE_LOOKUP)
    {
      m_state = STATE_LOOKUP;
      walk_speed = 0;
      m_physic.set_velocity_x(0);
      set_action("lookup", m_dir);

      // Don't wave again because we've
      // already spotted the player
      m_has_waved = true;
    }
  }

  return FORCE_MOVE;
}

void Granito::kill_fall()
{
  return;
}

void Granito::activate()
{
  WalkingBadguy::activate();
  m_has_waved = false;
}

GameObjectTypes Granito::get_types() const
{
  return {
    {"walking", _("Walking")},
    {"standing", _("Standing")},
    {"sitting", _("Sitting")}
  };
}

void Granito::initialize()
{
  WalkingBadguy::initialize();

  switch (m_type)
  {
    case WALK:
      set_action(m_dir);
      break;

    case SIT:
      set_action("sit", m_dir);
      break;

    case STAND:
      set_action("stand", m_dir);
      break;
  }

  set_colgroup_active(COLGROUP_MOVING_STATIC);
}

void Granito::on_type_change(__attribute__((unused)) int old_type)
{
  // FIXME: change action for type in editor
}

bool Granito::try_wave()
{
  using RaycastResult = CollisionSystem::RaycastResult;

  if (!on_ground()) return false;

  Player* plr = get_nearest_player();
  if (!plr) return false;

  RaycastResult result = Sector::get().get_first_line_intersection(get_bbox().get_middle(),
                                                                   plr->get_bbox().get_middle(),
                                                                   false,
                                                                   get_collision_object());

  if (result.hit.object != nullptr && result.hit.object == plr->get_collision_object())
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

void Granito::wave()
{
  walk_speed = 0;
  m_physic.set_velocity_x(0);

  m_state = STATE_WAVE;

  set_action("wave", m_dir, 1);
}

bool Granito::try_jump()
{
  if (walk_speed == 0 || !on_ground()) return false;

  Rectf detect = get_bbox().grown(-1.f);
  float inc = (m_dir == Direction::LEFT ? -32.f*1.25f : 32.f*.25f);

  detect.set_pos({(m_dir == Direction::LEFT ? detect.get_left() : detect.get_right()) + inc, detect.get_top()});
  if (Sector::get().is_free_of_tiles(detect)) return false;

  detect.set_pos({detect.get_left(), detect.get_top() - (32.f*2)});
  if (!Sector::get().is_free_of_tiles(detect)) return false;

  jump();
  return true;
}

void Granito::jump()
{
  m_state = STATE_JUMPING;
  m_physic.set_velocity_y(-420.f);
}

void Granito::restore_original_state()
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
