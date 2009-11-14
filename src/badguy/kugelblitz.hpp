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

#ifndef __KUGELBLITZ_H__
#define __KUGELBLITZ_H__

#include "badguy.hpp"
#include "timer.hpp"
#include "object/electrifier.hpp"

class Kugelblitz : public BadGuy
{
public:
  Kugelblitz(const lisp::Lisp& reader);

  void initialize();
  HitResponse collision_badguy(BadGuy& other, const CollisionHit& hit);
  void collision_solid(const CollisionHit& hit);
  HitResponse collision_player(Player& player, const CollisionHit& hit);

  void write(lisp::Writer& writer);
  void active_update(float);
  void kill_fall();
  void explode();

  virtual Kugelblitz* clone() const { return new Kugelblitz(*this); }

private:
  void try_activate();
  HitResponse hit(const CollisionHit& hit);
  Vector pos_groundhit;
  bool groundhit_pos_set;
  bool dying;
  Timer movement_timer;
  Timer lifetime;
  int direction;
  State state;
};

#endif
