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
#include "supertux/game_object_factory.hpp"
#include "util/reader_iterator.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

bool GameObjectManager::s_draw_solids_only = false;

GameObjectManager::GameObjectManager() :
  m_uid_generator(),
  m_gameobjects(),
  m_gameobjects_new(),
  m_solid_tilemaps(),
  m_objects_by_name(),
  m_objects_by_uid(),
  m_objects_by_type_index(),
  m_name_resolve_requests()
{
}

GameObjectManager::~GameObjectManager()
{
  // clear_objects() must be called before destructing the GameObjectManager
  assert(m_gameobjects.size() == 0);
  assert(m_gameobjects_new.size() == 0);
}

void
GameObjectManager::request_name_resolve(const std::string& name, std::function<void (UID)> callback)
{
  m_name_resolve_requests.push_back({name, std::move(callback)});
}

void
GameObjectManager::process_resolve_requests()
{
  assert(m_gameobjects_new.empty());

  for (const auto& request : m_name_resolve_requests)
  {
    GameObject* object = get_object_by_name<GameObject>(request.name);
    if (!object)
    {
      log_warning << "GameObjectManager: name resolve for '" << request.name << "' failed" << std::endl;
      request.callback({});
    }
    else
    {
      request.callback(object->get_uid());
    }
  }
  m_name_resolve_requests.clear();
}

void
GameObjectManager::try_process_resolve_requests()
{
  assert(m_gameobjects_new.empty());
  std::vector<GameObjectManager::NameResolveRequest> new_list;

  for (const auto& request : m_name_resolve_requests)
  {
    auto* object = get_object_by_name<GameObject>(request.name);
    if (!object)
    {
      // Unlike process_resolve_requests(), we just keep that one in mind
      new_list.push_back(request);
    }
    else
    {
      request.callback(object->get_uid());
    }
  }

  m_name_resolve_requests.clear();
  m_name_resolve_requests.assign(new_list.begin(), new_list.end());
}

const std::vector<std::unique_ptr<GameObject> >&
GameObjectManager::get_objects() const
{
  return m_gameobjects;
}

GameObject&
GameObjectManager::add_object(std::unique_ptr<GameObject> object)
{
  assert(object);
  assert(!object->get_uid());

  object->set_uid(m_uid_generator.next());

  // make sure the object isn't already in the list
#ifndef NDEBUG
  for (const auto& game_object : m_gameobjects) {
    assert(game_object != object);
  }
  for (const auto& gameobject : m_gameobjects_new) {
    assert(gameobject != object);
  }
#endif

  GameObject& tmp = *object;
  m_gameobjects_new.push_back(std::move(object));
  return tmp;
}

void
GameObjectManager::clear_objects()
{
  flush_game_objects();

  for (const auto& obj: m_gameobjects) {
    before_object_remove(*obj);
  }
  m_gameobjects.clear();
}

void
GameObjectManager::update(float dt_sec)
{
  for (const auto& object : m_gameobjects)
  {
    if (!object->is_valid())
      continue;

    object->update(dt_sec);
  }
}

void
GameObjectManager::draw(DrawingContext& context)
{
  for (const auto& object : m_gameobjects)
  {
    if (!object->is_valid())
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
GameObjectManager::flush_game_objects()
{
  { // cleanup marked objects
    m_gameobjects.erase(
      std::remove_if(m_gameobjects.begin(), m_gameobjects.end(),
                     [this](const std::unique_ptr<GameObject>& obj) {
                       if (!obj->is_valid())
                       {
                         this_before_object_remove(*obj);
                         before_object_remove(*obj);
                         return true;
                       } else {
                         return false;
                       }
                     }),
      m_gameobjects.end());
  }

  { // add newly created objects
    // Objects might add new objects in finish_construction(), so we
    // loop until no new objects show up.
    while (!m_gameobjects_new.empty()) {
      auto new_objects = std::move(m_gameobjects_new);
      for (auto& object : new_objects)
      {
        if (before_object_add(*object))
        {
          this_before_object_add(*object);
          m_gameobjects.push_back(std::move(object));
        }
      }
    }
  }
  update_solids();
}

void
GameObjectManager::update_solids() 
{
  m_solid_tilemaps.clear();
  for (auto tilemap : get_objects_by_type_index(typeid(TileMap)))
  {
    TileMap* tm = static_cast<TileMap*>(tilemap);
    if (tm->is_solid()) m_solid_tilemaps.push_back(tm);
  }
}

void 
GameObjectManager::update_solid(TileMap* tm) {
  auto it = std::find(m_solid_tilemaps.begin(), m_solid_tilemaps.end(), tm);
  bool found = it != m_solid_tilemaps.end();
  if (tm->is_solid() && !found) {
    m_solid_tilemaps.push_back(tm);
  } else if(!tm->is_solid() && found) {
    m_solid_tilemaps.erase(it);
  }
}
void
GameObjectManager::this_before_object_add(GameObject& object)
{
  { // by_name
    if (!object.get_name().empty())
    {
      m_objects_by_name[object.get_name()] = &object;
    }
  }

  { // by_id
    assert(object.get_uid());

    m_objects_by_uid[object.get_uid()] = &object;
  }

  { // by_type_index
    m_objects_by_type_index[std::type_index(typeid(object))].push_back(&object);
  }
}

void
GameObjectManager::this_before_object_remove(GameObject& object)
{
  { // by_name
    const std::string& name = object.get_name();
    if (!name.empty())
    {
      m_objects_by_name.erase(name);
    }
  }

  { // by_id
    m_objects_by_uid.erase(object.get_uid());
  }

  { // by_type_index
    auto& vec = m_objects_by_type_index[std::type_index(typeid(object))];
    auto it = std::find(vec.begin(), vec.end(), &object);
    assert(it != vec.end());
    vec.erase(it);
  }
}

float
GameObjectManager::get_width() const
{
  float width = 0;
  for (auto& solids: get_solid_tilemaps()) {
    width = std::max(width, solids->get_bbox().get_right());
  }

  return width;
}

float
GameObjectManager::get_height() const
{
  float height = 0;
  for (const auto& solids: get_solid_tilemaps()) {
    height = std::max(height, solids->get_bbox().get_bottom());
  }

  return height;
}

float
GameObjectManager::get_tiles_width() const
{
  float width = 0;
  for (const auto& solids : get_solid_tilemaps()) {
    if (static_cast<float>(solids->get_width()) > width)
      width = static_cast<float>(solids->get_width());
  }
  return width;
}

float
GameObjectManager::get_tiles_height() const
{
  float height = 0;
  for (const auto& solids : get_solid_tilemaps()) {
    if (static_cast<float>(solids->get_height()) > height)
      height = static_cast<float>(solids->get_height());
  }
  return height;
}

void
GameObjectManager::backup(Writer& writer)
{
  writer.start_list("supertux-savestate");
  for (const auto& gameobject : m_gameobjects)
  {
    writer.start_list(gameobject->get_uid().key_string());
    gameobject->backup(writer);
    writer.end_list(gameobject->get_uid().key_string());
  }
  writer.end_list("supertux-savestate");
}

void
GameObjectManager::restore(const ReaderMapping& reader)
{
  /*std::cout << "### EXISTING ###" << std::endl;
  for (auto pair : m_objects_by_uid)
    std::cout << pair.first.to_string() << " - " << pair.first.key_string() << std::endl;*/

  ReaderIterator it = reader.get_iter();

  std::vector<UID> uids;
  //std::cout << "### BACKUPED ###" << std::endl;

  while (it.next())
  {
    try {
      //std::cout << it.get_key() << std::endl;
      UID uid = UID::from_key_string(it.get_key());
      uids.push_back(uid);

      GameObject* obj = nullptr;
      for (auto& pair : m_objects_by_uid) {
        if (pair.first == uid) {
          obj = pair.second;
          break;
        }
      }

      if (obj)
      {
        //std::cout << "Found: " << uid.to_string() << ": " << obj->get_uid().to_string() << std::endl;
        obj->restore(it.as_mapping());
      }
      else
      {
        //std::cout << "Not found, adding: " << uid.to_string() << std::endl;
        ReaderMapping mapping = it.as_mapping();
        std::string classname;
        if (!mapping.get("classname", classname)) {
          continue;
        }
        //std::cout << "would've added something of class " << classname << std::endl;
        std::unique_ptr<GameObject> new_obj =
                      GameObjectFactory::instance().create(classname, Vector());

        if (new_obj->is_singleton())
        {
          // Can't use get_singleton_by_type<T>() because it uses templates
          // TODO: Update the singleton
          continue;
        }

        new_obj->restore(mapping);
        add_object(std::move(new_obj));
      }
    } catch (...) {
      // Could not parse an object
    }
  }

  //std::cout << "###   END    ###" << std::endl;
  for (auto pair : m_objects_by_uid)
    if (std::find(uids.begin(), uids.end(), pair.first) == uids.end() && !pair.second->is_singleton())
      pair.second->remove_me();
}

/* EOF */