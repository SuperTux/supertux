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
#include "supertux/debug.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "util/unique_name.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"

PathStyle PathStyle_from_string(const std::string& text)
{
  if (text == "none") {
    return PathStyle::NONE;
  } else if (text == "solid") {
    return PathStyle::SOLID;
  } else {
    log_warning << "unknown PathStyle: " << text << std::endl;
    return PathStyle::NONE;
  }
}

PathGameObject::PathGameObject() :
  m_path(new Path),
  m_style(PathStyle::NONE)
{
  m_name = make_unique_name("path", this);
}

PathGameObject::PathGameObject(const Vector& pos) :
  m_path(new Path(pos)),
  m_style(PathStyle::NONE)
{
  m_name = make_unique_name("path", this);
}

PathGameObject::PathGameObject(const ReaderMapping& mapping) :
  GameObject(mapping),
  m_path(new Path),
  m_style(PathStyle::NONE)
{
  boost::optional<ReaderMapping> path_mapping;
  if (mapping.get("path", path_mapping))
  {
    m_path->read(*path_mapping);
  }

  if (m_name.empty()) {
    m_name = make_unique_name("path", this);
  }

  mapping.get_custom("style", m_style, PathStyle_from_string);
}

void
PathGameObject::update(float dt_sec)
{
  // nothing to do
}

void
PathGameObject::draw(DrawingContext& context)
{
  const bool draw_graph = g_debug.show_collision_rects || m_style == PathStyle::SOLID;

  if (draw_graph)
  {
    const bool debug = g_debug.show_collision_rects;
    const Color node_color = debug ? Color::BLUE : Color(0.75f, 0.75f, 0.75f);
    const Color edge_color = debug ? Color::MAGENTA : Color::BLACK;

    boost::optional<Vector> previous_node;
    for(const auto& node : m_path->get_nodes())
    {
      if (previous_node)
      {
        context.color().draw_line(*previous_node, node.position, edge_color, LAYER_OBJECTS - 2);
      }

      context.color().draw_filled_rect(Rectf::from_center(node.position, Sizef(16.0f, 16.0f)),
                                       node_color, LAYER_OBJECTS - 1);

      previous_node = node.position;
    }
  }
}

void
PathGameObject::save(Writer& writer)
{
  GameObject::save(writer);
  m_path->save(writer);
}

/* EOF */
