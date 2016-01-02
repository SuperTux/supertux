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
#include "util/reader_mapping.hpp"

SequenceTrigger::SequenceTrigger(const ReaderMapping& reader) :
  triggerevent(),
  sequence(SEQ_ENDSEQUENCE)
{
  if (!reader.get("x", bbox.p1.x)) bbox.p1.x = 0;
  if (!reader.get("y", bbox.p1.y)) bbox.p1.y = 0;
  float w, h;
  if (!reader.get("width", w)) w = 0;
  if (!reader.get("height", h)) h = 0;
  bbox.set_size(w, h);
  std::string sequence_name;
  if (reader.get("sequence", sequence_name)) {
    sequence = string_to_sequence(sequence_name);
  }
  triggerevent = EVENT_TOUCH;
}

SequenceTrigger::SequenceTrigger(const Vector& pos, const std::string& sequence_name) :
  triggerevent(),
  sequence(SEQ_ENDSEQUENCE)
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

/* EOF */
