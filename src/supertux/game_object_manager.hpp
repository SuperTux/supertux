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

#include <unordered_map>
#include <vector>

#include "supertux/game_object.hpp"
#include "supertux/game_object_ptr.hpp"
#include "util/uid_generator.hpp"

class DrawingContext;
class TileMap;

class GameObjectManager
{
public:
  static bool s_draw_solids_only;

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
  T* get_object_by_uid(const UID& uid) const
  {
    auto it = m_objects_by_uid.find(uid);
    if (it != m_objects_by_uid.end())
    {
      return nullptr;
    }
    else
    {
      return dynamic_cast<T*>(it->second);
    }
  }

  template<class T>
  T* get_object_by_name(const std::string& name) const
  {
    auto it = m_objects_by_name.find(name);
    if (it == m_objects_by_name.end())
    {
      return nullptr;
    }
    else
    {
      return dynamic_cast<T*>(it->second);
    }
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

private:
  void this_before_object_add(const GameObjectPtr& object);
  void this_before_object_remove(const GameObjectPtr& object);

private:
  UIDGenerator m_uid_generator;

  std::vector<GameObjectPtr> m_gameobjects;

  /** container for newly created objects, they'll be added in update_game_objects() */
  std::vector<GameObjectPtr> m_gameobjects_new;

  /** Fast access to solid tilemaps */
  std::vector<TileMap*> m_solid_tilemaps;

  std::unordered_map<std::string, GameObject*> m_objects_by_name;
  std::unordered_map<UID, GameObject*> m_objects_by_uid;

private:
  GameObjectManager(const GameObjectManager&) = delete;
  GameObjectManager& operator=(const GameObjectManager&) = delete;
};

#endif

/* EOF */
