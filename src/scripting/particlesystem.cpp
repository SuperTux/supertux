//  SuperTux
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

#include "scripting/particlesystem.hpp"

#include "object/particlesystem.hpp"

namespace scripting {

ParticleSystem::ParticleSystem(::ParticleSystem* parent) :
  particlesystem(parent)
{
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::set_enabled(bool enable)
{
  particlesystem->set_enabled(enable);
}

bool ParticleSystem::get_enabled() const
{
  return particlesystem->get_enabled();
}

}

/* EOF */
