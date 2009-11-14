//  SuperTux - Trampoline
//  Copyright (C) 2006 Wolfgang Becker <uafr@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SUPERTUX_TRAMPOLINE_H
#define SUPERTUX_TRAMPOLINE_H

#include "moving_sprite.hpp"
#include "lisp/lisp.hpp"
#include "object/rock.hpp"

/**
 * Jumping on a trampoline makes tux jump higher.
 */
class Trampoline : public Rock
{
public:
  Trampoline(const lisp::Lisp& reader);

  HitResponse collision(GameObject& other, const CollisionHit& hit);
  void collision_solid(const CollisionHit& hit);
  void update(float elapsed_time);

  void grab(MovingObject&, const Vector& pos, Direction);
  void ungrab(MovingObject&, Direction);
  bool is_portable() const;

private:
  bool portable;

};

#endif
