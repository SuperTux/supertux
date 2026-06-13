//  SuperTux
//  Copyright (C) 2026
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

#include "object/ghost_replay.hpp"

#include <algorithm>
#include <cassert>

#include "math/rectf.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/constants.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"

namespace {

// Universal Tux action used as initial sprite state for ghost replay
const char* const BOOTSTRAP_ACTION = "small-stand-right";

} // namespace

GhostReplay::GhostReplay(std::vector<worldmap::LevelTile::GhostRunPoint> path) :
  m_path(std::move(path)),
  m_time(0.0f),
  m_position(),
  m_sprite(),
  m_current_action()
{
  if (!m_path.empty())
    m_position = m_path.front().position;

  m_sprite = SpriteManager::current()->create("images/creatures/tux/tux.sprite");

  if (m_sprite && m_sprite->load_successful())
  {
    m_sprite->set_alpha(0.65f);
    m_sprite->set_color(Color(0.6f, 0.9f, 1.0f, 1.0f));

    if (m_sprite->has_action(BOOTSTRAP_ACTION))
    {
      m_sprite->set_action(BOOTSTRAP_ACTION, -1);
      m_current_action = BOOTSTRAP_ACTION;
    }

    if (!m_path.empty())
      apply_action(m_path.front().action);
  }
}

void
GhostReplay::update(float dt_sec)
{
  if (m_path.empty())
    return;

  const float end_time = m_path.back().time;
  float new_time = m_time + dt_sec;
  if (new_time > end_time)
    new_time = end_time;

  m_position = sample_position(new_time);
  apply_action(sample_action(new_time));
  m_time = new_time;
}

void
GhostReplay::draw(DrawingContext& context)
{
  if (m_path.empty())
    return;

  if (m_sprite && m_sprite->load_successful())
  {
    m_sprite->draw(context.color(), m_position, LAYER_OBJECTS + 2);
  }
  else
  {
    const float ghost_size = 16.0f;
    const Vector ghost_pos = m_position + Vector(-ghost_size * 0.5f, -ghost_size * 0.5f);
    const Rectf rect(ghost_pos, Vector(ghost_size, ghost_size));
    context.color().draw_filled_rect(rect, Color(0.2f, 0.8f, 1.0f, 0.45f), LAYER_OBJECTS + 2);
  }
}

Vector
GhostReplay::sample_position(float time) const
{
  assert(!m_path.empty());

  if (time <= m_path.front().time)
    return m_path.front().position;

  if (time >= m_path.back().time)
    return m_path.back().position;

  const auto it = std::upper_bound(m_path.begin(), m_path.end(), time,
    [](float value, const worldmap::LevelTile::GhostRunPoint& point) {
      return value < point.time;
    });

  if (it == m_path.end())
    return m_path.back().position;

  const auto& next = *it;
  const auto& previous = *(it - 1);
  const float segment_duration = next.time - previous.time;
  const float segment_progress = segment_duration > 0.0f ? (time - previous.time) / segment_duration : 0.0f;
  return previous.position + (next.position - previous.position) * segment_progress;
}

const std::string&
GhostReplay::sample_action(float time) const
{
  assert(!m_path.empty());

  if (time <= m_path.front().time)
    return m_path.front().action;

  if (time >= m_path.back().time)
    return m_path.back().action;

  const auto it = std::upper_bound(
    m_path.begin(), 
    m_path.end(), 
    time,
    [](
      float value, const worldmap::LevelTile::GhostRunPoint& point) {
        return value < point.time;
    });

  return (it == m_path.begin() ? *it : *(it - 1)).action;
}

void
GhostReplay::apply_action(const std::string& action)
{
  if (action == m_current_action)
    return;
  if (!m_sprite || !m_sprite->load_successful())
    return;
  if (!m_sprite->has_action(action))
    return;

  m_sprite->set_action(action, -1);
  m_current_action = action;
}
