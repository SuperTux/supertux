//  $Id$
//
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

#ifndef __ROCKETEXPLOSION_H__
#define __ROCKETEXPLOSION_H__

#include "badguy.hpp"

class RocketExplosion : public BadGuy
{
public:
  RocketExplosion(const Vector& pos, Direction dir);

  void write(lisp::Writer& writer);
  HitResponse collision_player(Player& player, const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);
  void active_update(float elapsed_time);
  void kill_fall();
  void explode();

  virtual RocketExplosion* clone() const { return new RocketExplosion(*this); }

private:
  Timer timer;
};

#endif
