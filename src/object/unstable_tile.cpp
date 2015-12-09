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
  state(STATE_NORMAL),
  slowfall_timer()
{
  sprite->set_action("normal");
  physic.set_gravity_modifier (.98);
  physic.enable_gravity (false);
}

HitResponse
UnstableTile::collision(GameObject& other, const CollisionHit& )
{
  if(state == STATE_NORMAL) {
    Player* player = dynamic_cast<Player*> (&other);
    if(player != NULL &&
       player->get_bbox().get_bottom() < bbox.get_top() + SHIFT_DELTA) {
      shake ();
    }

    if (dynamic_cast<Explosion*> (&other)) {
      shake ();
    }
  }
  return FORCE_MOVE;
}

void UnstableTile::shake (void)
{
  if (state != STATE_NORMAL)
    return;

  if (sprite->has_action ("shake")) {
    state = STATE_SHAKE;
    this->set_action ("shake", /* loops = */ 1);
  }
  else {
    dissolve ();
  }
}

void UnstableTile::dissolve (void)
{
  if ((state != STATE_NORMAL) && (state != STATE_SHAKE))
    return;

  if (sprite->has_action ("dissolve")) {
    state = STATE_DISSOLVE;
    this->set_action ("dissolve", /* loops = */ 1);
  }
  else {
    slow_fall ();
  }
}

void UnstableTile::slow_fall (void)
{
  /* Only enter slow-fall if neither shake nor dissolve is available. */
  if (state != STATE_NORMAL) {
    this->fall_down ();
    return;
  }

  if (sprite->has_action ("fall-down")) {
    state = STATE_SLOWFALL;
    this->set_action ("fall-down", /* loops = */ 1);
    physic.set_gravity_modifier (.10);
    physic.enable_gravity (true);
    slowfall_timer = 0.5; /* Fall slowly for half a second. */
  }
  else {
    remove_me ();
  }
}

void UnstableTile::fall_down (void)
{
  if ((state != STATE_NORMAL)
      && (state != STATE_SHAKE)
      && (state != STATE_DISSOLVE)
      && (state != STATE_SLOWFALL))
    return;

  if (sprite->has_action ("fall-down")) {
    state = STATE_FALL;
    this->set_action ("fall-down", /* loops = */ 1);
    physic.set_gravity_modifier (.98);
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

    case STATE_SHAKE:
      if (sprite->animation_done())
        dissolve ();
      break;

    case STATE_DISSOLVE:
      if (sprite->animation_done()) {
        /* dissolving is done. Set to non-solid. */
        set_group (COLGROUP_DISABLED);
        fall_down ();
      }
      break;

    case STATE_SLOWFALL:
      if (slowfall_timer >= elapsed_time)
	slowfall_timer -= elapsed_time;
      else /* Switch to normal falling procedure */
	fall_down ();
      movement = physic.get_movement (elapsed_time);
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
