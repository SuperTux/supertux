//  $Id$
//
//  SuperTux - PushButton running a script
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <config.h>
#include <stdexcept>

#include "pushbutton.hpp"
#include "object_factory.hpp"
#include "player.hpp"
#include "audio/sound_manager.hpp"
#include "sprite/sprite_manager.hpp"
#include "sector.hpp"
#include "log.hpp"
#include "sprite/sprite.hpp"

namespace {
  const std::string BUTTON_SOUND = "sounds/switch.ogg";
 //14 -> 8
}

PushButton::PushButton(const lisp::Lisp& lisp)
        : MovingSprite(lisp, "images/objects/pushbutton/pushbutton.sprite", LAYER_BACKGROUNDTILES+1, COLGROUP_MOVING), state(OFF)
{
  sound_manager->preload(BUTTON_SOUND);
  set_action("off", -1);
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());

  if (!lisp.get("script", script)) throw std::runtime_error("no script set");
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
  float vy = player->physic.get_velocity_y();

  //player->add_velocity(Vector(0, -150));
  player->physic.set_velocity_y(-150);

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
  sound_manager->play(BUTTON_SOUND);

  // run script
  std::istringstream stream(script);
  Sector::current()->run_script(stream, "PushButton");

  return FORCE_MOVE;
}

IMPLEMENT_FACTORY(PushButton, "pushbutton");
