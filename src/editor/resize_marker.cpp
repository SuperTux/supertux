//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#include <algorithm>

#include "editor/resize_marker.hpp"

ResizeMarker::ResizeMarker(Rectf* rect, Side vert, Side horz) :
  m_rect(rect),
  m_vert(vert),
  m_horz(horz)
{
  refresh_pos();
}

void
ResizeMarker::update(float dt_sec)
{
  refresh_pos();
}

void
ResizeMarker::refresh_pos()
{
  Vector new_pos;
  switch (m_vert) {
    case NONE:
      new_pos.y = (m_rect->p1.y + m_rect->p2.y)/2 - 8;
      break;
    case LEFT_UP:
      new_pos.y = m_rect->p1.y - 16;
      break;
    case RIGHT_DOWN:
      new_pos.y = m_rect->p2.y;
      break;
  }

  switch (m_horz) {
    case NONE:
      new_pos.x = (m_rect->p1.x + m_rect->p2.x)/2 - 8;
      break;
    case LEFT_UP:
      new_pos.x = m_rect->p1.x - 16;
      break;
    case RIGHT_DOWN:
      new_pos.x = m_rect->p2.x;
      break;
  }

  set_pos(new_pos);
}

void
ResizeMarker::move_to(const Vector& pos)
{
  switch (m_vert) {
    case NONE:
      break;
    case LEFT_UP:
      m_rect->p1.y = std::min(pos.y + 16, m_rect->p2.y - 2);
      break;
    case RIGHT_DOWN:
      m_rect->p2.y = std::max(pos.y, m_rect->p1.y + 2);
      break;
  }

  switch (m_horz) {
    case NONE:
      break;
    case LEFT_UP:
      m_rect->p1.x = std::min(pos.x + 16, m_rect->p2.x - 2);
      break;
    case RIGHT_DOWN:
      m_rect->p2.x = std::max(pos.x, m_rect->p1.x + 2);
      break;
  }

  refresh_pos();
}

Vector
ResizeMarker::get_point_vector() const
{
  Vector result;

  switch (m_vert) {
    case NONE:
      result.y = 0;
      break;
    case LEFT_UP:
      result.y = -1;
      break;
    case RIGHT_DOWN:
      result.y = 1;
      break;
  }

  switch (m_horz) {
    case NONE:
      result.x = 0;
      break;
    case LEFT_UP:
      result.x = -1;
      break;
    case RIGHT_DOWN:
      result.x = 1;
      break;
  }

  return result;
}

Vector
ResizeMarker::get_offset() const
{
  return Vector((m_horz == LEFT_UP) ? 16 : 0, (m_vert == LEFT_UP) ? 16 : 0);
}

/* EOF */
