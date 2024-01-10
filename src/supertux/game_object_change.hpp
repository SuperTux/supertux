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

#ifndef HEADER_SUPERTUX_SUPERTUX_GAME_OBJECT_CHANGE_HPP
#define HEADER_SUPERTUX_SUPERTUX_GAME_OBJECT_CHANGE_HPP

#include <string>

#include "util/uid.hpp"

class ReaderMapping;
class Writer;

/** Stores a change in a GameObject's state. */
class GameObjectChange final
{
  friend class GameObjectManager;

private:
  GameObjectChange(const std::string& name, const UID& uid,
                   const std::string& data, bool creation);

public:
  GameObjectChange(const ReaderMapping& reader);

  void save(Writer& writer) const;

private:
  std::string name;
  UID uid;
  std::string data;
  bool creation; // Set when the change represents an object creation
};

/** Stores multiple GameObjectChanges. */
class GameObjectChanges final
{
  friend class GameObjectManager;

private:
  GameObjectChanges(const UID& uid, std::vector<GameObjectChange> objects);

public:
  GameObjectChanges(const ReaderMapping& reader);

  void save(Writer& writer) const;

private:
  UID uid;
  std::vector<GameObjectChange> objects;
};

#endif

/* EOF */
