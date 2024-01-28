//  SuperTux - Thunderstorm Game Object
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "object/thunderstorm.hpp"

#include "audio/sound_manager.hpp"
#include "editor/editor.hpp"
#include "object/ambient_light.hpp"
#include "object/background.hpp"
#include "object/electrifier.hpp"
#include "supertux/level.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile_manager.hpp"
#include "supertux/tile_set.hpp"
#include "util/reader.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

namespace {

const float LIGHTNING_DELAY = 2.0f;
const float FLASH_DISPLAY_TIME = 2.0f;
const float ELECTRIFY_TIME = 0.5f;
} // namespace

Thunderstorm::Thunderstorm(const ReaderMapping& reader) :
  GameObject(reader),
  ExposedObject<Thunderstorm, scripting::Thunderstorm>(this),
  running(true),
  interval(10.0f),
  layer(LAYER_BACKGROUNDTILES-1),
  m_strike_script(),
  time_to_thunder(),
  time_to_lightning(),
  flash_display_timer(),
  changing_tiles(TileManager::current()->get_tileset(Level::current()->get_tileset())->m_thunderstorm_tiles),
  last_ambient_color(),
  last_layer(),
  flash_layer(),
  flash_color()
{
  reader.get("running", running);
  reader.get("interval", interval);
  reader.get("strike-script", m_strike_script, "");
  if (interval <= 0) {
    log_warning << "Running a thunderstorm with non-positive time interval is a bad idea" << std::endl;
  }
  layer = reader_get_layer (reader, LAYER_BACKGROUNDTILES - 1);
  last_layer = layer;

  SoundManager::current()->preload("sounds/thunder.wav");
  SoundManager::current()->preload("sounds/lightning.wav");

  if (running) {
    running = false; // else start() is ignored
    start();
  }
}

ObjectSettings
Thunderstorm::get_settings()
{
  ObjectSettings result = GameObject::get_settings();

  result.add_int(_("Z-pos"), &layer, "z-pos", LAYER_BACKGROUNDTILES - 1);
  result.add_bool(_("Running"), &running, "running", true);
  result.add_float(_("Interval"), &interval, "interval", 10.0f);
  result.add_text(_("Strike Script"), &m_strike_script, "strike-script");

  result.reorder({"interval", "name", "z-pos" "strike-script"});

  result.add_remove();

  return result;
}

void
Thunderstorm::update(float )
{
  if (!running) return;

  if (time_to_thunder.check()) {
    thunder();
    time_to_lightning.start(LIGHTNING_DELAY);
  }
  if (time_to_lightning.check()) {
    lightning();
    time_to_thunder.start(interval);
  }

  if(flash_display_timer.started()) {
    float alpha = 1.0f;
    if(flash_display_timer.get_timegone() > 0.4f) {
      auto progress = flash_display_timer.get_timegone() / flash_display_timer.get_timeleft() - 0.4f;
      alpha = 1.0f - progress;
    }
    
    if(alpha < 0.0f) {
      flash_display_timer.stop();
      Sector::current()->get_singleton_by_type<AmbientLight>()
        .set_ambient_light(last_ambient_color);
      return;
    }

    flash_color = Color(1, 1, 1, alpha);
    flash_layer = Sector::current()->get_foremost_layer() + 1;
  }
}

void
Thunderstorm::draw(DrawingContext& context)
{
  if (!flash_display_timer.started()) return;

  context.push_transform();
  context.set_translation(Vector(0, 0));
  context.color().draw_filled_rect(context.get_rect(), flash_color, flash_layer, Blend::BLEND);
  context.pop_transform();
}

void
Thunderstorm::start()
{
  if (running) return;
  running = true;
  time_to_thunder.start(interval);
  time_to_lightning.stop();
}

void
Thunderstorm::stop()
{
  if (!running) return;
  running = false;
  time_to_thunder.stop();
  time_to_lightning.stop();
}

void
Thunderstorm::thunder()
{
  SoundManager::current()->play("sounds/thunder.wav");
  change_background_colors(false);
  last_ambient_color = Sector::current()->get_singleton_by_type<AmbientLight>().get_ambient_light();
}

void
Thunderstorm::lightning()
{
  flash();
  electrify();
  if (!m_strike_script.empty()) {
	  Sector::get().run_script(m_strike_script, "strike-script");
  }

  change_background_colors(true);
  Sector::current()->get_singleton_by_type<AmbientLight>().set_ambient_light(Color::WHITE);
}

void
Thunderstorm::flash()
{
  SoundManager::current()->play("sounds/lightning.wav");
  flash_display_timer.start(FLASH_DISPLAY_TIME);
}

void
Thunderstorm::electrify()
{
  Sector::get().add<Electrifier>(changing_tiles, ELECTRIFY_TIME);
}

void
Thunderstorm::change_background_colors(bool is_lightning)
{
  auto factor = is_lightning ? (1.0f / 0.7f) : 0.7f;
  auto backgrounds = Sector::current()->get_objects_by_type<Background>();
  for(auto& background : backgrounds)
  {
    auto new_color = background.get_color() * factor;
    background.fade_color(new_color.validate(), 0.1f);
  }
}

/* EOF */
