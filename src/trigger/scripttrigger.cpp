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
#include "object/player.hpp"
#include "supertux/debug.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

ScriptTrigger::ScriptTrigger(const ReaderMapping& reader) :
  Trigger(reader),
  triggerevent(),
  m_trigger_direction(Direction::AUTO),
  script(),
  must_activate(false),
  oneshot(false),
  runcount(0)
{
  std::string trigger_direction;
  reader.get("trigger-direction", trigger_direction, dir_to_string(Direction::AUTO).c_str());
  m_trigger_direction = string_to_dir(trigger_direction);

  reader.get("script", script);
  reader.get("button", must_activate);
  reader.get("oneshot", oneshot);
  if (script.empty() && !Editor::is_active())
    log_warning << "No script set in script trigger" << std::endl;

  if (must_activate)
    triggerevent = EVENT_ACTIVATE;
  else
    triggerevent = EVENT_TOUCH;
}

ObjectSettings
ScriptTrigger::get_settings()
{
  ObjectSettings result = Trigger::get_settings();

  result.add_script(_("Script"), &script, "script");
  result.add_bool(_("Button"), &must_activate, "button");
  result.add_bool(_("Oneshot"), &oneshot, "oneshot", false);
  result.add_direction(_("Trigger Direction"), &m_trigger_direction,
                        { Direction::AUTO, Direction::LEFT, Direction::RIGHT, Direction::UP, Direction::DOWN }, "trigger-direction");

  result.reorder({"script", "button", "width", "height", "x", "y"});

  return result;
}

void
ScriptTrigger::event(Player& player, EventType type)
{
  if (type != triggerevent || (oneshot && runcount >= 1))
    return;

  if(m_trigger_direction != Direction::AUTO)
  {
    const auto& pos = get_pos();
    const auto& player_pos = player.get_pos();
    Direction player_direction = Direction::NONE;

    if (player_pos.x < pos.x)
      player_direction = Direction::LEFT;
    if (player_pos.x + player.get_width() > pos.x + get_width())
      player_direction = Direction::RIGHT;
    if (player_pos.y < pos.y)
      player_direction = Direction::UP;
    if (player_pos.y + player.get_height() > pos.y + get_height())
      player_direction = Direction::DOWN;

    if (player_direction != m_trigger_direction)
      return;
  }

  Sector::get().run_script(script, "ScriptTrigger");
  runcount++;
}

void
ScriptTrigger::draw(DrawingContext& context)
{
  if (Editor::is_active() || g_debug.show_collision_rects)
    context.color().draw_filled_rect(m_col.m_bbox, Color(1.0f, 0.0f, 1.0f, 0.6f),
                             0.0f, LAYER_OBJECTS);
}

/* EOF */
