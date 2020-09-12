//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_CLOUDS_HPP
#define HEADER_SUPERTUX_SCRIPTING_CLOUDS_HPP

#ifndef SCRIPTING_API
#include "scripting/game_object.hpp"

class CloudParticleSystem;
#endif

namespace scripting {

class Clouds final
#ifndef SCRIPTING_API
  : public GameObject<::CloudParticleSystem>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  Clouds(const Clouds&) = delete;
  Clouds& operator=(const Clouds&) = delete;
#endif

public:
  void set_enabled(bool enable);
  bool get_enabled() const;

  /** Smoothly changes the rain speed to the given value */
  void fade_speed(float speed, float time);

  /** Smoothly changes the amount of particles to the given value */
  void fade_amount(int amount, float time, float time_between);

  /** Smoothly changes the amount of particles to the given value */
  void set_amount(int amount, float time);
};

} // namespace scripting

#endif

/* EOF */
