//  SuperTux - BicyclePlatform
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_BICYCLE_PLATFORM_HPP
#define HEADER_SUPERTUX_OBJECT_BICYCLE_PLATFORM_HPP

#include "object/moving_sprite.hpp"

/**
 * Used to construct a pair of bicycle platforms: If one is pushed down, the other one rises
 */
class BicyclePlatform : public MovingSprite
{
public:
  BicyclePlatform(const Reader& reader);
  BicyclePlatform(BicyclePlatform* master);
  virtual ~BicyclePlatform();

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);
  virtual void update(float elapsed_time);

protected:
  BicyclePlatform* master; /**< pointer to BicyclePlatform that does movement calculation */
  BicyclePlatform* slave; /**< pointer to BicyclePlatform that reacts to master platform's movement calculation */
  Vector center; /**< pivot point */
  float radius; /**< radius of circle */
  float angle; /**< current angle */
  float angular_speed; /**< angular speed in rad per second */
  std::set<GameObject*> contacts; /**< objects that are currently pushing on the platform */
  float momentum; /** angular momentum in rad per second per second*/

};

#endif

/* EOF */
