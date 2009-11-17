//  SuperTux - Switch Trigger
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

#include <config.h>
#include <stdexcept>

#include "audio/sound_manager.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "trigger/switch.hpp"

namespace {
const std::string SWITCH_SOUND = "sounds/switch.ogg";
}

Switch::Switch(const Reader& reader) :
  sprite_name(),
  sprite(),
  script(),
  state(OFF)
{
  if (!reader.get("x", bbox.p1.x)) throw std::runtime_error("no x position set");
  if (!reader.get("y", bbox.p1.y)) throw std::runtime_error("no y position set");
  if (!reader.get("sprite", sprite_name)) throw std::runtime_error("no sprite name set");
  sprite = sprite_manager->create(sprite_name);
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());

  if (!reader.get("script", script)) throw std::runtime_error("no script set");
  sound_manager->preload( SWITCH_SOUND );
}

Switch::~Switch()
{
}

void
Switch::update(float )
{
  switch (state) {
    case OFF:
      break;
    case TURN_ON:
      if(sprite->animation_done()) {
        std::istringstream stream(script);
        std::ostringstream location;
        location << "switch" << bbox.p1;
        Sector::current()->run_script(stream, location.str());

        sprite->set_action("on", 1);
        state = ON;
      }
      break;
    case ON:
      if(sprite->animation_done()) {
        sprite->set_action("turnoff", 1);
        state = TURN_OFF;
      }
      break;
    case TURN_OFF:
      if(sprite->animation_done()) {
        sprite->set_action("off");
        state = OFF;
      }
      break;
  }
}

void
Switch::draw(DrawingContext& context)
{
  sprite->draw(context, bbox.p1, LAYER_TILES);
}

void
Switch::event(Player& , EventType type)
{
  if(type != EVENT_ACTIVATE) return;

  switch (state) {
    case OFF:
      sprite->set_action("turnon", 1);
      sound_manager->play( SWITCH_SOUND );
      state = TURN_ON;
      break;
    case TURN_ON:
      break;
    case ON:
      break;
    case TURN_OFF:
      break;
  }

}

IMPLEMENT_FACTORY(Switch, "switch");

/* EOF */
