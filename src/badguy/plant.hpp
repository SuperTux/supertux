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

#ifndef HEADER_SUPERTUX_BADGUY_PLANT_HPP
#define HEADER_SUPERTUX_BADGUY_PLANT_HPP

#include "badguy/badguy.hpp"

class Plant : public BadGuy
{
public:
  Plant(const Reader& reader);

  void initialize();
  void collision_solid(const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);
  void active_update(float elapsed_time);
  virtual std::string get_class() const {
    return "plant";
  }

  virtual ObjectSettings get_settings();

protected:
  enum PlantState {
    PLANT_SLEEPING,
    PLANT_WAKING,
    PLANT_WALKING
  };

private:
  Timer timer;
  PlantState state;
};

#endif

/* EOF */
