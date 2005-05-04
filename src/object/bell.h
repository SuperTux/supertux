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
#ifndef __BELL_H__
#define __BELL_H__

#include "lisp/lisp.h"
#include "moving_object.h"
#include "sprite/sprite.h"
#include "serializable.h"

/**
 * A bell: When tux touches it, it begins ringing and you will respawn at this
 * position.
 */
class Bell : public MovingObject, public Serializable
{
public:
  Bell(const lisp::Lisp& lisp);
  ~Bell();

  void write(lisp::Writer& writer);
  void action(float elapsed_time);
  void draw(DrawingContext& context);
  HitResponse collision(GameObject& other, const CollisionHit& hit);

private:
  Sprite* sprite;
  bool ringing;
};

#endif

