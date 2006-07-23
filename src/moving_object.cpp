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

#include "moving_object.hpp"

MovingObject::MovingObject(std::string name) :
  GameObject(name), bbox(0, 0, 0, 0), group(COLGROUP_MOVING), solid(false)
{
}

MovingObject::MovingObject(const lisp::Lisp& lisp) :
  GameObject(lisp), bbox(0, 0, 0, 0), group(COLGROUP_MOVING)
{
  lisp.get("x", bbox.p1.x);
  lisp.get("y", bbox.p1.y);
  lisp.get("w", bbox.p2.x);
  lisp.get("h", bbox.p2.y);
  lisp.get("solid", solid);
  bbox.p2.x+=bbox.p1.x;
  bbox.p2.y+=bbox.p1.y;
}

MovingObject::MovingObject(Rect bbox, CollisionGroup group, bool solid) :
  bbox(bbox), group(group), solid(solid)
{
}

MovingObject::~MovingObject()
{
}
