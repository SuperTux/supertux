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
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "video/drawing_context.hpp"

SequenceTrigger::SequenceTrigger(const ReaderMapping& reader) :
  triggerevent(EVENT_TOUCH),
  sequence(SEQ_ENDSEQUENCE),
  new_size(),
  new_spawnpoint(),
  fade_tilemap(),
  fade()
{
  reader.get("x", bbox.p1.x, 0);
  reader.get("y", bbox.p1.y, 0);
  float w, h;
  reader.get("width", w, 32);
  reader.get("height", h, 32);
  bbox.set_size(w, h);
  std::string sequence_name;
  if (reader.get("sequence", sequence_name)) {
    sequence = string_to_sequence(sequence_name);
  }

  reader.get("new_spawnpoint", new_spawnpoint);
  reader.get("fade_tilemap", fade_tilemap);
  reader.get("fade", (int&)fade);
}

SequenceTrigger::SequenceTrigger(const Vector& pos, const std::string& sequence_name) :
  triggerevent(EVENT_TOUCH),
  sequence(string_to_sequence(sequence_name)),
  new_size(),
  new_spawnpoint(),
  fade_tilemap(),
  fade()
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);
}

void
SequenceTrigger::save(Writer& writer) {
  MovingObject::save(writer);
  writer.write("sequence", sequence_to_string(sequence), false);
}

ObjectSettings
SequenceTrigger::get_settings() {
  new_size.x = bbox.get_width();
  new_size.y = bbox.get_height();
  ObjectSettings result(_("Sequence trigger"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Width"), &new_size.x, "width"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Height"), &new_size.y, "height"));

  ObjectOption seq(MN_STRINGSELECT, _("Sequence"), &sequence);
  seq.select.push_back(_("end sequence"));
  seq.select.push_back(_("stop Tux"));
  seq.select.push_back(_("fireworks"));

  result.options.push_back( seq );

  result.options.push_back(ObjectOption(MN_TEXTFIELD, _("New worldmap spawnpoint"), &new_spawnpoint, "new_spawnpoint"));
  result.options.push_back(ObjectOption(MN_TEXTFIELD, _("Worldmap fade tilemap"), &fade_tilemap, "fade_tilemap"));
  ObjectOption fade_toggle(MN_STRINGSELECT, _("Fade"), &fade, "fade");
  fade_toggle.select.push_back(_("Fade in"));
  fade_toggle.select.push_back(_("Fade out"));
  result.options.push_back(fade_toggle);
  return result;
}

void
SequenceTrigger::after_editor_set() {
  bbox.set_size(new_size.x, new_size.y);
}

void
SequenceTrigger::event(Player& player, EventType type)
{
  if(type == triggerevent) {
    auto data = SequenceData(new_spawnpoint, fade_tilemap, fade);
    player.trigger_sequence(sequence, &data);
  }
}

std::string
SequenceTrigger::get_sequence_name() const {
  return sequence_to_string(sequence);
}

void
SequenceTrigger::draw(DrawingContext& context)
{
  if (Editor::is_active()) {
    context.color().draw_filled_rect(bbox, Color(1.0f, 0.0f, 0.0f, 0.6f),
                             0.0f, LAYER_OBJECTS);
  }
}

/* EOF */
