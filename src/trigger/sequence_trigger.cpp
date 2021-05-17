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

#include "trigger/sequence_trigger.hpp"

#include "editor/editor.hpp"
#include "object/player.hpp"
#include "supertux/debug.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "video/drawing_context.hpp"

SequenceTrigger::SequenceTrigger(const ReaderMapping& reader) :
  triggerevent(EVENT_TOUCH),
  sequence(SEQ_ENDSEQUENCE),
  new_size(0.0f, 0.0f),
  new_spawnpoint(),
  fade_tilemap(),
  fade()
{
  reader.get("x", m_col.m_bbox.get_left(), 0.0f);
  reader.get("y", m_col.m_bbox.get_top(), 0.0f);
  float w, h;
  reader.get("width", w, 32.0f);
  reader.get("height", h, 32.0f);
  m_col.m_bbox.set_size(w, h);
  new_size.x = w;
  new_size.y = h;
  std::string sequence_name;
  if (reader.get("sequence", sequence_name)) {
    sequence = string_to_sequence(sequence_name);
  }

  reader.get("new_spawnpoint", new_spawnpoint);
  reader.get("fade_tilemap", fade_tilemap);
  reader.get("fade", reinterpret_cast<int&>(fade));
}

SequenceTrigger::SequenceTrigger(const Vector& pos, const std::string& sequence_name) :
  triggerevent(EVENT_TOUCH),
  sequence(string_to_sequence(sequence_name)),
  new_size(0.0f, 0.0f),
  new_spawnpoint(),
  fade_tilemap(),
  fade()
{
  m_col.m_bbox.set_pos(pos);
  m_col.m_bbox.set_size(32, 32);
}

ObjectSettings
SequenceTrigger::get_settings()
{
  new_size.x = m_col.m_bbox.get_width();
  new_size.y = m_col.m_bbox.get_height();

  ObjectSettings result = TriggerBase::get_settings();

  //result.add_float(_("Width"), &new_size.x, "width");
  //result.add_float(_("Height"), &new_size.y, "height");

  result.add_enum(_("Sequence"), reinterpret_cast<int*>(&sequence),
                  {_("end sequence"), _("stop Tux"), _("fireworks")},
                  {"endsequence", "stoptux", "fireworks"},
                  boost::none, "sequence");

  result.add_text(_("New worldmap spawnpoint"), &new_spawnpoint, "new_spawnpoint");
  result.add_text(_("Worldmap fade tilemap"), &fade_tilemap, "fade_tilemap");
  result.add_string_select(_("Fade"), reinterpret_cast<int*>(&fade),
                           {_("Fade in"), _("Fade out")},
                           0, "fade");

  result.reorder({"sequence", "region", "width", "height", "x", "y", "fade"});

  return result;
}

void
SequenceTrigger::after_editor_set()
{
  m_col.m_bbox.set_size(new_size.x, new_size.y);
}

void
SequenceTrigger::event(Player& player, EventType type)
{
  if (type == triggerevent) {
    auto data = SequenceData(new_spawnpoint, fade_tilemap, fade);
    player.trigger_sequence(sequence, &data);
  }
}

std::string
SequenceTrigger::get_sequence_name() const
{
  return sequence_to_string(sequence);
}

void
SequenceTrigger::draw(DrawingContext& context)
{
  if (Editor::is_active() || g_debug.show_collision_rects) {
    context.color().draw_filled_rect(m_col.m_bbox, Color(1.0f, 0.0f, 0.0f, 0.6f),
                             0.0f, LAYER_OBJECTS);
  }
}

/* EOF */
