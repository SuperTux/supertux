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

#ifndef HEADER_SUPERTUX_BADGUY_SHORT_FUSE_HPP
#define HEADER_SUPERTUX_BADGUY_SHORT_FUSE_HPP

#include "badguy/walking_badguy.hpp"

class ShortFuse : public WalkingBadguy
{
public:
  ShortFuse(const ReaderMapping& reader);
  virtual std::string get_class() {
    return "short_fuse";
  }

  virtual ObjectSettings get_settings();

protected:
  HitResponse collision_player (Player& player, const CollisionHit& hit);
  HitResponse collision_bullet (Bullet& bullet, const CollisionHit& );
  bool collision_squished (GameObject& object);
  void kill_fall();
  void ignite();
  void explode();
};

#endif /* HEADER_SUPERTUX_BADGUY_SHORT_FUSE_HPP */

/* EOF */
