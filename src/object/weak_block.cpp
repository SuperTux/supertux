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
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

#include <math.h>

WeakBlock::WeakBlock(const Reader& lisp)
  : MovingSprite(lisp, "images/objects/strawbox/strawbox.sprite", LAYER_TILES, COLGROUP_STATIC), state(STATE_NORMAL)
{
  sprite->set_action("normal");
}

HitResponse
WeakBlock::collision(GameObject& other, const CollisionHit& )
{
  switch (state) {

    case STATE_NORMAL:
      if (dynamic_cast<Bullet*>(&other)) {
        startBurning();
        return FORCE_MOVE;
      }
      return FORCE_MOVE;
      break;

    case STATE_BURNING:
      return FORCE_MOVE;
      break;

    case STATE_DISINTEGRATING:
      return FORCE_MOVE;
      break;

  }

  log_debug << "unhandled state" << std::endl;
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

IMPLEMENT_FACTORY(WeakBlock, "weak_block");

/* EOF */
