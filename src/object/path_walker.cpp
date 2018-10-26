//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "object/path_walker.hpp"

#include "editor/editor.hpp"
#include "editor/object_option.hpp"
#include "math/random.hpp"
#include "util/gettext.hpp"

#include <math.h>
#include <assert.h>

PathWalker::PathWalker(const Path* path_, bool running_) :
  m_path(path_),
  m_running(running_),
  m_current_node_nr(0),
  m_next_node_nr(m_path->m_nodes.size() > 1 ? 1 : 0),
  m_stop_at_node_nr(m_running?-1:0),
  m_node_time(0),
  m_node_mult(1 / m_path->m_nodes[0].time),
  m_walking_speed(1.0)
{
}

PathWalker::~PathWalker()
{
}

Vector
PathWalker::advance(float elapsed_time)
{
  if (!m_path->is_valid()) return Vector(0, 0);
  if (Editor::is_active()) {
    Vector pos__ = m_path->m_nodes.begin()->position;
//    log_warning << "x" << pos__.x << " y" << pos__.y << std::endl;
    return pos__;
  }

  if (!m_running) return m_path->m_nodes[m_current_node_nr].position;

  assert(elapsed_time >= 0);

  elapsed_time *= fabsf(m_walking_speed);

  while(m_node_time + elapsed_time * m_node_mult >= 1) {
    elapsed_time -= (1 - m_node_time) / m_node_mult;

    if (m_walking_speed > 0) {
      advance_node();
    } else if (m_walking_speed < 0) {
      goback_node();
    }

    auto current_node = & (m_path->m_nodes[m_current_node_nr]);
    m_node_time = 0;
    if (m_walking_speed > 0) {
      m_node_mult = 1 / current_node->time;
    } else {
      m_node_mult = 1 / m_path->m_nodes[m_next_node_nr].time;
    }
  }

  m_node_time += elapsed_time * m_node_mult;

  return get_pos();
}

Vector
PathWalker::get_pos() const
{
  if (!m_path->is_valid()) return Vector(0, 0);
  if (Editor::is_active()) return m_path->m_nodes.begin()->position;

  const Path::Node* current_node = & (m_path->m_nodes[m_current_node_nr]);
  const Path::Node* next_node = & (m_path->m_nodes[m_next_node_nr]);
  Vector new_pos = current_node->position +
    (next_node->position - current_node->position) * m_node_time;

  return new_pos;
}

void
PathWalker::goto_node(int node_no)
{
  if (m_path->m_mode == Path::UNORDERED && m_running) return;
  if (node_no == m_stop_at_node_nr) return;
  m_running = true;
  m_stop_at_node_nr = node_no;

  if (m_path->m_mode == Path::UNORDERED) {
    m_next_node_nr = node_no;
  }
}

void
PathWalker::start_moving()
{
  m_running = true;
  m_stop_at_node_nr = -1;
}

void
PathWalker::stop_moving()
{
  m_stop_at_node_nr = static_cast<int>(m_next_node_nr);
}

void
PathWalker::advance_node()
{
  if (!m_path->is_valid()) return;

  m_current_node_nr = m_next_node_nr;
  if (static_cast<int>(m_current_node_nr) == m_stop_at_node_nr) m_running = false;

  if (m_path->m_mode == Path::UNORDERED) {
    m_next_node_nr = gameRandom.rand( static_cast<int>(m_path->m_nodes.size()) );
    return;
  }

  if (m_next_node_nr + 1 < m_path->m_nodes.size()) {
    m_next_node_nr++;
    return;
  }

  switch(m_path->m_mode) {
    case Path::ONE_SHOT:
      m_next_node_nr = m_path->m_nodes.size() - 1;
      m_walking_speed = 0;
      return;

    case Path::PING_PONG:
      m_walking_speed = -m_walking_speed;
      m_next_node_nr = m_path->m_nodes.size() > 1 ? m_path->m_nodes.size() - 2 : 0;
      return;

    case Path::CIRCULAR:
      m_next_node_nr = 0;
      return;

    case Path::UNORDERED:
      return;
  }

  // we shouldn't get here
  assert(false);
  m_next_node_nr = m_path->m_nodes.size() - 1;
  m_walking_speed = 0;
}

void
PathWalker::goback_node()
{
  if (!m_path->is_valid()) return;

  m_current_node_nr = m_next_node_nr;

  if (m_next_node_nr > 0) {
    m_next_node_nr--;
    return;
  }

  switch(m_path->m_mode) {
    case Path::PING_PONG:
      m_walking_speed = -m_walking_speed;
      m_next_node_nr = m_path->m_nodes.size() > 1 ? 1 : 0;
      return;
    default:
      break;
  }

  assert(false);
  m_next_node_nr = 0;
  m_walking_speed = 0;
}

ObjectOption
PathWalker::get_running_option(bool* _running) {
  ObjectOption result(MN_TOGGLE, _("Running"), _running);
  return result;
}

/* EOF */
