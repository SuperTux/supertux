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

#include "object/level_time.hpp"

#include <algorithm>

#include "editor/editor.hpp"
#include "object/player.hpp"
#include "supertux/resources.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"

/** When to alert player they're low on time! */
static const float TIME_WARNING = 20;

LevelTime::LevelTime(const ReaderMapping& reader) :
  GameObject(reader),
  ExposedObject<LevelTime, scripting::LevelTime>(this),
  time_surface(Surface::create("images/engine/hud/time-0.png")),
  running(!Editor::is_active()),
  time_left()
{
  reader.get("time", time_left, 0);
  if(time_left <= 0 && !Editor::is_active()) {
    log_warning << "No or invalid leveltime specified." << std::endl;
    remove_me();
  }
}

ObjectSettings
LevelTime::get_settings() {
  ObjectSettings result = GameObject::get_settings();
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Time"), &time_left, "time"));

  result.options.push_back( ObjectOption(MN_REMOVE, "", NULL));
  return result;
}

void
LevelTime::update(float elapsed_time)
{
  if (!running) return;

  int prev_time = (int) floor(time_left*5);
  time_left -= elapsed_time;
  if(time_left <= 0) {
    if(time_left <= -5 || !Sector::current()->player->get_coins())
    {
      Sector::current()->player->kill(true);
      stop();
    }
    if(prev_time != (int) floor(time_left*5))
    {
      Sector::current()->player->add_coins(-1);
    }
  }
}

void
LevelTime::draw(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(Vector(0, 0));

  if ((time_left > TIME_WARNING) || (int(game_time * 2.5) % 2)) {
    std::stringstream ss;
    ss << int(time_left);
    std::string time_text = ss.str();

    if (time_surface)
    {
      float all_width = time_surface->get_width() + Resources::normal_font->get_text_width(time_text);
      context.color().draw_surface(time_surface, Vector((context.get_width() - all_width)/2, BORDER_Y + 1), LAYER_FOREGROUND1);
      context.color().draw_text(Resources::normal_font, time_text,
                        Vector((context.get_width() - all_width)/2 + time_surface->get_width(), BORDER_Y),
                        ALIGN_LEFT, LAYER_FOREGROUND1, LevelTime::text_color);
    }
  }

  context.pop_transform();
}

void
LevelTime::start()
{
  running = true;
}

void
LevelTime::stop()
{
  running = false;
}

float
LevelTime::get_time() const
{
  return time_left;
}

void
LevelTime::set_time(float time_left_)
{
  time_left = std::min(std::max(time_left_, 0.0f), 999.0f);
}

/* EOF */
