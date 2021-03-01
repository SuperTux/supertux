//  SuperTux
//  Copyright (C) 2020 Grzegorz Przybylski <zwatotem@gmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_DECAL_HPP
#define HEADER_SUPERTUX_SCRIPTING_DECAL_HPP

#ifndef SCRIPTING_API
#include "scripting/game_object.hpp"

class Decal;
#endif

namespace scripting {

class Decal final
#ifndef SCRIPTING_API
  : public GameObject<::Decal>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;
private:
  Decal(const Decal&) = delete;
  Decal& operator=(const Decal&) = delete;
#endif

public:
  void fade_sprite(const std::string& new_sprite_name, float fade_time);
  void change_sprite(const std::string& new_sprite_name);
  void fade_in(float fade_time);
  void fade_out(float fade_time);
};

} // namespace scripting

#endif

/* EOF */
