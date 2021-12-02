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

#include "trigger/text_area.hpp"

#include "editor/editor.hpp"
#include "object/player.hpp"
#include "object/text_object.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"
#include "video/layer.hpp"

TextArea::TextArea(const ReaderMapping& mapping) :
  TriggerBase(mapping),
  m_started(false),
  m_inside(false),
  m_once(false),
  m_finished(false),
  m_items(),
  m_delay(4.0f),
  m_fade_delay(1.0f),
  m_text_id(0),
  m_update_timer(),
  m_fade_timer()
{
  float w, h;
  mapping.get("x", m_col.m_bbox.get_left(), 0.0f);
  mapping.get("y", m_col.m_bbox.get_top(), 0.0f);
  mapping.get("width", w, 32.0f);
  mapping.get("height", h, 32.0f);
  mapping.get("strings", m_items);
  mapping.get("delay", m_delay);
  mapping.get("once", m_once);
  mapping.get("fade-delay", m_fade_delay);
  m_col.m_bbox.set_size(w, h);
}

TextArea::TextArea(const Vector& pos) :
  m_started(false),
  m_inside(false),
  m_once(false),
  m_finished(false),
  m_items(),
  m_delay(4.0f),
  m_fade_delay(1.0f),
  m_text_id(0),
  m_update_timer(),
  m_fade_timer()
{
  m_col.m_bbox.set_pos(pos);
  m_col.m_bbox.set_size(32,32);
}

void
TextArea::draw(DrawingContext& context)
{
  if(Editor::is_active())
    context.color().draw_filled_rect(m_col.m_bbox, Color(1.0f, 1.0f, 1.0f, 0.6f), LAYER_OBJECTS);
}

void
TextArea::event(Player& player, EventType type)
{
  switch (type)
  {
  case EVENT_TOUCH:
    if (!m_started && !m_fade_timer.started() && m_items.size() > 0 && !m_inside && (!m_once || !m_finished))
    {
      m_update_timer.start(m_delay + m_fade_delay * 2, true);
      m_started = true;
      m_text_id = 0;
    }
    m_inside = true;
    break;
  case EVENT_LOSETOUCH:
    m_inside = false;
    break;
  default:
    break;
  }
}

void
TextArea::update(float dt_sec)
{
  TriggerBase::update(dt_sec);
  if (m_started)
  {
    TextObject& text_object = Sector::get().get_singleton_by_type<TextObject>();
    if (m_text_id < m_items.size() && (m_update_timer.check() || m_text_id == 0) && !m_fade_timer.started())
    {
      m_fade_timer.start(m_delay + m_fade_delay);
      text_object.set_text(m_items[m_text_id]);
      text_object.fade_in(m_fade_delay);
      m_text_id++;
    }
    else if (m_text_id >= m_items.size())
    {
      m_started = false;
      m_update_timer.stop();
      m_fade_timer.start(m_delay + m_fade_delay);
      m_finished = true;
    }
  }
  if (m_fade_timer.check())
  {
    Sector::get().get_singleton_by_type<TextObject>().fade_out(m_fade_delay);
    m_fade_timer.stop();
  }
}

ObjectSettings
TextArea::get_settings()
{
  ObjectSettings settings = TriggerBase::get_settings();
  settings.add_bool(_("Once"), &m_once, "once");
  settings.add_float(_("Text change time"), &m_delay, "delay");
  settings.add_float(_("Fade time"), &m_fade_delay, "fade-delay");
  settings.add_string_array(_("Texts"), "texts", m_items);
  return settings;
}
/* EOF */
