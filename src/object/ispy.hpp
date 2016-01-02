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

/**
 * An Ispy: When it spots Tux, a script will run.
 */
class Ispy : public MovingSprite
{
public:
  Ispy(const ReaderMapping& lisp);
  virtual void save(Writer& writer);

  HitResponse collision(GameObject& other, const CollisionHit& hit);

  virtual void update(float elapsed_time);
  virtual std::string get_class() const {
    return "ispy";
  }

  virtual ObjectSettings get_settings();

private:
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
