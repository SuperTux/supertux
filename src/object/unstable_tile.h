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

#ifndef __UNSTABLE_TILE_H__
#define __UNSTABLE_TILE_H__

#include "moving_object.h"
#include "lisp/lisp.h"
#include "math/physic.h"
#include "timer.h"

class Sprite;
class Player;

using namespace SuperTux;

/** A tile that starts falling down if tux stands to long on it */
class UnstableTile : public MovingObject
{
public:
  UnstableTile(const lisp::Lisp& lisp);
  ~UnstableTile();

  HitResponse collision(GameObject& other, const CollisionHit& hit);
  void action(float elapsed_time);
  void draw(DrawingContext& context);

private:
  Physic physic;
  Sprite* sprite;
  Timer2 timer;
  bool hit;
  bool falling;
};

#endif

