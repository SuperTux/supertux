//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2017 M. Teufel <mteufel@supertux.org>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_TORCH_HPP
#define HEADER_SUPERTUX_SCRIPTING_TORCH_HPP

#ifndef SCRIPTING_API
class Torch;
#endif

namespace scripting {

class Torch
{
public:
#ifndef SCRIPTING_API
  Torch(::Torch* torch);
  ~Torch();
#endif

  bool get_burning() const; /**< returns true if torch is lighted */
  void set_burning(bool burning); /**< true: light torch, false: extinguish torch */

#ifndef SCRIPTING_API
  ::Torch* torch;

private:
  Torch(const Torch&);
  Torch& operator=(const Torch&);
#endif
};

}

#endif

/* EOF */
