//  SuperTux - Weak Block
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "object/weak_block.hpp"

#include "object/bullet.hpp"
#include "object/explosion.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "util/reader.hpp"

#include <math.h>

WeakBlock::WeakBlock(const Reader& lisp)
: MovingSprite(lisp, "images/objects/weak_block/strawbox.sprite", LAYER_TILES, COLGROUP_STATIC), state(STATE_NORMAL),
linked(true)
{
  sprite->set_action("normal");
  //Check if this weakblock destroys adjacent weakblocks
  if(lisp.get("linked", linked)){
    if(! linked){
      sprite_name = "images/objects/weak_block/meltbox.sprite";
      sprite = sprite_manager->create(sprite_name);
      sprite->set_action("normal");
    }
  }
}

HitResponse
WeakBlock::collision_bullet(Bullet& bullet, const CollisionHit& hit)
{
  switch (state) {
			
    case STATE_NORMAL:
      //Ensure only fire destroys weakblock
      if(bullet.get_type() == FIRE_BONUS) {
        startBurning();
        bullet.remove_me();
      }
      //Other bullets ricochet
      else {
        bullet.ricochet(*this, hit);
      }
    break;
			
    case STATE_BURNING:
    case STATE_DISINTEGRATING:
      break;
			
    default:
      log_debug << "unhandled state" << std::endl;
      break;
	}
	
	return FORCE_MOVE;
}

HitResponse
WeakBlock::collision(GameObject& other, const CollisionHit& hit)
{
  switch (state) {
				
      case STATE_NORMAL:
        if (Bullet* bullet = dynamic_cast<Bullet*> (&other)) {
          return collision_bullet(*bullet, hit);
        }
        //Explosions destroy weakblocks as well
        if (dynamic_cast<Explosion*> (&other)) {
          startBurning();
        }
        break;
				
      case STATE_BURNING:
      case STATE_DISINTEGRATING:
        break;
				
      default:
        log_debug << "unhandled state" << std::endl;
        break;
  }
	
  return FORCE_MOVE;
}

void
WeakBlock::update(float )
{
  switch (state) {
				
      case STATE_NORMAL:
        break;
				
      case STATE_BURNING:
        if (sprite->animation_done()) {
          state = STATE_DISINTEGRATING;
          sprite->set_action("disintegrating", 1);
          spreadHit();
          set_group(COLGROUP_DISABLED);
        }
        break;
				
      case STATE_DISINTEGRATING:
        if (sprite->animation_done()) {
          remove_me();
          return;
        }
        break;
				
  }
}

void
WeakBlock::startBurning()
{
  if (state != STATE_NORMAL) return;
  state = STATE_BURNING;
  sprite->set_action("burning", 1);
}

void
WeakBlock::spreadHit()
{
  //Destroy adjacent weakblocks if applicable
  if(linked) {
    Sector* sector = Sector::current();
    if (!sector) {
      log_debug << "no current sector" << std::endl;
      return;
    }
    for(Sector::GameObjects::iterator i = sector->gameobjects.begin(); i != sector->gameobjects.end(); ++i) {
      WeakBlock* wb = dynamic_cast<WeakBlock*>(*i);
      if (!wb) continue;
      if (wb == this) continue;
      if (wb->state != STATE_NORMAL) continue;
      float dx = fabsf(wb->get_pos().x - this->get_pos().x);
      float dy = fabsf(wb->get_pos().y - this->get_pos().y);
      if ((dx <= 32.5) && (dy <= 32.5)) wb->startBurning();
    }
  }
}


/* EOF */
