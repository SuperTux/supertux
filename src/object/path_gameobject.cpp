//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "object/path_gameobject.hpp"

#include <boost/optional.hpp>

#include "object/path.hpp"
#include "video/drawing_context.hpp"
#include "video/color.hpp"
#include "util/log.hpp"

PathGameObject::PathGameObject() :
  m_path(new Path)
{
}

PathGameObject::PathGameObject(const Vector& pos) :
  m_path(new Path(pos))
{
}

PathGameObject::PathGameObject(const ReaderMapping& mapping) :
  GameObject(mapping),
  m_path(new Path)
{
  m_path->read(mapping);
}

void
PathGameObject::update(float elapsed_time)
{
  // nothing to do
}

void
PathGameObject::draw(DrawingContext& context)
{
  boost::optional<Vector> previous_node = boost::none;
  for(const auto& node : m_path->get_nodes())
  {
    if (previous_node)
    {
      context.color().draw_line(*previous_node, node.position, Color::MAGENTA, LAYER_OBJECTS - 2);
    }

    context.color().draw_filled_rect(Rectf::from_center(node.position, Sizef(16.0f, 16.0f)),
                                     Color::BLUE, LAYER_OBJECTS - 1);

    previous_node = node.position;
  }
}

/* EOF */
