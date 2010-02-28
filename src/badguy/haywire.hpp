//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2010 Florian Forster <supertux at octo.it>
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

#ifndef HEADER_SUPERTUX_BADGUY_HAYWIRE_HPP
#define HEADER_SUPERTUX_BADGUY_HAYWIRE_HPP

#include "badguy/walking_badguy.hpp"

class Haywire : public WalkingBadguy
{
public:
  Haywire(const Reader& reader);
  Haywire(const Vector& pos, Direction d);

  void kill_fall();
  HitResponse collision(GameObject& object, const CollisionHit& hit);
  HitResponse collision_player(Player& player, const CollisionHit& hit);

  void active_update(float elapsed_time);

  void freeze();
  bool is_freezable() const;

protected:
  bool collision_squished(GameObject& object);

private:
  bool is_exploding;
  float time_until_explosion;
  bool is_stunned;
  float time_stunned;
};

#endif /* HEADER_SUPERTUX_BADGUY_HAYWIRE_HPP */

/* EOF */
