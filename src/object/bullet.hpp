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

#ifndef __BULLET_H__
#define __BULLET_H__

#include "moving_object.hpp"
#include "physic.hpp"
#include "sprite/sprite.hpp"

enum BulletsKind {
  FIRE_BULLET,
  ICE_BULLET
};

class Bullet : public MovingObject
{
public:
  Bullet(const Vector& pos, float xm, int dir, int kind);
  ~Bullet();
  
  void update(float elapsed_time);
  void draw(DrawingContext& context);
  void collision_solid(const CollisionHit& hit);
  HitResponse collision(GameObject& other, const CollisionHit& hit);

  int kind;        
  
private:
  int life_count;
  Physic physic;
  Sprite* sprite;
};

#endif
