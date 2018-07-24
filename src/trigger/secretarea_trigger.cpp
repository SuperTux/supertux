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

#include "trigger/secretarea_trigger.hpp"

#include "editor/editor.hpp"
#include "object/tilemap.hpp"
#include "supertux/level.hpp"
#include "supertux/resources.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

static const float MESSAGE_TIME=3.5;

SecretAreaTrigger::SecretAreaTrigger(const ReaderMapping& reader) :
  message_timer(),
  message_displayed(false),
  message(),
  fade_tilemap(),
  script(),
  new_size()
{
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  float w,h;
  reader.get("width", w, 32.0f);
  reader.get("height", h, 32.0f);
  bbox.set_size(w, h);
  reader.get("fade-tilemap", fade_tilemap);
  reader.get("message", message);
  if(message.empty() && !Editor::is_active()) {
    message = _("You found a secret area!");
  }
  reader.get("script", script);
}

SecretAreaTrigger::SecretAreaTrigger(const Rectf& area, std::string fade_tilemap_) :
  message_timer(),
  message_displayed(false),
  message(_("You found a secret area!")),
  fade_tilemap(fade_tilemap_),
  script(),
  new_size()
{
  bbox = area;
}

ObjectSettings
SecretAreaTrigger::get_settings() {
  new_size.x = bbox.get_width();
  new_size.y = bbox.get_height();
  ObjectSettings result(_("Secret area"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Width"), &new_size.x, "width"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Height"), &new_size.y, "height"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Fade-tilemap"), &fade_tilemap,
                                         "fade-tilemap", (OPTION_VISIBLE)));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Message"), &message, "message"));
  result.options.push_back( ObjectOption(MN_SCRIPT, _("Script"), &script,
                                         "script", (OPTION_VISIBLE)));
  return result;
}

void
SecretAreaTrigger::after_editor_set() {
  bbox.set_size(new_size.x, new_size.y);
}

std::string
SecretAreaTrigger::get_fade_tilemap_name() const
{
  return fade_tilemap;
}

void
SecretAreaTrigger::draw(DrawingContext& context)
{
  if (message_timer.started()) {
    context.push_transform();
    context.set_translation(Vector(0, 0));
    Vector pos = Vector(0, SCREEN_HEIGHT/2 - Resources::normal_font->get_height()/2);
    context.color().draw_center_text(Resources::normal_font, message, pos, LAYER_HUD, SecretAreaTrigger::text_color);
    context.pop_transform();
  }
  if (Editor::is_active()) {
    context.color().draw_filled_rect(bbox, Color(0.0f, 1.0f, 0.0f, 0.6f),
                             0.0f, LAYER_OBJECTS);
  } else if (message_timer.check()) {
    remove_me();
  }
}

void
SecretAreaTrigger::event(Player& , EventType type)
{
  if(type == EVENT_TOUCH) {
    if (!message_displayed) {
      message_timer.start(MESSAGE_TIME);
      message_displayed = true;
      Sector::current()->get_level()->stats.secrets++;

      if (!fade_tilemap.empty()) {
        // fade away tilemaps
        auto& sector = *Sector::current();
        for(const auto& i : sector.gameobjects) {
          auto tm = dynamic_cast<TileMap*>(i.get());
          if (!tm) continue;
          if (tm->get_name() != fade_tilemap) continue;
          tm->fade(0.0, 1.0);
        }
      }

      if(!script.empty()) {
        Sector::current()->run_script(script, "SecretAreaScript");
      }
    }
  }
}

/* EOF */
