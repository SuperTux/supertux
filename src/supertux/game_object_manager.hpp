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

#include <functional>
#include <iostream>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "supertux/game_object.hpp"
#include "util/uid_generator.hpp"

class DrawingContext;
class ReaderMapping;
class TileMap;

template<class T> class GameObjectRange;

class GameObjectManager
{
public:
  static bool s_draw_solids_only;

private:
  struct NameResolveRequest
  {
    std::string name;
    std::function<void (UID)> callback;
  };

public:
  GameObjectManager();
  virtual ~GameObjectManager();

  /** Queue an object up to be added to the object list */
  GameObject& add_object(std::unique_ptr<GameObject> object);
  void clear_objects();

  template<typename T, typename... Args>
  T& add(Args&&... args)
  {
    auto obj = std::make_unique<T>(std::forward<Args>(args)...);
    T& obj_ref = *obj;
    add_object(std::move(obj));
    return obj_ref;
  }

  void update(float dt_sec);
  void draw(DrawingContext& context);

  const std::vector<std::unique_ptr<GameObject> >& get_objects() const;

  /** Commit the queued up additions and deletions to the object list */
  void flush_game_objects();

  float get_width() const;
  float get_height() const;

  /** returns the width (in tiles) of a worldmap */
  float get_tiles_width() const;

  /** returns the height (in tiles) of a worldmap */
  float get_tiles_height() const;

  /** Hook that is called before an object is added to the vector */
  virtual bool before_object_add(GameObject& object) = 0;

  /** Hook that is called before an object is removed from the vector */
  virtual void before_object_remove(GameObject& object) = 0;

  /** Writes the session's full state to the writer, so that another
      instance of the game can reproduce it exactly. */
  virtual void backup(Writer& writer);

  /** Restores this session's internal status from a savestate */
  virtual void restore(const ReaderMapping& reader);

  template<class T>
  GameObjectRange<T> get_objects_by_type() const
  {
    return GameObjectRange<T>(*this);
  }

  const std::vector<GameObject*>&
  get_objects_by_type_index(std::type_index type_idx) const
  {
    auto it = m_objects_by_type_index.find(type_idx);
    if (it == m_objects_by_type_index.end()) {
      // use a dummy return value to avoid making this method non-const
      static std::vector<GameObject*> dummy;
      return dummy;
    } else {
      return it->second;
    }
  }

  template<class T>
  T& get_singleton_by_type() const
  {
    const auto& range = get_objects_by_type<T>();
    assert(range.begin() != range.end());
    assert(range.begin()->is_singleton());
    return *range.begin();
  }

  template<class T>
  T* get_object_by_uid(const UID& uid) const
  {
    auto it = m_objects_by_uid.find(uid);
    if (it == m_objects_by_uid.end())
    {
      // FIXME: Is this a good idea? Should gameobjects be made
      // accessible even when not fully inserted into the manager?
      for (auto&& itnew : m_gameobjects_new)
      {
        if (itnew->get_uid() == uid)
          return static_cast<T*>(itnew.get());
      }
      return nullptr;
    }
    else
    {
#ifdef NDEBUG
      return static_cast<T*>(it->second);
#else
      // Since uids should be unique, there should be no need to guess
      // the type, thus we assert() when the object type is not what
      // we expected.
      auto ptr = dynamic_cast<T*>(it->second);
      assert(ptr != nullptr);
      return ptr;
#endif
    }
  }

  /** Register a callback to be called once the given name can be
      resolsed to a UID. Note that this function is only valid in the
      construction phase, not during draw() or update() calls, use
      get_object_by_uid() instead. */
  void request_name_resolve(const std::string& name, std::function<void (UID)> callback);

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
  int get_object_count(std::function<bool(const T&)> predicate = nullptr) const
  {
    int total = 0;
    for (const auto& obj : m_gameobjects) {
      auto object = dynamic_cast<T*>(obj.get());
      if (object && (predicate == nullptr || predicate(*object)))
      {
        total += 1;
      }
    }
    return total;
  }

  const std::vector<TileMap*>& get_solid_tilemaps() const { return m_solid_tilemaps; }
  
  void update_solids();
  void update_solid(TileMap* solid);

protected:
  void process_resolve_requests();

  /** Same as process_resolve_requests(), but those it can't find will be kept in the buffer */
  void try_process_resolve_requests();

  template<class T>
  T* get_object_by_type() const
  {
    const auto& range = get_objects_by_type<T>();
    if (range.begin() == range.end()) {
      return nullptr;
    } else {
      return &*range.begin();
    }
  }

private:
  void this_before_object_add(GameObject& object);
  void this_before_object_remove(GameObject& object);

private:
  UIDGenerator m_uid_generator;

  std::vector<std::unique_ptr<GameObject>> m_gameobjects;

  /** container for newly created objects, they'll be added in flush_game_objects() */
  std::vector<std::unique_ptr<GameObject>> m_gameobjects_new;

  /** Fast access to solid tilemaps */
  std::vector<TileMap*> m_solid_tilemaps;

  std::unordered_map<std::string, GameObject*> m_objects_by_name;
  std::unordered_map<UID, GameObject*> m_objects_by_uid;
  std::unordered_map<std::type_index, std::vector<GameObject*> > m_objects_by_type_index;

  std::vector<NameResolveRequest> m_name_resolve_requests;

private:
  GameObjectManager(const GameObjectManager&) = delete;
  GameObjectManager& operator=(const GameObjectManager&) = delete;
};

#include "supertux/game_object_iterator.hpp"

#endif

/* EOF */
