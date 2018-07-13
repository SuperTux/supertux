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

#include "object/spotlight.hpp"
#include "scripting/spotlight.hpp"

namespace scripting {

Spotlight::Spotlight(::Spotlight* spotlight)
  : spotlight(spotlight)
{ }

bool Spotlight::is_emitting() const
{
  return spotlight->is_emitting();
}

void Spotlight::set_emitting(bool emitting)
{
  spotlight->set_emitting(emitting);
}

void Spotlight::set_speed(int rot_speed)
{
  spotlight->set_speed(rot_speed);
}

int Spotlight::get_speed() const
{
  spotlight->get_speed();
}

void Spotlight::set_color(float red, float green, float blue, float alpha)
{
  spotlight->set_color(Color(red, green, blue, alpha));
}


}

/* EOF */
