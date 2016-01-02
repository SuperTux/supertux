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

#include <memory>
#include <sstream>
#include <stdexcept>

#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "trigger/scripttrigger.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"

ScriptTrigger::ScriptTrigger(const ReaderMapping& reader) :
  triggerevent(),
  script()
{
  bool must_activate = false;

  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  float w = 32, h = 32;
  reader.get("width", w);
  reader.get("height", h);
  bbox.set_size(w, h);
  reader.get("script", script);
  reader.get("button", must_activate);
  if(script.empty()) {
    log_warning << "No script set in script trigger" << std::endl;
  }

  if (must_activate)
    triggerevent = EVENT_ACTIVATE;
  else
    triggerevent = EVENT_TOUCH;
}

ScriptTrigger::ScriptTrigger(const Vector& pos, const std::string& script_) :
  triggerevent(),
  script()
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);
  this->script = script_;
  triggerevent = EVENT_TOUCH;
}

ScriptTrigger::~ScriptTrigger()
{
}

void
ScriptTrigger::save(Writer& writer) {
  MovingObject::save(writer);
  writer.write("width", bbox.get_width());
  writer.write("height", bbox.get_height());
  writer.write("script", script, false);
  writer.write("button", triggerevent == EVENT_ACTIVATE);
}

void
ScriptTrigger::event(Player& , EventType type)
{
  if(type != triggerevent)
    return;

  std::istringstream stream(script);
  Sector::current()->run_script(stream, "ScriptTrigger");
}

/* EOF */
