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

#include "supertux/game_object_manager.hpp"

#include <algorithm>

#include "object/tilemap.hpp"
#include "supertux/sector.hpp"

GameObjectManager::GameObjectManager() :
  m_gameobjects(),
  m_gameobjects_new()
{
}

GameObjectManager::~GameObjectManager()
{
}

const std::vector<GameObjectPtr>&
GameObjectManager::get_objects() const
{
  return m_gameobjects;
}

void
GameObjectManager::add_object(GameObjectPtr object)
{
  // make sure the object isn't already in the list
#ifndef NDEBUG
  for(const auto& game_object : m_gameobjects) {
    assert(game_object != object);
  }
  for(const auto& gameobject : m_gameobjects_new) {
    assert(gameobject != object);
  }
#endif

  m_gameobjects_new.push_back(object);
}

void
GameObjectManager::update(float delta)
{
  for(const auto& object : m_gameobjects)
  {
    if(!object->is_valid())
      continue;

    object->update(delta);
  }
}

void
GameObjectManager::draw(DrawingContext& context)
{
  for(const auto& object : m_gameobjects)
  {
    if(!object->is_valid())
      continue;

    if (static_cast<Sector*>(this)->s_draw_solids_only) // FIXME: hack
    {
      auto tm = dynamic_cast<TileMap*>(object.get());
      if (tm && !tm->is_solid())
        continue;
    }

    object->draw(context);
  }
}

void
GameObjectManager::update_game_objects()
{
  {
    // cleanup marked objects
    auto erase_it = std::remove_if(m_gameobjects.begin(), m_gameobjects.end(),
                                   [](const GameObjectPtr& obj){ return !obj->is_valid(); });

    for(auto it = erase_it; it != m_gameobjects.end(); ++it) {
      before_object_remove(*it);
    }

    m_gameobjects.erase(erase_it, m_gameobjects.end());
  }

  // add newly created objects
  for(const auto& object : m_gameobjects_new)
  {
    before_object_add(object);

    m_gameobjects.push_back(object);
  }
  m_gameobjects_new.clear();

  // update solid_tilemaps list
  //FIXME: this could be more efficient
  static_cast<Sector*>(this)->m_solid_tilemaps.clear();
  for(const auto& obj : m_gameobjects)
  {
    const auto& tm = dynamic_cast<TileMap*>(obj.get());
    if (!tm) continue;
    if (tm->is_solid()) static_cast<Sector*>(this)->m_solid_tilemaps.push_back(tm);
  }
}

/* EOF */
