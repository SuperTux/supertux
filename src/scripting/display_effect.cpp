//  SuperTux
//  Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
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

#include "scripting/display_effect.hpp"

#include "object/display_effect.hpp"

namespace scripting {

DisplayEffect::DisplayEffect(::DisplayEffect* parent) :
  m_parent(parent)
{
}

DisplayEffect::~DisplayEffect()
{
}

void
DisplayEffect::fade_out(float fadetime)
{
  m_parent->fade_out(fadetime);
}

void
DisplayEffect::fade_in(float fadetime)
{
  m_parent->fade_in(fadetime);
}

void
DisplayEffect::set_black(bool enabled)
{
  m_parent->set_black(enabled);
}

bool
DisplayEffect::is_black() const
{
  return m_parent->is_black();
}

void
DisplayEffect::sixteen_to_nine(float fadetime)
{
  m_parent->sixteen_to_nine(fadetime);
}

void
DisplayEffect::four_to_three(float fadetime)
{
  m_parent->four_to_three(fadetime);
}

} // namespace scripting

/* EOF */
