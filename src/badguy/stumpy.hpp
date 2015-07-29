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

#ifndef HEADER_SUPERTUX_BADGUY_STUMPY_HPP
#define HEADER_SUPERTUX_BADGUY_STUMPY_HPP

#include "badguy/walking_badguy.hpp"

class Stumpy : public WalkingBadguy
{
public:
  Stumpy(const Reader& reader);
  Stumpy(const Vector& pos, Direction d);

  void initialize();
  void active_update(float elapsed_time);
  void collision_solid(const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);

  bool is_freezable() const;
  virtual std::string get_class() {
    return "stumpy";
  }
protected:
  enum MyState {
    STATE_INVINCIBLE, STATE_NORMAL
  };

protected:
  bool collision_squished(GameObject& object);

private:
  MyState mystate;
  Timer   invincible_timer;
};

#endif

/* EOF */
