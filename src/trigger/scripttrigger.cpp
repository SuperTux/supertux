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
  Trigger(Color(1.0f, 0.0f, 1.0f, 0.6f), reader),
  m_triggerevent(),
  m_script(),
  m_leave_script(),
  m_must_activate(false),
  m_oneshot(false),
  m_runcount(0)
{
  reader.get("script", m_script);
  reader.get("leave-script", m_leave_script);
  reader.get("button", m_must_activate);
  reader.get("oneshot", m_oneshot);
  if (m_script.empty() && !Editor::is_active())
    log_warning << "No script set in script trigger" << std::endl;

  if (m_must_activate)
    m_triggerevent = EVENT_ACTIVATE;
  else
    m_triggerevent = EVENT_TOUCH;
}

ObjectSettings
ScriptTrigger::get_settings()
{
  ObjectSettings result = Trigger::get_settings();

  result.add_script(get_uid(), _("Script"), &m_script, "script");
  result.add_script(get_uid(), _("Leave script"), &m_leave_script, "leave-script");
  result.add_bool(_("Button"), &m_must_activate, "button");
  result.add_bool(_("Oneshot"), &m_oneshot, "oneshot", false);

  result.reorder({"script", "leave-script", "button", "width", "height", "x", "y"});

  return result;
}

void
ScriptTrigger::event(Player& , EventType type)
{
  if ((type != m_triggerevent && !(m_triggerevent == EVENT_TOUCH && type == EVENT_LOSETOUCH)) ||
      (m_oneshot && m_runcount >= 1))
    return;

  if (type == EVENT_LOSETOUCH) Sector::get().run_script(m_leave_script, "ScriptTrigger");
  else Sector::get().run_script(m_script, "ScriptTrigger");

  //for non-button triggers. only increase the counter upon leaving the trigger to avoid double increase
  if (!(m_triggerevent == EVENT_TOUCH && type == EVENT_TOUCH)) m_runcount++;
}
