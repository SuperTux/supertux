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

bool GameObjectManager::s_draw_solids_only = false;

GameObjectManager::GameObjectManager() :
  m_uid_generator(),
  m_gameobjects(),
  m_gameobjects_new(),
  m_solid_tilemaps(),
  m_objects_by_name(),
  m_objects_by_uid()
{
}

GameObjectManager::~GameObjectManager()
{
  // clear_objects() must be called before destructing the GameObjectManager
  assert(m_gameobjects.size() == 0);
  assert(m_gameobjects_new.size() == 0);
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
GameObjectManager::clear_objects()
{
  update_game_objects();

  for(const auto& obj: m_gameobjects) {
    before_object_remove(obj);
  }
  m_gameobjects.clear();
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

    if (s_draw_solids_only)
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
  { // cleanup marked objects
    m_gameobjects.erase(
      std::remove_if(m_gameobjects.begin(), m_gameobjects.end(),
                     [this](const GameObjectPtr& obj) {
                       if (!obj->is_valid())
                       {
                         this_before_object_remove(obj);
                         before_object_remove(obj);
                         return true;
                       } else {
                         return false;
                       }
                     }),
      m_gameobjects.end());
  }

  { // add newly created objects
    for(const auto& object : m_gameobjects_new)
    {
      if (before_object_add(object))
      {
        object->set_uid(m_uid_generator.next());
        this_before_object_add(object);
        m_gameobjects.push_back(object);
      }
    }
    m_gameobjects_new.clear();
  }

  { // update solid_tilemaps list
    m_solid_tilemaps.clear();
    for(const auto& obj : m_gameobjects)
    {
      const auto& tm = dynamic_cast<TileMap*>(obj.get());
      if (!tm) continue;
      if (tm->is_solid()) static_cast<Sector*>(this)->m_solid_tilemaps.push_back(tm);
    }
  }
}

void
GameObjectManager::this_before_object_add(const GameObjectPtr& object)
{
  { // by_name
    if (!object->get_name().empty())
    {
      m_objects_by_name[object->get_name()] = object.get();
    }
  }

  { // by_id
    assert(object->get_uid());

    m_objects_by_uid[object->get_uid()] = object.get();
  }
}

void
GameObjectManager::this_before_object_remove(const GameObjectPtr& object)
{
  { // by_name
    const std::string& name = object->get_name();
    if (!name.empty())
    {
      m_objects_by_name.erase(name);
    }
  }

  { // by_id
    m_objects_by_uid.erase(object->get_uid());
  }
}

float
GameObjectManager::get_width() const
{
  float width = 0;
  for(auto& solids: get_solid_tilemaps()) {
    width = std::max(width, solids->get_bbox().get_right());
  }

  return width;
}

float
GameObjectManager::get_height() const
{
  float height = 0;
  for(const auto& solids: get_solid_tilemaps()) {
    height = std::max(height, solids->get_bbox().get_bottom());
  }

  return height;
}

float
GameObjectManager::get_tiles_width() const
{
  float width = 0;
  for(const auto& solids : get_solid_tilemaps()) {
    if (static_cast<float>(solids->get_width()) > width)
      width = static_cast<float>(solids->get_width());
  }
  return width;
}

float
GameObjectManager::get_tiles_height() const
{
  float height = 0;
  for(const auto& solids : get_solid_tilemaps()) {
    if (static_cast<float>(solids->get_height()) > height)
      height = static_cast<float>(solids->get_height());
  }
  return height;
}

/* EOF */
