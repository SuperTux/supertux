//  SuperTux - Decal
//  Copyright (C) 2008 Christoph Sommer <christoph.sommer@2008.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_DECAL_HPP
#define HEADER_SUPERTUX_OBJECT_DECAL_HPP

#include "object/moving_sprite.hpp"

class ReaderMapping;

/**
 * A decorative image, perhaps part of the terrain
 */
class Decal : public MovingSprite
{
public:
  Decal(const ReaderMapping& reader);
  virtual ~Decal();
  virtual void save(lisp::Writer& writer);

  virtual HitResponse collision(GameObject& , const CollisionHit& ) {
    return FORCE_MOVE;
  }
  virtual std::string get_class() {
    return "decal";
  }

private:
  std::string default_action;
};

#endif /*SUPERTUX_DECAL_H*/

/* EOF */
