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

#ifndef __FISH_H__
#define __FISH_H__

#include "badguy.hpp"

class Fish : public BadGuy
{
public:
  Fish(const lisp::Lisp& );
  Fish(const Vector& pos);

  void draw(DrawingContext& context);

  void collision_solid(const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& , const CollisionHit& );
  void collision_tile(uint32_t tile_attributes);

  void write(lisp::Writer& );
  void active_update(float);

  void freeze();
  void unfreeze();
  bool is_freezable() const;

  virtual Fish* clone() const { return new Fish(*this); }

private:
  HitResponse hit(const CollisionHit& );
  void start_waiting();
  void jump();

  Timer waiting;
  float stop_y; /**< y-coordinate to stop at */
};

#endif
