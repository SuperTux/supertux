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
#include "object/electrifier.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/globals.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader.hpp"

namespace {
const float LIGHTNING_DELAY = 2.0f;
const float FLASH_DISPLAY_TIME = 0.1f;
const float ELECTRIFY_TIME = 0.5f;
}

Thunderstorm::Thunderstorm(const Reader& reader) :
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
  reader.get("layer", layer);

  sound_manager->preload("sounds/thunder.wav");
  sound_manager->preload("sounds/lightning.wav");

  if (running) {
    running = false; // else start() is ignored
    start();
  }
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
}

void
Thunderstorm::draw(DrawingContext& context)
{
  if (!flash_display_timer.started()) return;

  float alpha = 0.33f;
  context.push_transform();
  context.set_translation(Vector(0, 0));
  context.draw_filled_rect(Vector(0, 0), Vector(SCREEN_WIDTH, SCREEN_HEIGHT), Color(1, 1, 1, alpha), layer);
  context.pop_transform();

}

void
Thunderstorm::expose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name == "") return;
  scripting::Thunderstorm* _this = new scripting::Thunderstorm(this);
  expose_object(vm, table_idx, _this, name, true);
}

void
Thunderstorm::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name == "") return;
  scripting::unexpose_object(vm, table_idx, name);
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
  sound_manager->play("sounds/thunder.wav");
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
  sound_manager->play("sounds/lightning.wav");
  flash_display_timer.start(FLASH_DISPLAY_TIME);
}

void
Thunderstorm::electrify()
{
  Sector::current()->add_object(new Electrifier(200, 1421, ELECTRIFY_TIME));
  Sector::current()->add_object(new Electrifier(201, 1422, ELECTRIFY_TIME));
}

/* EOF */
