//  $Id: flyingsnowball.hpp 2642 2005-06-26 13:38:53Z matzebraun $
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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

#ifndef __SPIDERMITE_H__
#define __SPIDERMITE_H__

#include "badguy.hpp"

class SpiderMite : public BadGuy
{
public:
  SpiderMite(const lisp::Lisp& reader);
  SpiderMite(float pos_x, float pos_y);

  void activate();
  void write(lisp::Writer& writer);
  void active_update(float elapsed_time);
  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);
protected:
  enum SpiderMiteMode {
    FLY_UP,
    FLY_DOWN
  };
  SpiderMiteMode mode;
  bool collision_squished(Player& player);
private:
  Timer timer;
};

#endif

