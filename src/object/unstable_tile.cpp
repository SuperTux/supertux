//  SuperTux - Unstable Tile
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

#include <config.h>

#include "unstable_tile.hpp"
#include "lisp/lisp.hpp"
#include "object_factory.hpp"
#include "player.hpp"
#include "sector.hpp"
#include "resources.hpp"
#include "sprite/sprite.hpp"
#include "random_generator.hpp"
#include "object/bullet.hpp"
#include "constants.hpp"

UnstableTile::UnstableTile(const lisp::Lisp& lisp)
  : MovingSprite(lisp, LAYER_TILES, COLGROUP_STATIC), state(STATE_NORMAL)
{
  sprite->set_action("normal");
}

HitResponse
UnstableTile::collision(GameObject& other, const CollisionHit& )
{
  if(state == STATE_NORMAL) {
    Player* player = dynamic_cast<Player*> (&other);
    if(player != NULL &&
        player->get_bbox().get_bottom() < get_bbox().get_top() + SHIFT_DELTA) {
      state = STATE_CRUMBLING;
      sprite->set_action("crumbling", 1);
    }
  }
  return SOLID;
}

void
UnstableTile::update(float elapsed_time)
{
  switch (state) {

    case STATE_NORMAL:
      break;

    case STATE_CRUMBLING:
      if (sprite->animation_done()) {
        state = STATE_DISINTEGRATING;
        sprite->set_action("disintegrating", 1);
        set_group(COLGROUP_DISABLED);
        physic.enable_gravity(true);
      }
      break;

    case STATE_DISINTEGRATING:
      movement = physic.get_movement(elapsed_time);
      if (sprite->animation_done()) {
        remove_me();
        return;
      }
      break;
  }
}

IMPLEMENT_FACTORY(UnstableTile, "unstable_tile");
