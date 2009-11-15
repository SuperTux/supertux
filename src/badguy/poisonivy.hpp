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

#ifndef HEADER_SUPERTUX_BADGUY_POISONIVY_HPP
#define HEADER_SUPERTUX_BADGUY_POISONIVY_HPP

#include "walking_badguy.hpp"

class PoisonIvy : public WalkingBadguy
{
public:
  PoisonIvy(const lisp::Lisp& reader);
  PoisonIvy(const Vector& pos, Direction d);

  void write(lisp::Writer& writer);
  virtual PoisonIvy* clone() const { return new PoisonIvy(*this); }

protected:
  bool collision_squished(GameObject& object);

};

#endif
