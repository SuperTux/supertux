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

#ifndef HEADER_SUPERTUX_OBJECT_COIN_HPP
#define HEADER_SUPERTUX_OBJECT_COIN_HPP

#include "object/moving_sprite.hpp"

class TileMap;

class Coin : public MovingSprite
{
public:
  Coin(const Vector& pos);
  Coin(const Vector& pos, TileMap* tilemap);
  Coin(const Reader& reader);

  HitResponse collision(GameObject& other, const CollisionHit& hit);

  void collect();
  virtual void update(float elapsed_time);

private:
  boost::shared_ptr<Path> path;
  boost::shared_ptr<PathWalker> walker;
  Vector offset;
  bool from_tilemap;
};

#endif

/* EOF */
