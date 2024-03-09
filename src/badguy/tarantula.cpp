//  SuperTux
//  Copyright (C) 2024 MatusGuy <matusguy@supertuxproject.org>
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

#include "badguy/tarantula.hpp"

#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"

static const float DROP_TIME = .5f;
static const float HANG_TIME = .5f;
static const float HANG_HEIGHT = 70.f;
static const float MOVE_SPEED = 75.f;

Tarantula::Tarantula(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/tarantula/tarantula.sprite"),
  m_state(STATE_IDLE),
  m_timer(),
  m_was_grabbed(false),
  m_target_height(0),
  m_last_height(0),
  m_retreat(true)
{
  parse_type(reader);
  set_action("idle");

  m_physic.enable_gravity(false);
}

void
Tarantula::initialize()
{
  m_last_height = m_start_position.y;
}

bool
Tarantula::collision_squished(GameObject& object)
{
  if (m_frozen)
    return BadGuy::collision_squished(object);

  set_action("squished", m_dir);
  kill_squished(object);
  return true;
}

void
Tarantula::active_update(float dt_sec)
{
  BadGuy::active_update(dt_sec);

  if (is_grabbed())
  {
    m_was_grabbed = true;
    return;
  }

  if (m_state == STATE_HANG_UP || m_state == STATE_HANG_DOWN)
  {
    Player* player = get_nearest_player();
    if (!player)
      goto state_logic;

    float dist = get_bbox().get_left() - player->get_bbox().get_left();
    if (std::abs(dist) > 5.f*32)
    {
      m_retreat = true;
    }
  }

state_logic:

  switch (m_state)
  {
    case STATE_IDLE:
      m_retreat = false;
      [[fallthrough]];
    case STATE_APPROACHING:
      switch (try_approach())
      {
        case NONE:
          m_physic.set_velocity_x(0.f);
          break;

        case DROP:
          m_physic.set_velocity_x(0.f);
          try_drop();
          break;

        default:
          break;
      }

      break;

    case STATE_DROPPING:
      hang_to(m_target_height, DROP_TIME, STATE_HANG_UP, EaseQuadIn, "dive");
      break;

    case STATE_HANG_UP:
      hang_to(m_last_height - HANG_HEIGHT, HANG_TIME, STATE_HANG_DOWN, EaseSineInOut, "rebound");
      break;

    case STATE_HANG_DOWN:
    {
      bool finished = hang_to(m_last_height + HANG_HEIGHT, HANG_TIME, STATE_HANG_UP, EaseSineInOut, "dive");
      if (m_retreat && finished)
      {
        m_state = STATE_RETREATING;
        [[fallthrough]];
      }
      else
      {
        break;
      }
    }

    case STATE_RETREATING:
      hang_to(m_start_position.y, DROP_TIME, STATE_IDLE, EaseQuadIn, "rebound");
      break;

    default:
      break;
  }
}

Tarantula::ApproachResponse
Tarantula::try_approach()
{
  Player* player = get_nearest_player();
  if (!player)
    return NONE;

  if (get_bbox().get_bottom() >= player->get_bbox().get_top())
    return NONE;

  Vector eye(get_bbox().get_middle().x, get_bbox().get_bottom() + 1);
  float dist = get_bbox().get_left() - player->get_bbox().get_left();

  if (std::abs(dist) > 5.f*32)
    return NONE;

  if (!Sector::get().can_see_player(eye))
    return NONE;

  if (std::abs(dist) <= 7.5f)
    return DROP;
  else if (dist > 0)
    m_physic.set_velocity_x(-MOVE_SPEED);
  else if (dist < 0)
    m_physic.set_velocity_x(MOVE_SPEED);

  m_state = STATE_APPROACHING;
  return APPROACH;
}

bool Tarantula::try_drop()
{
  using RaycastResult = CollisionSystem::RaycastResult;

  // Assuming the player has already been checked...

  Vector eye(get_bbox().get_middle().x, get_bbox().get_bottom() + 1);
  RaycastResult result = Sector::get().get_first_line_intersection(eye,
                                                                   Vector(eye.x, eye.y + 600.f),
                                                                   true,
                                                                   nullptr);

  if (!result.is_valid)
    return false;

  m_state = STATE_DROPPING;
  m_target_height = std::max(result.box.get_top() - 16.f - get_bbox().get_height(), 0.f);
  m_timer.start(DROP_TIME);

  return true;
}

bool Tarantula::hang_to(float height, float nexttime, State nextstate, EasingMode easing, const std::string& action)
{
  set_action(action, 0);

  m_target_height = height;

  if (!m_timer.started())
  {
    m_state = nextstate;
    m_last_height = height;
    m_timer.start(nexttime);
    return true;
  }

  double progress = static_cast<double>(m_timer.get_timegone() / m_timer.get_period());
  float offset = static_cast<float>(getEasingByName(easing)(progress)) * (m_target_height - m_last_height);

  Vector pos(get_bbox().get_left(), m_last_height + offset);
  set_pos(pos);

  return false;
}

void
Tarantula::collision_solid(const CollisionHit& hit)
{
  BadGuy::collision_solid(hit);

  if (m_was_grabbed && m_frozen && hit.bottom) {
    kill_fall();
  }
}

void
Tarantula::draw(DrawingContext& context)
{
  BadGuy::draw(context);

  context.color().draw_filled_rect(Rectf(Vector(get_bbox().get_left()-3, m_target_height -3), Sizef(3,3)), Color::CYAN, 1.5f, LAYER_HUD);
}

void
Tarantula::freeze()
{
  m_physic.enable_gravity(true);
  BadGuy::freeze();
}

void
Tarantula::unfreeze(bool)
{
  kill_fall();
}

bool
Tarantula::is_freezable() const
{
  return true;
}

bool Tarantula::is_snipable() const
{
  return m_state != STATE_DROPPING;
}

GameObjectTypes
Tarantula::get_types() const
{
  return {
    { "tarantula", _("Tarantula") },
    { "spidermite", _("Spidermite") }
  };
}

std::string
Tarantula::get_default_sprite_name() const
{
  switch (m_type)
  {
    case TARANTULA: return "images/creatures/tarantula/tarantula.sprite";
    case SPIDERMITE: return "images/creatures/spidermite/spidermite.sprite";
  }
  return "images/creatures/tarantula/tarantula.sprite";
}

std::vector<Direction>
Tarantula::get_allowed_directions() const
{
  return {};
}

/* EOF */
