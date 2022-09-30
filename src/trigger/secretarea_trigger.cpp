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

#include "audio/sound_manager.hpp"
#include "editor/editor.hpp"
#include "object/tilemap.hpp"
#include "supertux/debug.hpp"
#include "supertux/level.hpp"
#include "supertux/resources.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

static const float MESSAGE_TIME=3.5;

SecretAreaTrigger::SecretAreaTrigger(const ReaderMapping& reader) :
  TriggerBase(reader),
  message_timer(),
  message_displayed(false),
  message(),
  fade_tilemap(),
  script(),
  new_size(0.0f, 0.0f)
{
  reader.get("x", m_col.m_bbox.get_left());
  reader.get("y", m_col.m_bbox.get_top());
  float w,h;
  reader.get("width", w, 32.0f);
  reader.get("height", h, 32.0f);
  m_col.m_bbox.set_size(w, h);
  new_size.x = w;
  new_size.y = h;
  reader.get("fade-tilemap", fade_tilemap);
  reader.get("message", message);
  if (message.empty() && !Editor::is_active()) {
    message = _("You found a secret area!");
  }
  reader.get("script", script);
}

SecretAreaTrigger::SecretAreaTrigger(const Rectf& area, const std::string& fade_tilemap_) :
  message_timer(),
  message_displayed(false),
  message(_("You found a secret area!")),
  fade_tilemap(fade_tilemap_),
  script(),
  new_size(0.0f, 0.0f)
{
  m_col.m_bbox = area;
}

ObjectSettings
SecretAreaTrigger::get_settings()
{
  new_size.x = m_col.m_bbox.get_width();
  new_size.y = m_col.m_bbox.get_height();

  ObjectSettings result = TriggerBase::get_settings();

  result.add_text(_("Name"), &m_name);
  result.add_text(_("Fade tilemap"), &fade_tilemap, "fade-tilemap");
  result.add_translatable_text(_("Message"), &message, "message");
  result.add_script(_("Script"), &script, "script");

  result.reorder({"fade-tilemap", "script", "sprite", "message", "region", "name", "x", "y"});

  return result;
}

void
SecretAreaTrigger::after_editor_set()
{
  m_col.m_bbox.set_size(new_size.x, new_size.y);
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
    context.transform().scale = 1.f;
    Vector pos = Vector(static_cast<float>(context.get_width()) / 2.0f, static_cast<float>(context.get_height()) / 2.0f - Resources::normal_font->get_height() / 2.0f);
    context.color().draw_text(Resources::normal_font, message, pos, FontAlignment::ALIGN_CENTER, LAYER_HUD, SecretAreaTrigger::text_color);
    context.pop_transform();
  }
  if (Editor::is_active() || g_debug.show_collision_rects) {
    context.color().draw_filled_rect(m_col.m_bbox, Color(0.0f, 1.0f, 0.0f, 0.6f),
                             0.0f, LAYER_OBJECTS);
  } else if (message_timer.check()) {
    remove_me();
  }
}

void
SecretAreaTrigger::event(Player& , EventType type)
{
  if (type == EVENT_TOUCH) {
    if (!message_displayed) {
      message_timer.start(MESSAGE_TIME);
      message_displayed = true;
      Sector::get().get_level().m_stats.increment_secrets();
      SoundManager::current()->play("sounds/welldone.ogg");

      if (!fade_tilemap.empty()) {
        // fade away tilemaps
        for (auto& tm : Sector::get().get_objects_by_type<TileMap>()) {
          if (tm.get_name() == fade_tilemap) {
            tm.fade(0.0, 1.0);
          }
        }
      }

      if (!script.empty()) {
        Sector::get().run_script(script, "SecretAreaScript");
      }
    }
  }
}

/* EOF */
