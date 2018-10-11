//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2018 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_GAME_OBJECT_MANAGER_HPP
#define HEADER_SUPERTUX_SUPERTUX_GAME_OBJECT_MANAGER_HPP

#include <vector>

#include "supertux/game_object_ptr.hpp"

class DrawingContext;

class GameObjectManager
{
public:
  GameObjectManager();
  virtual ~GameObjectManager();

  void add_object(GameObjectPtr object);

  void update(float delta);
  void draw(DrawingContext& context);

  void update_game_objects();

  /** Hook that is called before an object is added to the vector */
  virtual bool before_object_add(GameObjectPtr object) = 0;

  /** Hook that is called before an object is removed from the vector */
  virtual void before_object_remove(GameObjectPtr object) = 0;

protected:
  std::vector<GameObjectPtr> m_gameobjects;

  /** container for newly created objects, they'll be added in update_game_objects() */
  std::vector<GameObjectPtr> m_gameobjects_new;

private:
  GameObjectManager(const GameObjectManager&) = delete;
  GameObjectManager& operator=(const GameObjectManager&) = delete;
};

#endif

/* EOF */
