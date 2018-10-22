//  SuperTux - Sector scripting
//  Copyright (C) 2016 Tobias Markus <tobbi.bugs@gmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_PARTICLESYSTEM_HPP
#define HEADER_SUPERTUX_SCRIPTING_PARTICLESYSTEM_HPP

#ifndef SCRIPTING_API
#include "scripting/game_object.hpp"

class ParticleSystem;
#endif

namespace scripting {

class ParticleSystem final
#ifndef SCRIPTING_API
  : public GameObject<::ParticleSystem>
#endif
{
public:
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  ParticleSystem(const ParticleSystem&) = delete;
  ParticleSystem& operator=(const ParticleSystem&) = delete;
#endif

public:
  void set_enabled(bool enable);
  bool get_enabled() const;
};

} // namespace scripting

#endif

/* EOF */
