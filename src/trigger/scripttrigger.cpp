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

#include "editor/editor.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "trigger/scripttrigger.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

ScriptTrigger::ScriptTrigger(const ReaderMapping& reader) :
  triggerevent(),
  script(),
  new_size(),
  must_activate(false),
  oneshot(false),
  runcount(0)
{
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  float w = 32, h = 32;
  reader.get("width", w);
  reader.get("height", h);
  bbox.set_size(w, h);
  reader.get("script", script);
  reader.get("button", must_activate);
  reader.get("oneshot", oneshot);
  if(script.empty()) {
    log_warning << "No script set in script trigger" << std::endl;
  }

  if (must_activate)
    triggerevent = EVENT_ACTIVATE;
  else
    triggerevent = EVENT_TOUCH;
}

ScriptTrigger::ScriptTrigger(const Vector& pos, const std::string& script_) :
  triggerevent(EVENT_TOUCH),
  script(script_),
  new_size(),
  must_activate(),
  oneshot(false),
  runcount(0)
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);
}

ScriptTrigger::~ScriptTrigger()
{
}

ObjectSettings
ScriptTrigger::get_settings() {
  new_size.x = bbox.get_width();
  new_size.y = bbox.get_height();
  ObjectSettings result(_("Script trigger"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Width"), &new_size.x, "width"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Height"), &new_size.y, "height"));
  result.options.push_back( ObjectOption(MN_SCRIPT, _("Script"), &script, "script"));
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Button"), &must_activate, "button"));
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Oneshot"), &oneshot, "oneshot"));
  return result;
}

void
ScriptTrigger::after_editor_set() {
  bbox.set_size(new_size.x, new_size.y);
  if (must_activate) {
    triggerevent = EVENT_ACTIVATE;
  } else {
    triggerevent = EVENT_TOUCH;
  }
}

void
ScriptTrigger::event(Player& , EventType type)
{
  if(type != triggerevent)
    return;

  if (oneshot && runcount >= 1) {
    return;
  }

  Sector::current()->run_script(script, "ScriptTrigger");
  runcount++;
}

void
ScriptTrigger::draw(DrawingContext& context)
{
  if (Editor::is_active()) {
    context.draw_filled_rect(bbox, Color(1.0f, 0.0f, 1.0f, 0.6f),
                             0.0f, LAYER_OBJECTS);
  }
}

/* EOF */
