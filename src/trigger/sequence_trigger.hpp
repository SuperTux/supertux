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

#ifndef HEADER_SUPERTUX_TRIGGER_SEQUENCE_TRIGGER_HPP
#define HEADER_SUPERTUX_TRIGGER_SEQUENCE_TRIGGER_HPP

#include "trigger/trigger_base.hpp"
#include "util/serializable.hpp"

class Player;

class SequenceTrigger : public TriggerBase, 
                        public Serializable
{
public:
  SequenceTrigger(const lisp::Lisp& reader);
  SequenceTrigger(const Vector& pos, const std::string& sequence);
  ~SequenceTrigger();

  void write(lisp::Writer& writer);
  void event(Player& player, EventType type);

  std::string get_sequence_name() const { return sequence_name; }

private:
  EventType triggerevent;
  std::string sequence_name;
};

#endif

/* EOF */
