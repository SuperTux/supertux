//  SuperTux
//  Copyright (C) 2008 Wolfgang Becker <uafr@gmx.de>
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

#ifndef HEADER_SUPERTUX_BADGUY_OWL_HPP
#define HEADER_SUPERTUX_BADGUY_OWL_HPP

#include "badguy/badguy.hpp"
#include "object/portable.hpp"

class Owl : public BadGuy
{
public:
  Owl(const Reader& reader);
  Owl(const Vector& pos, Direction d);

  void initialize();
  void collision_solid(const CollisionHit& hit);

protected:
  void active_update (float elapsed_time);
  bool collision_squished(GameObject& object);
  HitResponse collision_player(Player& player, const CollisionHit& hit);

  std::string carried_obj_name;
  Portable *carried_object;
};

#endif /* HEADER_SUPERTUX_BADGUY_OWL_HPP */

/* EOF */
