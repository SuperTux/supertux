//  SuperTux - Mole Badguy
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

#ifndef HEADER_SUPERTUX_BADGUY_MOLE_HPP
#define HEADER_SUPERTUX_BADGUY_MOLE_HPP

#include "badguy/badguy.hpp"

class Mole : public BadGuy
{
public:
  Mole(const lisp::Lisp& );
  Mole(const Vector& pos);

  void kill_fall();
  HitResponse collision_badguy(BadGuy& , const CollisionHit& );
  bool collision_squished(GameObject& object);

  void activate();
  void write(lisp::Writer& );
  void active_update(float);

  virtual Mole* clone() const { return new Mole(*this); }

private:
  enum MoleState {
    PRE_THROWING,
    THROWING,
    POST_THROWING,
    PEEKING,
    DEAD
  };

  MoleState state;
  Timer timer;
  Timer throw_timer;

  void set_state(MoleState new_state);
  void throw_rock();

};

#endif

/* EOF */
