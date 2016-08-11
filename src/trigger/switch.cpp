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
#include "util/gettext.hpp"

#include <sstream>

namespace {
const std::string SWITCH_SOUND = "sounds/switch.ogg";
}

Switch::Switch(const ReaderMapping& reader) :
  sprite_name(),
  sprite(),
  script(),
  off_script(),
  state(OFF),
  bistable()
{
  if (!reader.get("x", bbox.p1.x)) throw std::runtime_error("no x position set");
  if (!reader.get("y", bbox.p1.y)) throw std::runtime_error("no y position set");
  if (!reader.get("sprite", sprite_name)) sprite_name = "images/objects/switch/left.sprite";
  sprite = SpriteManager::current()->create(sprite_name);
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());

  reader.get("script", script);
  bistable = reader.get("off-script", off_script);

  SoundManager::current()->preload( SWITCH_SOUND );
}

Switch::~Switch()
{
}

ObjectSettings
Switch::get_settings() {
  ObjectSettings result(_("Switch"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  ObjectOption spr(MN_FILE, _("Sprite"), &sprite_name, "sprite", true, false);
  spr.select.push_back(".sprite");
  result.options.push_back(spr);
  result.options.push_back( ObjectOption(MN_SCRIPT, _("Turn on script"), &script, "script"));
  result.options.push_back( ObjectOption(MN_SCRIPT, _("Turn off script"), &off_script,
                                         "off-script", true, false));
  return result;
}

void
Switch::after_editor_set() {
  sprite = SpriteManager::current()->create(sprite_name);
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
      if(sprite->animation_done() && !bistable) {
        sprite->set_action("turnoff", 1);
        state = TURN_OFF;
      }
      break;
    case TURN_OFF:
      if(sprite->animation_done()) {
        if (bistable) {
          std::istringstream stream(off_script);
          std::ostringstream location;
          location << "switch" << bbox.p1;
          Sector::current()->run_script(stream, location.str());
        }

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
      SoundManager::current()->play( SWITCH_SOUND );
      state = TURN_ON;
      break;
    case TURN_ON:
      break;
    case ON:
      if (bistable) {
        sprite->set_action("turnoff", 1);
        SoundManager::current()->play( SWITCH_SOUND );
        state = TURN_OFF;
      }
      break;
    case TURN_OFF:
      break;
  }

}

/* EOF */
