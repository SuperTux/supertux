//  $Id$
//
//  SuperTux
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

#include "candle.hpp"
#include "object_factory.hpp"

Candle::Candle(const lisp::Lisp& lisp)
	: MovingSprite(lisp, "images/objects/candle/candle.sprite", LAYER_BACKGROUNDTILES+1, COLGROUP_DISABLED)
{
}

void
Candle::write(lisp::Writer& writer)
{
  writer.start_list("candle");
  writer.write_float("x", bbox.p1.x);
  writer.write_float("y", bbox.p1.y);
  writer.end_list("candle");
}

HitResponse
Candle::collision(GameObject& , const CollisionHit& )
{
  return FORCE_MOVE;
}

IMPLEMENT_FACTORY(Candle, "candle");
