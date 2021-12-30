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

#include "trigger/switch.hpp"

#include <sstream>

#include "audio/sound_manager.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"

namespace {
const std::string SWITCH_SOUND = "sounds/switch.ogg";
}

Switch::Switch(const ReaderMapping& reader) :
  sprite_name(),
  sprite(),
  script(),
  off_script(),
  state(OFF),
  bistable(),
  m_flip(NO_FLIP)
{
  if (!reader.get("x", m_col.m_bbox.get_left())) throw std::runtime_error("no x position set");
  if (!reader.get("y", m_col.m_bbox.get_top())) throw std::runtime_error("no y position set");
  if (!reader.get("sprite", sprite_name)) sprite_name = "images/objects/switch/left.sprite";
  sprite = SpriteManager::current()->create(sprite_name);
  m_col.m_bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());

  reader.get("script", script);
  bistable = reader.get("off-script", off_script);

  SoundManager::current()->preload( SWITCH_SOUND );
}

Switch::~Switch()
{
}

ObjectSettings
Switch::get_settings()
{
  ObjectSettings result = TriggerBase::get_settings();

  result.add_sprite(_("Sprite"), &sprite_name, "sprite", std::string("images/objects/switch/left.sprite"));
  result.add_script(_("Turn on script"), &script, "script");
  result.add_script(_("Turn off script"), &off_script, "off-script");

  result.reorder({"script", "off-script", "sprite", "x", "y"});

  return result;
}

void
Switch::after_editor_set() {
  sprite = SpriteManager::current()->create(sprite_name);
  m_col.m_bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
}

void
Switch::update(float )
{
  switch (state) {
    case OFF:
      break;
    case TURN_ON:
      if (sprite->animation_done()) {
        std::ostringstream location;
        location << "switch" << m_col.m_bbox.p1();
        Sector::get().run_script(script, location.str());

        sprite->set_action("on", 1);
        state = ON;
      }
      break;
    case ON:
      if (sprite->animation_done() && !bistable) {
        sprite->set_action("turnoff", 1);
        state = TURN_OFF;
      }
      break;
    case TURN_OFF:
      if (sprite->animation_done()) {
        if (bistable) {
          std::ostringstream location;
          location << "switch" << m_col.m_bbox.p1();
          Sector::get().run_script(off_script, location.str());
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
  sprite->draw(context.color(), m_col.m_bbox.p1(), LAYER_TILES, m_flip);
}

void
Switch::event(Player& , EventType type)
{
  if (type != EVENT_ACTIVATE) return;

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

void
Switch::on_flip(float height)
{
  TriggerBase::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
}

/* EOF */
