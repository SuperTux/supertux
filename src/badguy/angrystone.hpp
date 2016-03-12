//  AngryStone - A spiked block that charges towards the player
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_BADGUY_ANGRYSTONE_HPP
#define HEADER_SUPERTUX_BADGUY_ANGRYSTONE_HPP

#include "badguy/badguy.hpp"

class AngryStone : public BadGuy
{
public:
  AngryStone(const ReaderMapping& reader);

  void collision_solid(const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);
  void active_update(float elapsed_time);
  void kill_fall();
  bool is_freezable() const;
  bool is_flammable() const;
  virtual std::string get_class() {
    return "angrystone";
  }

  virtual ObjectSettings get_settings();

protected:
  enum AngryStoneState {
    IDLE,
    CHARGING,
    ATTACKING,
    RECOVERING
  };

private:
  Vector attackDirection;  /**< 1-normalized vector of current attack direction */
  Vector oldWallDirection; /**< if wall was hit during last attack: 1-normalized vector of last attack direction, (0,0) otherwise */
  Timer timer;
  AngryStoneState state;
};

#endif

/* EOF */
