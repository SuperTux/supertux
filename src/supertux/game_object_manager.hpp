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
class TileMap;

class GameObjectManager
{
public:
  GameObjectManager();
  virtual ~GameObjectManager();

  /** Queue an object up to be added to the object list */
  void add_object(GameObjectPtr object);
  void clear_objects();

  void update(float delta);
  void draw(DrawingContext& context);

  const std::vector<GameObjectPtr>& get_objects() const;

  /** Commit the queued up additions and deletions to the object list */
  void update_game_objects();

  float get_width() const;
  float get_height() const;

  /** returns the width (in tiles) of a worldmap */
  float get_tiles_width() const;

  /** returns the height (in tiles) of a worldmap */
  float get_tiles_height() const;

  /** Hook that is called before an object is added to the vector */
  virtual bool before_object_add(GameObjectPtr object) = 0;

  /** Hook that is called before an object is removed from the vector */
  virtual void before_object_remove(GameObjectPtr object) = 0;

  template<class T>
  T* get_object_by_name(const std::string& name) const
  {
    for(const auto& obj : get_objects())
    {
      if (auto typed_obj = dynamic_cast<T*>(obj.get()))
      {
        if (typed_obj->get_name() == name)
        {
          return typed_obj;
        }
      }
    }
    return nullptr;
  }

  /** Get total number of GameObjects of given type */
  template<class T>
  int get_object_count() const
  {
    int total = 0;
    for(const auto& obj : m_gameobjects) {
      if (dynamic_cast<T*>(obj.get()))
      {
        total += 1;
      }
    }
    return total;
  }

  const std::vector<TileMap*>& get_solid_tilemaps() const { return m_solid_tilemaps; }

public:
  static bool s_draw_solids_only;

private:
  std::vector<GameObjectPtr> m_gameobjects;

  /** container for newly created objects, they'll be added in update_game_objects() */
  std::vector<GameObjectPtr> m_gameobjects_new;

  std::vector<TileMap*> m_solid_tilemaps;

private:
  GameObjectManager(const GameObjectManager&) = delete;
  GameObjectManager& operator=(const GameObjectManager&) = delete;
};

#endif

/* EOF */
