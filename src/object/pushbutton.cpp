//  SuperTux - PushButton running a script
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

#include "audio/sound_manager.hpp"
#include "object/player.hpp"
#include "object/pushbutton.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"
#include "util/reader.hpp"

#include <sstream>
#include <stdexcept>

namespace {
const std::string BUTTON_SOUND = "sounds/switch.ogg";
//14 -> 8
}

PushButton::PushButton(const Reader& lisp) :
  MovingSprite(lisp, "images/objects/pushbutton/pushbutton.sprite", LAYER_BACKGROUNDTILES+1, COLGROUP_MOVING),
  script(),
  state(OFF)
{
  SoundManager::current()->preload(BUTTON_SOUND);
  set_action("off", -1);
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());

  if (!lisp.get("script", script)) throw std::runtime_error("no script set");
}

void
PushButton::save(lisp::Writer& writer) {
  MovingSprite::save(writer);
  writer.write("script", script, false);
}

ObjectSettings
PushButton::get_settings() {
  ObjectSettings result(_("Button"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Script"), &script));

  return result;
}

void
PushButton::update(float /*elapsed_time*/)
{
}

HitResponse
PushButton::collision(GameObject& other, const CollisionHit& hit)
{
  Player* player = dynamic_cast<Player*>(&other);
  if (!player) return FORCE_MOVE;
  float vy = player->get_physic().get_velocity_y();

  //player->add_velocity(Vector(0, -150));
  player->get_physic().set_velocity_y(-150);

  if (state != OFF) return FORCE_MOVE;
  if (!hit.top) return FORCE_MOVE;
  if (vy <= 0) return FORCE_MOVE;

  // change appearance
  state = ON;
  float old_bbox_height = bbox.get_height();
  set_action("on", -1);
  float new_bbox_height = bbox.get_height();
  set_pos(get_pos() + Vector(0, old_bbox_height - new_bbox_height));

  // play sound
  SoundManager::current()->play(BUTTON_SOUND);

  // run script
  std::istringstream stream(script);
  Sector::current()->run_script(stream, "PushButton");

  return FORCE_MOVE;
}

/* EOF */
