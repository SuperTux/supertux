//  SuperTux badguy - Ghostflame a flame-like enemy that cannot be killed
//  Copyright (C) 2013 LMH <lmh.0013@gmail.com>
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

#include "badguy/ghostflame.hpp"

Ghostflame::Ghostflame(const ReaderMapping& reader) :
  Flame(reader, "images/creatures/flame/ghostflame.sprite")
{
  m_lightsprite->set_color(Color(0.21f, 0.00f, 0.21f));
  m_allowed_directions = {};
}

bool
Ghostflame::is_flammable() const
{
  return false;
}

bool
Ghostflame::is_freezable() const
{
  return false;
}

/* EOF */
