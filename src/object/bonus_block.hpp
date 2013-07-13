//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_BONUS_BLOCK_HPP
#define HEADER_SUPERTUX_OBJECT_BONUS_BLOCK_HPP

#include "object/block.hpp"
#include "object/moving_sprite.hpp"

class BonusBlock : public Block
{
public:
  BonusBlock(const Vector& pos, int data);
  BonusBlock(const Reader& lisp);
  virtual ~BonusBlock();
  HitResponse collision(GameObject& other, const CollisionHit& hit);

  void try_open(Player *player);

  enum Contents {
    CONTENT_COIN,
    CONTENT_FIREGROW,
    CONTENT_ICEGROW,
    CONTENT_STAR,
    CONTENT_1UP,
    CONTENT_CUSTOM,
    CONTENT_SCRIPT,
    CONTENT_LIGHT,
    CONTENT_TRAMPOLINE,
    CONTENT_PORTTRAMPOLINE,
    CONTENT_ROCK,
    CONTENT_RAIN,
    CONTENT_EXPLODE
  };

protected:
  virtual void hit(Player& player);

public:
  Contents contents;
  MovingObject* object;
  int hit_counter;
  void draw(DrawingContext& context);

private:
  BonusBlock(const BonusBlock&);
  BonusBlock& operator=(const BonusBlock&);
  std::string sprite_name;
  std::string script;
  SurfacePtr lightsprite;
};

#endif

/* EOF */
