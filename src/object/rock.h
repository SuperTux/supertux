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

#ifndef __ROCK_H__
#define __ROCK_H__

#include "moving_object.h"
#include "math/physic.h"
#include "lisp/lisp.h"
#include "portable.h"
#include "serializable.h"

class Sprite;

class Rock : public MovingObject, public Portable, public Serializable
{
public:
  Rock(const lisp::Lisp& reader);
  virtual ~Rock();

  HitResponse collision(GameObject& other, const CollisionHit& hit);
  void action(float elapsed_time);
  void draw(DrawingContext& context);
  void write(lisp::Writer& writer);
    
  void grab(MovingObject& object, const Vector& pos);

private:
  bool grabbed;
  Sprite* sprite;
  Physic physic;
};

#endif
