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

#ifndef __INVISIBLE_BLOCK_H__
#define __INVISIBLE_BLOCK_H__

#include "block.hpp"

class InvisibleBlock : public Block
{
public:
  InvisibleBlock(const Vector& pos);

  virtual void draw(DrawingContext& context);
  virtual bool collides(GameObject& other, const CollisionHit& hit);
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);

protected:
  virtual void hit(Player& player);

private:
  bool visible;
};

#endif
