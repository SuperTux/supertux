//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#include <config.h>

#include <math.h>
#include "growup.h"
#include "resources.h"
#include "camera.h"
#include "sector.h"
#include "player.h"
#include "audio/sound_manager.h"
#include "sprite/sprite_manager.h"

GrowUp::GrowUp()
{
  bbox.set_size(32, 32);
  
  sprite = sprite_manager->create("egg");
  physic.enable_gravity(true);
  physic.set_velocity_x(100);
}

GrowUp::~GrowUp()
{
  delete sprite;
}

void
GrowUp::update(float elapsed_time)
{
  movement = physic.get_movement(elapsed_time);
}

HitResponse
GrowUp::collision(GameObject& other, const CollisionHit& hit)
{
  if(other.get_flags() & FLAG_SOLID) {
    if(fabsf(hit.normal.y) > .5) { // roof or ground
      physic.set_velocity_y(0);
    } else { // bumped left or right
      physic.set_velocity_x(-physic.get_velocity_x());
    }

    return CONTINUE;
  }
  
  Player* player = dynamic_cast<Player*>(&other);
  if(player != 0) {
    player->set_bonus(GROWUP_BONUS, true);
    sound_manager->play("sounds/grow.wav");
    remove_me();
    
    return ABORT_MOVE;
  }

  return FORCE_MOVE;
}

void
GrowUp::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

