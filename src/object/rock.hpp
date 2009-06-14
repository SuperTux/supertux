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

#ifndef __ROCK_H__
#define __ROCK_H__

#include "object/moving_sprite.hpp"
#include "physic.hpp"
#include "portable.hpp"
#include "serializable.hpp"

class Sprite;

class Rock : public MovingSprite, public Portable, protected UsesPhysic, public Serializable
{
public:
  Rock(const Vector& pos, std::string spritename);
  Rock(const lisp::Lisp& reader);
  Rock(const lisp::Lisp& reader, std::string spritename);
  virtual Rock* clone() const { return new Rock(*this); }

  void collision_solid(const CollisionHit& hit);
  HitResponse collision(GameObject& other, const CollisionHit& hit);
  void update(float elapsed_time);
  void write(lisp::Writer& writer);

  void grab(MovingObject& object, const Vector& pos, Direction dir);
  void ungrab(MovingObject& object, Direction dir);

protected:
  bool on_ground;
  bool grabbed;
  Vector last_movement;
};

#endif
