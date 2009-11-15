//  SuperTux - Ispy
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

#ifndef HEADER_SUPERTUX_OBJECT_ISPY_HPP
#define HEADER_SUPERTUX_OBJECT_ISPY_HPP

#include "object/moving_sprite.hpp"
#include "supertux/direction.hpp"
#include "util/serializable.hpp"

/**
 * An Ispy: When it spots Tux, a script will run.
 */
class Ispy : public MovingSprite, 
             public Serializable
{
public:
  Ispy(const lisp::Lisp& lisp);

  void write(lisp::Writer& writer);
  HitResponse collision(GameObject& other, const CollisionHit& hit);

  virtual void update(float elapsed_time);

private:
  bool line_intersects_line(Vector line1_start, Vector line1_end, Vector line2_start, Vector line2_end);
  bool intersects_line(Rect r, Vector line_start, Vector line_end);
  bool free_line_of_sight(Vector p1, Vector p2, const MovingObject* ignore_object);

  enum IspyState {
    ISPYSTATE_IDLE,
    ISPYSTATE_ALERT,
    ISPYSTATE_HIDING,
    ISPYSTATE_SHOWING
  };
  IspyState state; /**< current state */

  std::string script; /**< script to execute when Tux is spotted */
  Direction dir;

};

#endif

/* EOF */
