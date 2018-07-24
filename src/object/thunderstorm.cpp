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
#include "object/electrifier.hpp"
#include "supertux/sector.hpp"
#include "util/reader.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

namespace {
const float LIGHTNING_DELAY = 2.0f;
const float FLASH_DISPLAY_TIME = 0.1f;
const float ELECTRIFY_TIME = 0.5f;
}

Thunderstorm::Thunderstorm(const ReaderMapping& reader) :
  GameObject(reader),
  ExposedObject<Thunderstorm, scripting::Thunderstorm>(this),
  running(true),
  interval(10.0f),
  layer(LAYER_BACKGROUNDTILES-1),
  time_to_thunder(),
  time_to_lightning(),
  flash_display_timer()
{
  reader.get("name", name);
  reader.get("running", running);
  reader.get("interval", interval);
  if(interval <= 0) {
    log_warning << "Running a thunderstorm with non-positive time interval is a bad idea" << std::endl;
  }
  layer = reader_get_layer (reader, /* default = */ LAYER_BACKGROUNDTILES-1);

  SoundManager::current()->preload("sounds/thunder.wav");
  SoundManager::current()->preload("sounds/lightning.wav");

  if (running) {
    running = false; // else start() is ignored
    start();
  }
}

ObjectSettings
Thunderstorm::get_settings() {
  ObjectSettings result = GameObject::get_settings();
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Running"), &running,
                                         "running"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Interval"), &interval,
                                         "interval"));

  result.options.push_back( ObjectOption(MN_REMOVE, "", NULL));
  return result;
}

void
Thunderstorm::update(float )
{
  if (Editor::is_active()) return;
  if (!running) return;

  if (time_to_thunder.check()) {
    thunder();
    time_to_lightning.start(LIGHTNING_DELAY);
  }
  if (time_to_lightning.check()) {
    lightning();
    time_to_thunder.start(interval);
  }
}

void
Thunderstorm::draw(DrawingContext& context)
{
  if (!flash_display_timer.started()) return;

  float alpha = 0.33f;
  context.push_transform();
  context.set_translation(Vector(0, 0));
  context.color().draw_filled_rect(Vector(0, 0), Vector(context.get_width(), context.get_height()), Color(1, 1, 1, alpha), layer);
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
}

void
Thunderstorm::lightning()
{
  flash();
  electrify();
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
  auto changing_tiles = TileChangeMap({
    {200, 1421}, {201, 1422},
    {3419, 3523}, {3420, 3524},
    {3421, 3525}, {3422, 3526},
    {3423, 3527}, {3424, 3528},
    {3425, 3529}, {3426, 3530},
    {3427, 3523}, {3428, 3524},
    {3429, 3525}, {3430, 3526},
    {3431, 3527}, {3432, 3528},
    {3433, 3529}, {3434, 3530}
  });
  Sector::current()->add_object(std::make_shared<Electrifier>(changing_tiles, ELECTRIFY_TIME));
}

/* EOF */
