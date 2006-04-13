//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "star.hpp"
#include "resources.hpp"
#include "player.hpp"
#include "player_status.hpp"
#include "sprite/sprite_manager.hpp"
#include "video/drawing_context.hpp"

static const float INITIALJUMP = 400;
static const float SPEED = 150;
static const float JUMPSPEED = 300;

Star::Star(const Vector& pos)
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);
  sprite = sprite_manager->create("images/powerups/star/star.sprite");
  physic.set_velocity(SPEED, INITIALJUMP);

  set_group(COLGROUP_MOVING);
}

Star::~Star()
{
  delete sprite;
}

void
Star::update(float elapsed_time)
{
  movement = physic.get_movement(elapsed_time);
}

void
Star::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

HitResponse
Star::collision(GameObject& other, const CollisionHit& hit)
{
  if(other.get_flags() & FLAG_SOLID) {
    if(hit.normal.y < -.5) { // ground
      physic.set_velocity_y(JUMPSPEED);
    } else if(hit.normal.y > .5) { // roof
      physic.set_velocity_y(0);
    } else { // bumped left or right
      physic.set_velocity_x(-physic.get_velocity_x());
    }

    return CONTINUE;
  }
  
  Player* player = dynamic_cast<Player*> (&other);
  if(player) {
    player->make_invincible();
    remove_me();
    return ABORT_MOVE;
  }

  return FORCE_MOVE;
}

