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
class ParticleSystem;
#endif

namespace scripting {

class ParticleSystem
{
public:
#ifndef SCRIPTING_API
  ParticleSystem(::ParticleSystem* parent);
  virtual ~ParticleSystem();
#endif

  void set_enabled(bool enable);
  bool get_enabled() const;

#ifndef SCRIPTING_API
  ::ParticleSystem* particlesystem;

private:
  ParticleSystem(const ParticleSystem&) = delete;
  ParticleSystem& operator=(const ParticleSystem&) = delete;
#endif
};

}

#endif

/* EOF */
