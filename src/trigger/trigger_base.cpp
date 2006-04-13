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

#include "trigger_base.hpp"
#include "video/drawing_context.hpp"
#include "object/player.hpp"

TriggerBase::TriggerBase()
  : sprite(0)
{
  set_group(COLGROUP_TOUCHABLE);
}

TriggerBase::~TriggerBase()
{
}

void
TriggerBase::update(float )
{
  lasthit = hit;
  hit = false;
}

void
TriggerBase::draw(DrawingContext& context)
{
  if(!sprite)
    return;

  sprite->draw(context, get_pos(), LAYER_TILES+1);
}

HitResponse
TriggerBase::collision(GameObject& other, const CollisionHit& )
{
  Player* player = dynamic_cast<Player*> (&other);
  if(player) {
    hit = true;
    if(!lasthit)
      event(*player, EVENT_TOUCH);
  }

  return ABORT_MOVE;
}

