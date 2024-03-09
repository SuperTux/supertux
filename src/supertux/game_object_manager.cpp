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

#include <simplesquirrel/class.hpp>
#include <simplesquirrel/vm.hpp>

#include "editor/editor.hpp"
#include "object/ambient_light.hpp"
#include "object/music_object.hpp"
#include "object/tilemap.hpp"
#include "supertux/game_object_factory.hpp"
#include "supertux/moving_object.hpp"

bool GameObjectManager::s_draw_solids_only = false;

GameObjectManager::GameObjectManager(bool undo_tracking) :
  m_initialized(false),
  m_uid_generator(),
  m_change_uid_generator(),
  m_undo_tracking(undo_tracking),
  m_undo_stack_size(20),
  m_undo_stack(),
  m_redo_stack(),
  m_pending_change_stack(),
  m_last_saved_change(),
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
  // clear_objects() must be called before destructing the GameObjectManager.
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
  // FIXME: Why is this assertion needed?
  // Removed to allow for resolving name requests in Sector before object creation,
  // despite there being queued objects.
  //assert(m_gameobjects_new.empty());

  for (const auto& request : m_name_resolve_requests)
  {
    GameObject* object = get_object_by_name<GameObject>(request.name);
    if (!object)
    {
      log_warning << "GameObjectManager: Name resolve for '" << request.name << "' failed." << std::endl;
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
  // FIXME: Why is this assertion needed?
  // Removed to allow for resolving name requests in Sector before object creation,
  // despite there being queued objects.
  //assert(m_gameobjects_new.empty());

  std::vector<GameObjectManager::NameResolveRequest> new_list;

  for (const auto& request : m_name_resolve_requests)
  {
    auto* object = get_object_by_name<GameObject>(request.name);
    if (!object)
    {
      // Unlike process_resolve_requests(), we just keep that one in mind.
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
  {
    object->set_uid(m_uid_generator.next());

    // No object UID would indicate the object is not a result of undo/redo.
    // Any newly placed object in the editor should be on its latest version.
    if (m_initialized && Editor::is_active())
      object->update_version();
  }

  // Make sure the object isn't already in the list.
#ifndef NDEBUG
  for (const auto& game_object : m_gameobjects) {
    assert(game_object != object);
  }
  for (const auto& gameobject : m_gameobjects_new) {
    assert(gameobject != object);
  }
#endif

  // Attempt to add object to editor layers.
  if (m_initialized && Editor::is_active())
    Editor::current()->add_layer(object.get());

  GameObject& tmp = *object;
  m_gameobjects_new.push_back(std::move(object));
  return tmp;
}

void
GameObjectManager::add_object(const std::string& class_name, const std::string& name,
                              float pos_x, float pos_y, const std::string& direction,
                              const std::string& data)
{
  add_object_scripting(class_name, name, Vector(pos_x, pos_y), direction, data);
}

MovingObject&
GameObjectManager::add_object_scripting(const std::string& class_name, const std::string& name,
                                        const Vector& pos, const std::string& direction,
                                        const std::string& data)
{
  if (class_name.empty())
    throw std::runtime_error("Object class name cannot be empty.");

  if (!name.empty() && get_object_by_name<GameObject>(name))
    throw std::runtime_error("Object with name '" + name + "' already exists.");

  auto obj = GameObjectFactory::instance().create(class_name, pos,
                                                  direction.empty() ? Direction::AUTO : string_to_dir(direction),
                                                  data);

  auto moving_object = dynamic_cast<MovingObject*>(obj.get());
  if (!moving_object)
    throw std::runtime_error("Only MovingObject instances can be created via scripting.");

  if (!name.empty())
    obj->set_name(name);

  add_object(std::move(obj));
  return *moving_object;
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
  { // Clean up marked objects.
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

  { // Add newly created objects.
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

          if (object->has_object_manager_priority())
            m_gameobjects.insert(m_gameobjects.begin(), std::move(object));
          else
            m_gameobjects.push_back(std::move(object));
        }
      }
    }
  }
  update_tilemaps();

  // A resolve request may depend on an object being added.
  try_process_resolve_requests();

  // If object changes have been performed since last flush, push them to the undo stack.
  if (m_undo_tracking && !m_pending_change_stack.empty())
  {
    m_undo_stack.push_back({ m_change_uid_generator.next(), std::move(m_pending_change_stack) });
    m_redo_stack.clear();
    undo_stack_cleanup();
  }

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
GameObjectManager::move_object(const UID& uid, GameObjectManager& other)
{
  if (&other == this)
    return;

  auto it = std::find_if(m_gameobjects.begin(), m_gameobjects.end(),
                         [uid](const auto& obj) {
                           return obj->get_uid() == uid;
                         });
  if (it == m_gameobjects.end())
  {
    std::ostringstream err;
    err << "Object with UID " << uid << " not found.";
    throw std::runtime_error(err.str());
  }

  this_before_object_remove(**it);
  before_object_remove(**it);

  other.add_object(std::move(*it));
  m_gameobjects.erase(it);

  other.flush_game_objects();
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
GameObjectManager::on_editor_save()
{
  m_last_saved_change = (m_undo_stack.empty() ? UID() : m_undo_stack.back().uid);
}

void
GameObjectManager::undo()
{
  if (m_undo_stack.empty()) return;
  ObjectChanges& changes = m_undo_stack.back();

  for (auto& obj_change : changes.objects)
    process_object_change(obj_change);

  m_redo_stack.push_back(std::move(changes));
  m_undo_stack.pop_back();
}

void
GameObjectManager::redo()
{
  if (m_redo_stack.empty()) return;
  ObjectChanges& changes = m_redo_stack.back();

  for (auto& obj_change : changes.objects)
    process_object_change(obj_change);

  m_undo_stack.push_back(std::move(changes));
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
    m_pending_change_stack.push_back({ object.get_class_name(), object.get_uid(), object.save(), creation });

  object.m_track_undo = true;
}

void
GameObjectManager::save_object_change(GameObject& object, const std::string& data)
{
  if (m_undo_tracking)
    m_pending_change_stack.push_back({ object.get_class_name(), object.get_uid(), data, false });
}

void
GameObjectManager::clear_undo_stack()
{
  m_undo_stack.clear();
  m_redo_stack.clear();
  m_last_saved_change = UID();
}

bool
GameObjectManager::has_object_changes() const
{
  return (m_undo_stack.empty() && m_last_saved_change) ||
         (!m_undo_stack.empty() && m_undo_stack.back().uid != m_last_saved_change);
}

void
GameObjectManager::this_before_object_add(GameObject& object)
{
  { // By name:
    if (!object.get_name().empty())
    {
      m_objects_by_name[object.get_name()] = &object;
    }
  }

  { // By id:
    assert(object.get_uid());

    m_objects_by_uid[object.get_uid()] = &object;
  }

  { // By type index:
    m_objects_by_type_index[std::type_index(typeid(object))].push_back(&object);
  }

  save_object_change(object, true);
}

void
GameObjectManager::this_before_object_remove(GameObject& object)
{
  save_object_change(object);

  { // By name:
    const std::string& name = object.get_name();
    if (!name.empty())
    {
      m_objects_by_name.erase(name);
    }
  }

  { // By id:
    m_objects_by_uid.erase(object.get_uid());
  }

  { // By type index:
    auto& vec = m_objects_by_type_index[std::type_index(typeid(object))];
    auto it = std::find(vec.begin(), vec.end(), &object);
    assert(it != vec.end());
    vec.erase(it);
  }
}

void
GameObjectManager::fade_to_ambient_light(float red, float green, float blue, float fadetime)
{
  get_singleton_by_type<AmbientLight>().fade_to_ambient_light(red, green, blue, fadetime);
}

void
GameObjectManager::set_ambient_light(float red, float green, float blue)
{
  get_singleton_by_type<AmbientLight>().set_ambient_light(Color(red, green, blue));
}

float
GameObjectManager::get_ambient_red() const
{
  return get_singleton_by_type<AmbientLight>().get_ambient_light().red;
}

float
GameObjectManager::get_ambient_green() const
{
  return get_singleton_by_type<AmbientLight>().get_ambient_light().green;
}

float
GameObjectManager::get_ambient_blue() const
{
  return get_singleton_by_type<AmbientLight>().get_ambient_light().blue;
}

void
GameObjectManager::set_music(const std::string& filename)
{
  get_singleton_by_type<MusicObject>().set_music(filename);
}

float
GameObjectManager::get_width() const
{
  float width = 0;
  for (auto& tilemap : get_solid_tilemaps())
    width = std::max(width, tilemap->get_bbox().get_right());

  return width;
}

float
GameObjectManager::get_height() const
{
  float height = 0;
  for (const auto& tilemap : get_solid_tilemaps())
    height = std::max(height, tilemap->get_bbox().get_bottom());

  return height;
}

float
GameObjectManager::get_editor_width() const
{
  float width = 0;
  for (const auto& tilemap : get_all_tilemaps()) // Determine from all tilemaps
    width = std::max(width, tilemap->get_bbox().get_right());

  return width;
}

float
GameObjectManager::get_editor_height() const
{
  float height = 0;
  for (const auto& tilemap : get_all_tilemaps()) // Determine from all tilemaps
    height = std::max(height, tilemap->get_bbox().get_bottom());

  return height;
}

float
GameObjectManager::get_tiles_width() const
{
  float width = 0;
  for (const auto& tilemap : get_solid_tilemaps())
  {
    if (static_cast<float>(tilemap->get_width()) > width)
      width = static_cast<float>(tilemap->get_width());
  }
  return width;
}

float
GameObjectManager::get_tiles_height() const
{
  float height = 0;
  for (const auto& tilemap : get_solid_tilemaps())
  {
    if (static_cast<float>(tilemap->get_height()) > height)
      height = static_cast<float>(tilemap->get_height());
  }
  return height;
}


void
GameObjectManager::register_class(ssq::VM& vm)
{
  ssq::Class cls = vm.addAbstractClass<GameObjectManager>("GameObjectManager");

  cls.addFunc("set_ambient_light", &GameObjectManager::set_ambient_light);
  cls.addFunc("fade_to_ambient_light", &GameObjectManager::fade_to_ambient_light);
  cls.addFunc("get_ambient_red", &GameObjectManager::get_ambient_red);
  cls.addFunc("get_ambient_green", &GameObjectManager::get_ambient_green);
  cls.addFunc("get_ambient_blue", &GameObjectManager::get_ambient_blue);
  cls.addFunc("set_music", &GameObjectManager::set_music);
  cls.addFunc<void, GameObjectManager, const std::string&, const std::string&,
              float, float, const std::string&, const std::string&>("add_object", &GameObjectManager::add_object);
}

/* EOF */
