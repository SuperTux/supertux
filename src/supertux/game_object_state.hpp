//  SuperTux
//  Copyright (C) 2024 Vankata453
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

#ifndef HEADER_SUPERTUX_SUPERTUX_GAME_OBJECT_STATE_HPP
#define HEADER_SUPERTUX_SUPERTUX_GAME_OBJECT_STATE_HPP

#include <string>

#include "util/uid.hpp"

class GameObject;
class ReaderMapping;
class Writer;

/** Stores a change in a GameObject's state. */
class GameObjectState final
{
public:
  enum class Action
  {
    NONE,
    CREATE,
    DELETE,
    MODIFY
  };

public:
  GameObjectState(const std::string& name, const UID& uid,
                  const std::string& data, Action action);
  GameObjectState(const ReaderMapping& reader);

  void save(Writer& writer) const;

public:
  std::string name;
  UID uid;
  std::string data;
  Action action; // The action which triggered a state change
};

/** Stores multiple GameObjectStates. */
class GameObjectStates final
{
public:
  GameObjectStates(const UID& uid, std::vector<GameObjectState> objects);
  GameObjectStates(const ReaderMapping& reader);

  void save(Writer& writer) const;

public:
  UID uid;
  std::vector<GameObjectState> objects;
};

#endif

/* EOF */
