//  $Id: weak_block.cpp 3435 2006-04-26 02:13:42Z sik0fewl $
//
//  SuperTux - Weak Block
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "weak_block.hpp"
#include "lisp/lisp.hpp"
#include "object_factory.hpp"
#include "player.hpp"
#include "sector.hpp"
#include "resources.hpp"
#include "sprite/sprite.hpp"
#include "random_generator.hpp"
#include "object/bullet.hpp"

WeakBlock::WeakBlock(const lisp::Lisp& lisp)
  : MovingSprite(lisp, "images/objects/strawbox/strawbox.sprite", LAYER_TILES, COLGROUP_STATIC), state(STATE_NORMAL)
{
  sprite->set_action("normal");
  flags |= FLAG_SOLID;
}

HitResponse
WeakBlock::collision(GameObject& other, const CollisionHit& )
{
  switch (state) {

    case STATE_NORMAL:
      if (dynamic_cast<Bullet*>(&other)) {
	state = STATE_BURNING;
	sprite->set_action("burning", 1);
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
	flags &= ~FLAG_SOLID;
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

IMPLEMENT_FACTORY(WeakBlock, "weak_block");
