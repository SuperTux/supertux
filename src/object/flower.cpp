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
#include "flower.h"
#include "resources.h"
#include "camera.h"
#include "sector.h"
#include "player.h"
#include "sprite/sprite_manager.h"

Flower::Flower(const Vector& pos, Type _type)
  : type(_type)
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);

  if(_type == FIREFLOWER)
    sprite = sprite_manager->create("fireflower");
  else
    sprite = sprite_manager->create("iceflower"); 
}

Flower::~Flower()
{
  delete sprite;
}

void
Flower::action(float )
{
}

void
Flower::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

HitResponse
Flower::collision(GameObject& other, const CollisionHit& )
{
  Player* player = dynamic_cast<Player*>(&other);
  if(!player)
    return ABORT_MOVE;

  if(type == FIREFLOWER)
    player->set_bonus(FIRE_BONUS, true);
  else
    player->set_bonus(ICE_BONUS, true);
  
  sound_manager->play_sound("fire-flower");
  remove_me();
  return ABORT_MOVE;
}

