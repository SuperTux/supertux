//  MoleRock - Rock thrown by "Mole" Badguy
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

#ifndef HEADER_SUPERTUX_BADGUY_MOLE_ROCK_HPP
#define HEADER_SUPERTUX_BADGUY_MOLE_ROCK_HPP

#include "badguy/badguy.hpp"

/**
 * Badguy "MoleRock" - Rock thrown by "Mole" Badguy
 */
class MoleRock : public BadGuy
{
public:
  MoleRock(const Reader& reader);
  MoleRock(const Vector& pos, const Vector& velocity, const BadGuy* parent);
  ~MoleRock();

  void initialize();
  void deactivate();

  void active_update(float elapsed_time);

  void collision_solid(const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);
  HitResponse collision_player(Player& player, const CollisionHit& hit);

  virtual bool updatePointers(const GameObject* from_object, GameObject* to_object);
  virtual std::string get_class() const {
    return "mole_rock";
  }

  virtual ObjectSettings get_settings();

  bool is_flammable() const;

protected:
  const BadGuy* parent; /**< collisions with this BadGuy will be ignored */
  const Vector initial_velocity; /**< velocity at time of creation */

private:
  MoleRock(const MoleRock&);
  MoleRock& operator=(const MoleRock&);
};

#endif

/* EOF */
