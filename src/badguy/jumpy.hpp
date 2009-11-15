//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_BADGUY_JUMPY_HPP
#define HEADER_SUPERTUX_BADGUY_JUMPY_HPP

#include "badguy/badguy.hpp"

class Jumpy : public BadGuy
{
public:
  Jumpy(const lisp::Lisp& reader);

  void collision_solid(const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& other, const CollisionHit& hit);

  void write(lisp::Writer& writer);
  void active_update(float);

  void freeze();
  bool is_freezable() const;

private:
  HitResponse hit(const CollisionHit& hit);

private:
  Vector pos_groundhit;
  bool groundhit_pos_set;
};

#endif

/* EOF */
