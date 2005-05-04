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
#ifndef __ONEUP_H__
#define __ONEUP_H__

#include "moving_object.h"
#include "sprite/sprite.h"
#include "physic.h"

class OneUp : public MovingObject
{
public:
  OneUp(const Vector& pos);
  ~OneUp();

  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);

private:
  Sprite* sprite;
  Physic physic;
};

#endif
