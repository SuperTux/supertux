//  SuperTux - PneumaticPlatform
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

#ifndef HEADER_SUPERTUX_OBJECT_PNEUMATIC_PLATFORM_HPP
#define HEADER_SUPERTUX_OBJECT_PNEUMATIC_PLATFORM_HPP

#include <memory>
#include <string>
#include <set>
#include "object/moving_sprite.hpp"
#include "object/path.hpp"
#include "object/path_walker.hpp"

/**
 * Used to construct a pair of pneumatic platforms: If one is pushed down, the other one rises
 */
class PneumaticPlatform : public MovingSprite
{
public:
  PneumaticPlatform(const lisp::Lisp& reader);
  PneumaticPlatform(PneumaticPlatform* master);
  virtual ~PneumaticPlatform();

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);
  virtual void update(float elapsed_time);

protected:
  PneumaticPlatform* master; /**< pointer to PneumaticPlatform that does movement calculation */
  PneumaticPlatform* slave; /**< pointer to PneumaticPlatform that reacts to master platform's movement calculation */
  float start_y; /**< vertical start position */
  float offset_y; /**< vertical offset from the start position in px */
  float speed_y; /**< vertical speed */
  std::set<GameObject*> contacts; /**< objects that are currently pushing on the platform */

};

#endif
