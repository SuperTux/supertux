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

#ifndef __JUMPY_H__
#define __JUMPY_H__

#include "badguy.hpp"

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

  virtual Jumpy* clone() const { return new Jumpy(*this); }

private:
  HitResponse hit(const CollisionHit& hit);
  Vector pos_groundhit;
  bool groundhit_pos_set;
};

#endif
