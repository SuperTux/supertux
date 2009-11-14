//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __COIN_H__
#define __COIN_H__

#include "moving_sprite.hpp"
#include "lisp/lisp.hpp"

class Coin : public MovingSprite
{
public:
  Coin(const Vector& pos);
  Coin(const lisp::Lisp& reader);

  HitResponse collision(GameObject& other, const CollisionHit& hit);

  void collect();
};

#endif
