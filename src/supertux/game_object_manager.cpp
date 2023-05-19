//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2018 Ingo Ruhnke <grumbel@gmail.com>
//                2023 Vankata453
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

#include "editor/editor.hpp"
#include "object/tilemap.hpp"
#include "supertux/game_object_factory.hpp"

bool GameObjectManager::s_draw_solids_only = false;

GameObjectManager::GameObjectManager(bool undo_tracking) :
  m_initialized(false),
  m_uid_generator(),
  m_undo_tracking(undo_tracking),
  m_undo_stack_size(20),
  m_undo_stack(),
  m_redo_stack(),
  m_gameobjects(),
  m_gameobjects_new(),
  m_solid_tilemaps(),
  m_all_tilemaps(),
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

  object->m_parent = this;

  if (!object->get_uid())
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

  // Attempt to add object to editor layers
  if (Editor::is_active())
    Editor::current()->add_layer(object.get());

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
          if (!m_initialized) object->m_track_undo = false;
          this_before_object_add(*object);
          m_gameobjects.push_back(std::move(object));
        }
      }
    }
  }
  update_tilemaps();

  m_initialized = true;
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
GameObjectManager::update_tilemaps()
{
  m_solid_tilemaps.clear();
  m_all_tilemaps.clear();
  for (auto tilemap : get_objects_by_type_index(typeid(TileMap)))
  {
    TileMap* tm = static_cast<TileMap*>(tilemap);
    if (tm->is_solid()) m_solid_tilemaps.push_back(tm);
    m_all_tilemaps.push_back(tm);
  }
}

void
GameObjectManager::toggle_undo_tracking(bool enabled)
{
  if (m_undo_tracking == enabled)
    return;

  m_undo_tracking = enabled;
  clear_undo_stack();
}

void
GameObjectManager::set_undo_stack_size(int size)
{
  if (m_undo_stack_size == size)
    return;

  m_undo_stack_size = size;
  undo_stack_cleanup();
}

void
GameObjectManager::undo_stack_cleanup()
{
  const int current_size = static_cast<int>(m_undo_stack.size());
  if (current_size > m_undo_stack_size)
    m_undo_stack.erase(m_undo_stack.begin(),
                       m_undo_stack.begin() + (current_size - m_undo_stack_size));
}

void
GameObjectManager::undo()
{
  if (m_undo_stack.empty()) return;
  ObjectChange& change = m_undo_stack.back();

  process_object_change(change);

  m_redo_stack.push_back(change);
  m_undo_stack.pop_back();
}

void
GameObjectManager::redo()
{
  if (m_redo_stack.empty()) return;
  ObjectChange& change = m_redo_stack.back();

  process_object_change(change);

  m_undo_stack.push_back(change);
  m_redo_stack.pop_back();
}

void
GameObjectManager::create_object_from_change(const ObjectChange& change)
{
  auto object = GameObjectFactory::instance().create(change.name, change.data);
  object->m_track_undo = false;
  object->set_uid(change.uid);
  object->after_editor_set();
  add_object(std::move(object));
}

void
GameObjectManager::process_object_change(ObjectChange& change)
{
  GameObject* object = get_object_by_uid<GameObject>(change.uid);
  if (object) // Object exists, remove it.
  {
    object->m_track_undo = false;
    object->remove_me();

    const std::string data = object->save();

    // If settings have changed, re-create object with old settings.
    if (!change.creation && change.data != data)
      create_object_from_change(change);

    change.data = std::move(data);
  }
  else // Object doesn't exist, create it.
  {
    create_object_from_change(change);
  }
}

void
GameObjectManager::save_object_change(GameObject& object, bool creation)
{
  if (m_undo_tracking && object.track_state() && object.m_track_undo)
  {
    m_undo_stack.push_back({ object.get_class_name(), object.get_uid(), object.save(), creation });
    m_redo_stack.clear();
    undo_stack_cleanup();
  }
  object.m_track_undo = true;
}

void
GameObjectManager::save_object_change(GameObject& object, const std::string& data)
{
  if (!m_undo_tracking) return;

  m_undo_stack.push_back({ object.get_class_name(), object.get_uid(), data, false });
  m_redo_stack.clear();
  undo_stack_cleanup();
}

void
GameObjectManager::clear_undo_stack()
{
  m_undo_stack.clear();
  m_redo_stack.clear();
}

bool
GameObjectManager::has_object_changes() const
{
  return !m_undo_stack.empty();
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

  save_object_change(object, true);
}

void
GameObjectManager::this_before_object_remove(GameObject& object)
{
  save_object_change(object);

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
  for (auto& tilemap: get_all_tilemaps()) {
    width = std::max(width, tilemap->get_bbox().get_right());
  }

  return width;
}

float
GameObjectManager::get_height() const
{
  float height = 0;
  for (const auto& tilemap: get_all_tilemaps()) {
    height = std::max(height, tilemap->get_bbox().get_bottom());
  }

  return height;
}

float
GameObjectManager::get_tiles_width() const
{
  float width = 0;
  for (const auto& tilemap : get_all_tilemaps()) {
    if (static_cast<float>(tilemap->get_width()) > width)
      width = static_cast<float>(tilemap->get_width());
  }
  return width;
}

float
GameObjectManager::get_tiles_height() const
{
  float height = 0;
  for (const auto& tilemap : get_all_tilemaps()) {
    if (static_cast<float>(tilemap->get_height()) > height)
      height = static_cast<float>(tilemap->get_height());
  }
  return height;
}

/* EOF */
