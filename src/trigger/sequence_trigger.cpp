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

#include <config.h>

#include "trigger/sequence_trigger.hpp"
#include "supertux/game_session.hpp"
#include "lisp/lisp.hpp"
#include "lisp/writer.hpp"
#include "supertux/object_factory.hpp"

SequenceTrigger::SequenceTrigger(const lisp::Lisp& reader)
{
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  float w = 0, h = 0;
  reader.get("width", w);
  reader.get("height", h);
  bbox.set_size(w, h);
  reader.get("sequence", sequence_name);
  triggerevent = EVENT_TOUCH;
}

SequenceTrigger::SequenceTrigger(const Vector& pos, const std::string& sequence)
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);
  sequence_name = sequence;
  triggerevent = EVENT_TOUCH;
}

SequenceTrigger::~SequenceTrigger()
{
}

void
SequenceTrigger::write(lisp::Writer& writer)
{
  writer.start_list("sequencetrigger");

  writer.write("x", bbox.p1.x);
  writer.write("y", bbox.p1.y);
  writer.write("width", bbox.get_width());
  writer.write("height", bbox.get_height());
  writer.write("sequence", sequence_name);

  writer.end_list("sequencetrigger");
}

void
SequenceTrigger::event(Player& player, EventType type)
{
  if(type == triggerevent) {
    player.trigger_sequence(sequence_name);
  }
}

IMPLEMENT_FACTORY(SequenceTrigger, "sequencetrigger");

