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

#ifndef HEADER_SUPERTUX_BADGUY_MRROCKET_HPP
#define HEADER_SUPERTUX_BADGUY_MRROCKET_HPP

#include "badguy/badguy.hpp"

class MrRocket : public BadGuy
{
public:
  MrRocket(const Reader& reader);
  MrRocket(const Vector& pos, Direction d);

  void initialize();
  void active_update(float elapsed_time);
  void collision_solid(const CollisionHit& hit);

protected:
  bool collision_squished(GameObject& object);

private:
  Timer collision_timer;
};

#endif

/* EOF */
