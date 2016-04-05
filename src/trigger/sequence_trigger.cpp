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

#include <string>

#include "trigger/sequence_trigger.hpp"

#include "object/player.hpp"
#include "supertux/game_session.hpp"
#include "supertux/object_factory.hpp"
#include "util/editor_active.hpp"
#include "util/gettext.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

SequenceTrigger::SequenceTrigger(const ReaderMapping& reader) :
  triggerevent(),
  sequence(SEQ_ENDSEQUENCE),
  new_size()
{
  if (!reader.get("x", bbox.p1.x)) bbox.p1.x = 0;
  if (!reader.get("y", bbox.p1.y)) bbox.p1.y = 0;
  float w, h;
  if (!reader.get("width", w)) w = 32;
  if (!reader.get("height", h)) h = 32;
  bbox.set_size(w, h);
  std::string sequence_name;
  if (reader.get("sequence", sequence_name)) {
    sequence = string_to_sequence(sequence_name);
  }
  triggerevent = EVENT_TOUCH;
}

SequenceTrigger::SequenceTrigger(const Vector& pos, const std::string& sequence_name) :
  triggerevent(),
  sequence(SEQ_ENDSEQUENCE),
  new_size()
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);
  sequence = string_to_sequence(sequence_name);
  triggerevent = EVENT_TOUCH;
}

SequenceTrigger::~SequenceTrigger()
{
}

void
SequenceTrigger::save(Writer& writer) {
  MovingObject::save(writer);
  writer.write("width", bbox.get_width());
  writer.write("height", bbox.get_height());
  writer.write("sequence", sequence_to_string(sequence), false);
}

ObjectSettings
SequenceTrigger::get_settings() {
  new_size.x = bbox.get_width();
  new_size.y = bbox.get_height();
  ObjectSettings result(_("Sequence trigger"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Width"), &new_size.x));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Height"), &new_size.y));

  ObjectOption seq(MN_STRINGSELECT, _("Sequence"), &sequence);
  seq.select.push_back(_("end sequence"));
  seq.select.push_back(_("stop Tux"));
  seq.select.push_back(_("fireworks"));

  result.options.push_back( seq );
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
    player.trigger_sequence(sequence);
  }
}

std::string
SequenceTrigger::get_sequence_name() const {
  return sequence_to_string(sequence);
}

void
SequenceTrigger::draw(DrawingContext& context)
{
  if (EditorActive()) {
    context.draw_filled_rect(bbox, Color(1.0f, 0.0f, 0.0f, 0.6f),
                             0.0f, LAYER_OBJECTS);
  }
}

/* EOF */
