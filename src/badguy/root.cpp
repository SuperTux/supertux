//  SuperTux - Corrupted Root
//  Copyright (C) 2023 MatusGuy <matusguy@supertuxproject.org>
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

#include "badguy/root.hpp"

#include "audio/sound_manager.hpp"
#include "math/random.hpp"
#include "object/sprite_particle.hpp"
#include "supertux/sector.hpp"

static const float HATCH_TIME = 0.7f;
static const float APPEAR_TIME = 0.5f;
static const float RETREAT_TIME = 1.f;

Root::Root(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/mole/corrupted/root.sprite" , LAYER_TILES-5)
{
  construct();
}

Root::Root(const Vector& pos, Direction dir, const std::string& sprite) :
  BadGuy(pos, dir, sprite, LAYER_TILES-5)
{
  construct();
}

void
Root::construct()
{
  m_base_surface = nullptr;
  m_state = STATE_HATCHING;
  m_offset = 0.f;
  m_maxheight = 0.f;

  m_countMe = false;
  m_physic.enable_gravity(false);
  set_colgroup_active(COLGROUP_TOUCHABLE);
  set_action("root", m_dir);

  Vector pos = get_pos();
  switch (m_dir)
  {
    case Direction::DOWN:
      pos.y -= get_bbox().get_height();
    case Direction::UP:
      pos.x -= (get_bbox().get_width() / 2);
      break;

    case Direction::RIGHT:
      pos.x -= get_bbox().get_width();
    case Direction::LEFT:
      pos.y -= (get_bbox().get_height() / 2);
      break;

    default: assert(false); break;
  }

  set_pos(pos);
  set_start_position(pos);

  auto surfaces = m_sprite->get_action_surfaces("base-" + dir_to_string(m_dir));
  if (surfaces.size() != 0)
    m_base_surface = surfaces[0];

  SoundManager::current()->preload("sounds/brick.wav");
  SoundManager::current()->preload("sounds/dartfire.wav");
}

void
Root::initialize()
{
  SoundManager::current()->play("sounds/brick.wav", get_pos());

  Vector basepos = get_bbox().get_middle();
  switch (m_dir)
  {
    case Direction::UP:
      basepos.y = m_start_position.y - 10;
      break;
    case Direction::DOWN:
      basepos.y = m_start_position.y + get_bbox().get_height() + 10;
      break;
    case Direction::LEFT:
      basepos.x = m_start_position.x - 10;
      break;
    case Direction::RIGHT:
      basepos.x = m_start_position.x + get_bbox().get_width() + 10;
      break;
    default: assert(false); break;
  }

  const float gravity = Sector::get().get_gravity() * 100.f;
  for (int i = 0; i < 5; i++)
  {
    const Vector velocity(graphicsRandom.randf(-100, 100),
                          graphicsRandom.randf(-400, -300));
    Sector::get().add<SpriteParticle>("images/particles/corrupted_rock.sprite",
                                      "piece-" + std::to_string(i),
                                      basepos, ANCHOR_MIDDLE,
                                      velocity, Vector(0, gravity),
                                      LAYER_OBJECTS + 3, true);
  }

  m_timer.start(HATCH_TIME);
}

void
Root::draw(DrawingContext &context)
{
  BadGuy::draw(context);

  if (!m_base_surface) return;

  Vector pos = m_start_position;
  switch (m_dir)
  {
    case Direction::UP:
      pos.x -= m_sprite->get_current_hitbox_x_offset();
      pos.y -= get_bbox().get_height() + 17.5f;
      break;

    case Direction::DOWN:
      pos.x -= m_sprite->get_current_hitbox_x_offset();
      pos.y += get_bbox().get_height() - 5;
      break;

    case Direction::LEFT:
      pos.x -= get_bbox().get_width() + 12.5f;
      pos.y -= m_sprite->get_current_hitbox_y_offset();
      break;

    case Direction::RIGHT:
      pos.x += get_bbox().get_width() - 5;
      pos.y -= m_sprite->get_current_hitbox_y_offset();
      break;

    default: assert(false); break;
  }
  context.color().draw_surface(m_base_surface,
                               pos,
                               m_sprite->get_angle(),
                               m_sprite->get_color(),
                               m_sprite->get_blend(),
                               m_layer+1);
}

void
Root::active_update(float dt_sec)
{
  BadGuy::active_update(dt_sec);

  switch (m_state)
  {
    case STATE_HATCHING:
      if (m_timer.check())
      {
        m_state = STATE_APPEARING;
        m_timer.start(APPEAR_TIME);
      }
      break;

    case STATE_APPEARING:
    {
      float size = 0;
      switch (m_dir)
      {
        case Direction::LEFT:
        case Direction::RIGHT:
          size = get_bbox().get_width();
          break;

        case Direction::UP:
        case Direction::DOWN:
          size = get_bbox().get_height();
          break;

        default: assert(false); break;
      }
      float progress = m_timer.get_timegone() / m_timer.get_period();
      m_offset = static_cast<float>(QuadraticEaseIn(static_cast<double>(progress))) * size;

      Vector pos = m_start_position;
      switch (m_dir)
      {
        case Direction::UP: pos.y -= m_offset; break;
        case Direction::DOWN: pos.y += m_offset; break;
        case Direction::LEFT: pos.x -= m_offset; break;
        case Direction::RIGHT: pos.x += m_offset; break;
        default: assert(false); break;
      }
      set_pos(pos);

      if (m_timer.check())
      {
        m_state = STATE_RETREATING;
        switch (m_dir)
        {
          case Direction::LEFT:
          case Direction::RIGHT:
            m_maxheight = get_pos().x;
            break;

          case Direction::UP:
          case Direction::DOWN:
            m_maxheight = get_pos().y;
            break;

          default: assert(false); break;
        }
        SoundManager::current()->play("sounds/darthit.wav", get_pos());
        m_timer.start(RETREAT_TIME);
      }

      break;
    }

    case STATE_RETREATING:
    {
      float size = 0;
      switch (m_dir)
      {
        case Direction::LEFT:
        case Direction::RIGHT:
          size = get_bbox().get_width();
          break;

        case Direction::UP:
        case Direction::DOWN:
          size = get_bbox().get_height();
          break;

        default: assert(false); break;
      }
      float progress = m_timer.get_timegone() / m_timer.get_period();
      m_offset = static_cast<float>(QuadraticEaseIn(static_cast<double>(progress))) * size;

      Vector pos = m_start_position;
      switch (m_dir)
      {
        case Direction::UP: pos.y = m_maxheight + m_offset; break;
        case Direction::DOWN: pos.y = m_maxheight - m_offset; break;
        case Direction::LEFT: pos.x = m_maxheight + m_offset; break;
        case Direction::RIGHT: pos.x = m_maxheight - m_offset; break;
        default: assert(false); break;
      }
      set_pos(pos);

      if (m_timer.check())
      {
        remove_me();
      }

      break;
    }
  }
}

HitResponse
Root::collision_badguy(BadGuy &other, const CollisionHit &hit)
{
  if (other.get_group() == COLGROUP_MOVING && other.is_snipable())
  {
    other.kill_fall();
    return ABORT_MOVE;
  }

  return BadGuy::collision_badguy(other, hit);
}

std::vector<Direction>
Root::get_allowed_directions() const
{
  return { Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT };
}

/* EOF */
