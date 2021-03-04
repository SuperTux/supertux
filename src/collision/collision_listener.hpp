//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_COLLISION_COLLISION_LISTENER_HPP
#define HEADER_SUPERTUX_COLLISION_COLLISION_LISTENER_HPP

class CollisionHit;
class GameObject;

class CollisionListener
{
public:
  virtual ~CollisionListener() {}

  /** this function is called when the object collided with something solid */
  virtual void collision_solid(const CollisionHit& /*hit*/) = 0;

  /** when 2 objects collided, we will first call the
      collision functions of both objects that can decide on
      how to react to the collision. */
  virtual bool collides(GameObject& /*other*/, const CollisionHit& /*hit*/) const = 0;

  /** this function is called when the object collided with any other object */
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) = 0;

  /** called when tiles with special attributes have been touched */
  virtual void collision_tile(uint32_t /*tile_attributes*/) = 0;

  virtual bool listener_is_valid() const = 0;
};

#endif

/* EOF */
