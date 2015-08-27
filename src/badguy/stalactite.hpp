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

#ifndef HEADER_SUPERTUX_BADGUY_STALACTITE_HPP
#define HEADER_SUPERTUX_BADGUY_STALACTITE_HPP

#include "badguy/badguy.hpp"

class Stalactite : public BadGuy
{
public:
  Stalactite(const ReaderMapping& reader);

  void active_update(float elapsed_time);
  void collision_solid(const CollisionHit& hit);
  HitResponse collision_player(Player& player, const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& other, const CollisionHit& hit);
  HitResponse collision_bullet(Bullet& bullet, const CollisionHit& );

  void kill_fall();
  void draw(DrawingContext& context);
  void deactivate();

  void squish();
  virtual std::string get_class() const {
    return "stalactite";
  }

  virtual ObjectSettings get_settings();

protected:
  enum StalactiteState {
    STALACTITE_HANGING,
    STALACTITE_SHAKING,
    STALACTITE_FALLING,
    STALACTITE_SQUISHED
  };

protected:
  Timer timer;
  StalactiteState state;
  Vector shake_delta;
};

#endif

/* EOF */
