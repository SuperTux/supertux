//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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
#include "object/water_drop.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"

WaterDrop::WaterDrop(const Vector& pos) :
  MovingSprite(pos, "images/objects/particles/water_drop.sprite", LAYER_OBJECTS, COLGROUP_MOVING_ONLY_STATIC),
  physic(),
  wd_state(WDS_FALLING)
{
  physic.enable_gravity(true);
}

void
WaterDrop::update(float elapsed_time)
{
  movement = physic.get_movement(elapsed_time);

  if ( sprite->animation_done() ) {
    remove_me();
  }
}

void
WaterDrop::collision_solid(const CollisionHit& hit)
{
  if(hit.bottom && wd_state == WDS_FALLING) {
    wd_state = WDS_SPLASH;
    physic.enable_gravity(false);
    SoundManager::current()->play("sounds/splash.ogg", get_pos());
    sprite->set_action("splash", 1);
  }
}

HitResponse
WaterDrop::collision(GameObject& other, const CollisionHit& )
{
  return FORCE_MOVE;
}

/* EOF */
