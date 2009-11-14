//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef __POISONIVY_H__
#define __POISONIVY_H__

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
