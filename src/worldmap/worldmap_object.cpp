//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
//                2023 Vankata453
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

#include "worldmap/worldmap_object.hpp"

#include "supertux/d_scope.hpp"
#include "util/gettext.hpp"
#include "util/reader_mapping.hpp"
#include "worldmap/worldmap_sector.hpp"

namespace worldmap {

bool
WorldMapObject::in_worldmap()
{
  return d_gameobject_manager.get() == d_worldmap_sector.get();
}


WorldMapObject::WorldMapObject(const ReaderMapping& mapping, const std::string& default_sprite) :
  MovingSprite(mapping, default_sprite),
  m_tile_x(),
  m_tile_y()
{
  initialize();
}

WorldMapObject::WorldMapObject(const ReaderMapping& mapping) :
  MovingSprite(mapping),
  m_tile_x(),
  m_tile_y()
{
  initialize();
}

WorldMapObject::WorldMapObject(const Vector& pos, const std::string& default_sprite) :
  MovingSprite(pos, default_sprite),
  m_tile_x(),
  m_tile_y()
{
  initialize();
}

void
WorldMapObject::initialize()
{
  m_layer = LAYER_OBJECTS - (in_worldmap() ? 1 : 0);

  m_col.m_bbox.set_left(32 * m_col.m_bbox.get_left());
  m_col.m_bbox.set_top(32 * m_col.m_bbox.get_top());
  m_col.m_bbox.set_size(32, 32);
  update_pos();
}

ObjectSettings
WorldMapObject::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();

  result.remove("x");
  result.remove("y");

  result.add_int(_("X"), &m_tile_x, "x", {}, OPTION_HIDDEN);
  result.add_int(_("Y"), &m_tile_y, "y", {}, OPTION_HIDDEN);

  return result;
}

void
WorldMapObject::draw(DrawingContext& context)
{
  if (in_worldmap())
    draw_worldmap(context);
  else if (m_sprite)
    m_sprite->draw(context.color(), m_col.m_bbox.p1() + get_editor_offset(), m_layer);
}

void
WorldMapObject::draw_worldmap(DrawingContext& context)
{
  if (!m_sprite) return;

  m_sprite->draw(context.color(), m_col.m_bbox.p1() + get_editor_offset(), m_layer);
}

void
WorldMapObject::update(float dt_sec)
{
  update_pos();
}

void
WorldMapObject::update_pos()
{
  m_tile_x = static_cast<int>(m_col.m_bbox.get_left()) / 32;
  m_tile_y = static_cast<int>(m_col.m_bbox.get_top()) / 32;
}

void
WorldMapObject::move_to(const Vector& pos)
{
  set_pos(Vector(32.0f * static_cast<float>(pos.x / 32),
                 32.0f * static_cast<float>(pos.y / 32)));
  update_pos();
}

} // namespace worldmap

/* EOF */
