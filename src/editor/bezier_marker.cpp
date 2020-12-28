//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#include <editor/bezier_marker.hpp>

BezierMarker::BezierMarker(Path::Node* node, Vector* bezier_pos) :
  m_node(node),
  m_pos(bezier_pos)
{
  set_pos(*m_pos - Vector(8, 8));
}

Vector
BezierMarker::get_point_vector() const
{
  return *m_pos - m_node->position;
}

Vector
BezierMarker::get_offset() const
{
  return Vector(8, 8);
}

void
BezierMarker::move_to(const Vector& pos)
{
  MovingObject::move_to(pos);
  *m_pos = m_col.m_bbox.get_middle();
}

void
BezierMarker::editor_update()
{
  set_pos(*m_pos - Vector(8, 8));
}

void
BezierMarker::update_iterator(Path::Node* node, Vector* bezier_pos)
{
  m_node = node;
  m_pos = bezier_pos;
}

/* EOF */
