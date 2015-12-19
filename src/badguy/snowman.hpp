//  SuperTux
//  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_BADGUY_SNOWMAN_HPP
#define HEADER_SUPERTUX_BADGUY_SNOWMAN_HPP

#include "badguy/walking_badguy.hpp"

#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"

class Snowman : public WalkingBadguy
{
public:
  Snowman(const Reader& reader);
  Snowman(const Vector& pos, Direction d);
  virtual std::string get_class() const {
    return "snowman";
  }

  virtual ObjectSettings get_settings();

protected:
  void loose_head();
  virtual HitResponse collision_bullet(Bullet& bullet, const CollisionHit& hit);
  bool collision_squished(GameObject& object);

};

#endif

/* EOF */
