//  SuperTux
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_GAME_OBJECT_MANAGER_HPP
#define HEADER_SUPERTUX_SCRIPTING_GAME_OBJECT_MANAGER_HPP

#ifndef SCRIPTING_API
#include <string>
class GameObjectManager;
#endif

namespace scripting {

/** Superclass for sectors and worldmaps */
class GameObjectManager
{
#ifndef SCRIPTING_API
private:
  ::GameObjectManager* m_gom_parent;

public:
  GameObjectManager(::GameObjectManager* parent);

private:
  GameObjectManager(const GameObjectManager&) = delete;
  GameObjectManager& operator=(const GameObjectManager&) = delete;
#endif

public:
  void set_ambient_light(float red, float green, float blue);
  void fade_to_ambient_light(float red, float green, float blue, float fadetime);
  float get_ambient_red() const;
  float get_ambient_green() const;
  float get_ambient_blue() const;

  void set_music(const std::string& music);
};

} // namespace scripting

#endif

/* EOF */
