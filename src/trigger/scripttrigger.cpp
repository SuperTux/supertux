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

#include <memory>
#include <sstream>
#include <stdexcept>

#include "lisp/lisp.hpp"
#include "lisp/writer.hpp"
#include "supertux/game_session.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "trigger/scripttrigger.hpp"

ScriptTrigger::ScriptTrigger(const lisp::Lisp& reader)
{
  bool must_activate = false;

  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  float w = 0, h = 0;
  reader.get("width", w);
  reader.get("height", h);
  bbox.set_size(w, h);
  reader.get("script", script);
  reader.get("button", must_activate);
  if(script == "") {
    throw std::runtime_error("Need to specify a script for trigger object");
  }

  if (must_activate)
    triggerevent = EVENT_ACTIVATE;
  else
    triggerevent = EVENT_TOUCH;
}

ScriptTrigger::ScriptTrigger(const Vector& pos, const std::string& script)
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);
  this->script = script;
  triggerevent = EVENT_TOUCH;
}

ScriptTrigger::~ScriptTrigger()
{
}

void
ScriptTrigger::write(lisp::Writer& writer)
{
  writer.start_list("scripttrigger");

  writer.write("x", bbox.p1.x);
  writer.write("y", bbox.p1.y);
  writer.write("width", bbox.get_width());
  writer.write("height", bbox.get_height());
  writer.write("script", script);
  writer.write("button", triggerevent == EVENT_ACTIVATE);

  writer.end_list("scripttrigger");
}

void
ScriptTrigger::event(Player& , EventType type)
{
  if(type != triggerevent)
    return;

  std::istringstream stream(script);
  Sector::current()->run_script(stream, "ScriptTrigger");
}

IMPLEMENT_FACTORY(ScriptTrigger, "scripttrigger");
