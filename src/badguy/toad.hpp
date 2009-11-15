//  Toad - A jumping toad
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_BADGUY_TOAD_HPP
#define HEADER_SUPERTUX_BADGUY_TOAD_HPP

#include "badguy/badguy.hpp"

/**
 * Badguy "Toad" - A jumping toad
 */
class Toad : public BadGuy
{
public:
  Toad(const lisp::Lisp& reader);
  Toad(const Vector& pos, Direction d);

  void initialize();
  void write(lisp::Writer& writer);
  void collision_solid(const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);
  bool collision_squished(GameObject& object);
  void active_update(float elapsed_time);

protected:
  enum ToadState {
    IDLE,
    JUMPING,
    FALLING
  };

private:
  void set_state(ToadState newState);

private:
  Timer recover_timer;
  ToadState state;
};

#endif

/* EOF */
