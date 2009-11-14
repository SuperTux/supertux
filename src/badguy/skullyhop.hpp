//  SkullyHop - A Hopping Skull
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#ifndef __SKULLYHOP_H__
#define __SKULLYHOP_H__

#include "badguy.hpp"

/**
 * Badguy "SkullyHop" - A Hopping Skull
 */
class SkullyHop : public BadGuy
{
public:
  SkullyHop(const lisp::Lisp& reader);
  SkullyHop(const Vector& pos, Direction d);

  void initialize();
  void write(lisp::Writer& writer);
  void collision_solid(const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);
  bool collision_squished(GameObject& object);
  void active_update(float elapsed_time);

  virtual SkullyHop* clone() const { return new SkullyHop(*this); }

protected:
  enum SkullyHopState {
    STANDING,
    CHARGING,
    JUMPING
  };

  Timer recover_timer;
  SkullyHopState state;

  void set_state(SkullyHopState newState);
};

#endif
