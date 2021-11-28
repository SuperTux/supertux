//  SuperTux
//  Copyright (C) 2021 mrkubax10 <mrkubax10@onet.pl>
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

#include "object/text_area.hpp"

#include "editor/editor.hpp"
#include "object/player.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"
#include "video/layer.hpp"

TextArea::TextArea(const ReaderMapping& mapping) :
  MovingObject(mapping),
  m_started(false)
{
  float w, h;
  mapping.get("x", m_col.m_bbox.get_left(), 0.0f);
  mapping.get("y", m_col.m_bbox.get_top(), 0.0f);
  mapping.get("width", w, 32.0f);
  mapping.get("height", h, 32.0f);
  m_col.m_bbox.set_size(w, h);
  m_col.m_group = COLGROUP_DISABLED;
}

TextArea::TextArea(const Vector& pos) :
  m_started(false)
{
  m_col.m_bbox.set_pos(pos);
  m_col.m_bbox.set_size(32,32);
  m_col.m_group = COLGROUP_DISABLED;
}

HitResponse
TextArea::collision(GameObject& other, const CollisionHit& hit)
{
  return FORCE_MOVE;
}

void
TextArea::draw(DrawingContext& context)
{
  if(Editor::is_active())
  {
    context.color().draw_filled_rect(m_col.m_bbox, Color(1.0f, 1.0f, 1.0f, 0.6f), LAYER_OBJECTS);
  }
}

void
TextArea::update(float dt_sec)
{
  if (m_col.m_bbox.contains(Sector::get().get_player().get_bbox()) && !m_started)
  {

  }
}

ObjectSettings
TextArea::get_settings()
{
  ObjectSettings settings = MovingObject::get_settings();
  return settings;
}