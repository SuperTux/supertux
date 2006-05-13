//  $Id$
//
//  Zeekling - flyer that swoops down when she spots the player
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2006 Christoph Sommer <supertux@2006.expires.deltadevelopment.de>
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

#ifndef __ZEEKLING_H__
#define __ZEEKLING_H__

#include "badguy.hpp"

class Zeekling : public BadGuy
{
public:
  Zeekling(const lisp::Lisp& reader);
  Zeekling(const Vector& pos, Direction d);

  void activate();
  void write(lisp::Writer& writer);
  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);
  void active_update(float elapsed_time);

  virtual Zeekling* clone() const { return new Zeekling(*this); }

protected:
  bool collision_squished(Player& player);
  bool set_direction;
  Direction initial_direction;
  float speed;

  Timer diveRecoverTimer;

  enum ZeeklingState {
    FLYING,
    DIVING,
    CLIMBING
  };
  ZeeklingState state;

private:
  bool should_we_dive();
  void onBumpHorizontal();
  void onBumpVertical();

};

#endif

