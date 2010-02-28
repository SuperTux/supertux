//  SuperTux - Unstable Tile
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//  Copyright (C) 2010 Florian Forster <supertux at octo.it>
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

#include "object/unstable_tile.hpp"

#include "object/explosion.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/constants.hpp"
#include "supertux/object_factory.hpp"

UnstableTile::UnstableTile(const Reader& lisp) :
  MovingSprite(lisp, LAYER_TILES, COLGROUP_STATIC), 
  physic(),
  state(STATE_NORMAL)
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
      dissolve ();
    }

    if (dynamic_cast<Explosion*> (&other)) {
      dissolve ();
    }
  }
  return FORCE_MOVE;
}

void UnstableTile::dissolve (void)
{
  if (state != STATE_NORMAL)
    return;

  if (sprite->has_action ("dissolve")) {
    state = STATE_DISSOLVE;
    this->set_action ("dissolve", /* loops = */ 1);
  }
  else {
    fall_down ();
  }
}

void UnstableTile::fall_down (void)
{
  if ((state != STATE_NORMAL) && (state != STATE_DISSOLVE))
    return;

  if (sprite->has_action ("fall-down")) {
    state = STATE_FALL;
    this->set_action ("fall-down", /* loops = */ 1);
    set_group (COLGROUP_DISABLED);
    physic.enable_gravity (true);
  }
  else {
    remove_me ();
  }
}

void
UnstableTile::update(float elapsed_time)
{
  switch (state)
  {
    case STATE_NORMAL:
      break;

    case STATE_DISSOLVE:
      if (sprite->animation_done())
        fall_down ();
      break;

    case STATE_FALL:
      if (sprite->animation_done())
        remove_me ();
      else
        movement = physic.get_movement (elapsed_time);
      break;
  }
}

/* EOF */
