//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
//
#include <config.h>

#include <sstream>

#include "scripttrigger.h"
#include "game_session.h"
#include "lisp/lisp.h"
#include "lisp/writer.h"
#include "object_factory.h"
#include "scripting/script_interpreter.h"
#include "sector.h"

ScriptTrigger::ScriptTrigger(const lisp::Lisp& reader)
{
  bool must_activate;
  
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  float w, h;
  reader.get("width", w);
  reader.get("height", h);
  bbox.set_size(w, h);
  reader.get("script", script);
  reader.get("button", must_activate);
  
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

  writer.write_float("x", bbox.p1.x);
  writer.write_float("y", bbox.p1.y);
  writer.write_float("width", bbox.get_width());
  writer.write_float("height", bbox.get_height());
  writer.write_string("script", script);
  writer.write_bool("button", (triggerevent == EVENT_ACTIVATE) ? true : false);

  writer.end_list("scripttrigger");
}

void
ScriptTrigger::event(Player& , EventType type)
{
  if(type == triggerevent)
  {
    if (script != "")
    {
      try
      {
        ScriptInterpreter* interpreter 
          = new ScriptInterpreter(GameSession::current()->get_working_directory());
        interpreter->register_sector(Sector::current());
        std::istringstream in(script);
        interpreter->load_script(in, "trigger-script");
        interpreter->start_script();
        Sector::current()->add_object(interpreter);
      }
      catch(std::exception& e)
      {
          std::cerr << "Couldn't execute trigger script: " << e.what() << "\n";
      }
    }
    else
    {
      std::cerr << "Couldn't find trigger script.\n";
    }
  }
}

IMPLEMENT_FACTORY(ScriptTrigger, "scripttrigger");

