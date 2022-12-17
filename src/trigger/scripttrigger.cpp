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

#include "trigger/scripttrigger.hpp"

#include "editor/editor.hpp"
#include "supertux/debug.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

ScriptTrigger::ScriptTrigger(const ReaderMapping& reader) :
  TriggerBase(reader),
  triggerevent(),
  script(),
  exit_script(),
  new_size(0.0f, 0.0f),
  must_activate(false),
  oneshot(false),
  has_exit_script(false),
  runcount(0)
{
  if (m_col.m_bbox.get_width() == 0.f)
    m_col.m_bbox.set_width(32.f);

  if (m_col.m_bbox.get_height() == 0.f)
    m_col.m_bbox.set_height(32.f);

  reader.get("script", script);
  reader.get("exit-script", exit_script);
  reader.get("button", must_activate);
  reader.get("oneshot", oneshot);
  reader.get("has-exit-script", has_exit_script);
  if (script.empty()) {
    log_warning << "No script set in script trigger" << std::endl;
  }

  if (must_activate)
    triggerevent = EVENT_ACTIVATE;
  else
    triggerevent = EVENT_TOUCH;
}

ScriptTrigger::ScriptTrigger(const Vector& pos, const std::string& script_) :
  TriggerBase(),
  triggerevent(EVENT_TOUCH),
  script(script_),
  exit_script(),
  new_size(0.0f, 0.0f),
  must_activate(),
  oneshot(false),
  has_exit_script(false),
  runcount(0)
{
  m_col.m_bbox.set_pos(pos);
  m_col.m_bbox.set_size(32, 32);
}

ObjectSettings
ScriptTrigger::get_settings()
{
  new_size.x = m_col.m_bbox.get_width();
  new_size.y = m_col.m_bbox.get_height();

  ObjectSettings result = TriggerBase::get_settings();

  result.add_script(_("Script"), &script, "script");
  result.add_script(_("Exit Script"), &exit_script, "exit-script");
  result.add_bool(_("Button"), &must_activate, "button");
  result.add_bool(_("Oneshot"), &oneshot, "oneshot", false);
  result.add_bool(_("Has Exit Script"), &has_exit_script, "has-exit-script", false);

  result.reorder({"script", "exit-script", "has-exit-script", "button", "width", "height", "x", "y"});

  return result;
}

void
ScriptTrigger::after_editor_set() {
  //m_col.m_bbox.set_size(new_size.x, new_size.y);
  if (must_activate) {
    triggerevent = EVENT_ACTIVATE;
  } else {
    triggerevent = EVENT_TOUCH;
  }
}

void
ScriptTrigger::event(Player& , EventType type)
{
  if (oneshot && runcount >= 1) {
    return;
  }

  if (!has_exit_script) {
    if (type != triggerevent)
      return;
  }

  Sector::get().run_script(type == EVENT_LOSETOUCH ? exit_script : script, "ScriptTrigger");
  runcount++;
}

void
ScriptTrigger::draw(DrawingContext& context)
{
  if (Editor::is_active() || g_debug.show_collision_rects) {
    context.color().draw_filled_rect(m_col.m_bbox, Color(1.0f, 0.0f, 1.0f, 0.6f),
                             0.0f, LAYER_OBJECTS);
  }
}

/* EOF */
