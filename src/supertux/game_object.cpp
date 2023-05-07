//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "supertux/game_object.hpp"

#include <algorithm>

#include "editor/editor.hpp"
#include "supertux/object_remove_listener.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "video/color.hpp"

GameObject::GameObject() :
  m_parent(),
  m_name(),
  m_type(0),
  m_fade_helpers(),
  m_track_undo(true),
  m_never_track_undo(false),
  m_previous_type(-1),
  m_uid(),
  m_scheduled_for_removal(false),
  m_last_state(),
  m_components(),
  m_remove_listeners()
{
}

GameObject::GameObject(const std::string& name) :
  m_parent(),
  m_name(name),
  m_type(0),
  m_fade_helpers(),
  m_track_undo(true),
  m_never_track_undo(false),
  m_previous_type(-1),
  m_uid(),
  m_scheduled_for_removal(false),
  m_last_state(),
  m_components(),
  m_remove_listeners()
{
}

GameObject::GameObject(const ReaderMapping& reader) :
  GameObject()
{
  reader.get("name", m_name, "");
}

GameObject::~GameObject()
{
  for (const auto& entry : m_remove_listeners) {
    entry->object_removed(this);
  }
  m_remove_listeners.clear();
}

void
GameObject::add_remove_listener(ObjectRemoveListener* listener)
{
  m_remove_listeners.push_back(listener);
}

void
GameObject::del_remove_listener(ObjectRemoveListener* listener)
{
  m_remove_listeners.erase(std::remove(m_remove_listeners.begin(),
                                       m_remove_listeners.end(),
                                       listener),
                           m_remove_listeners.end());
}

void
GameObject::save(Writer& writer)
{
  auto settings = get_settings();
  for (const auto& option : settings.get_options())
  {
    option->save(writer);
  }
}

std::string
GameObject::save()
{
  std::ostringstream save_stream;
  Writer writer(save_stream);
  save(writer);

  return save_stream.str();
}

ObjectSettings
GameObject::get_settings()
{
  ObjectSettings result(get_display_name());

  result.add_text(_("Name"), &m_name, "name", std::string());

  const GameObjectTypes types = get_types();
  if (!types.empty())
  {
    m_previous_type = m_type;

    std::vector<std::string> ids, names;
    for (const GameObjectType& type : types)
    {
      ids.push_back(type.id);
      names.push_back(type.name);
    }

    result.add_enum(_("Type"), &m_type, names, ids, 0, "type");
  }

  return result;
}

void
GameObject::save_state()
{
  if (!m_parent->undo_tracking_enabled())
  {
    m_last_state.clear();
    return;
  }

  if (m_last_state.empty())
    m_last_state = save();
}

void
GameObject::check_state()
{
  if (!m_parent->undo_tracking_enabled())
  {
    m_last_state.clear();
    return;
  }

  // If settings have changed, save the change.
  if (!m_last_state.empty())
  {
    if (m_last_state != save())
    {
      m_parent->save_object_change(*this, m_last_state);
    }
    m_last_state.clear();
  }
}

void
GameObject::parse_type(const ReaderMapping& reader)
{
  std::string type;
  if (reader.get("type", type))
  {
    try
    {
      set_type(type_id_to_value(type));
    }
    catch (...)
    {
      if (Editor::is_active())
        log_warning << "Unknown type of " << get_class_name() << ": '" << type << "', using default." << std::endl;
    }
  }
}

GameObjectTypes
GameObject::get_types() const
{
  return {};
}

void
GameObject::after_editor_set()
{
  // Check if the type has changed.
  if (m_previous_type > -1 &&
      m_previous_type != m_type)
  {
    on_type_change(m_previous_type);
  }
  m_previous_type = -1;
}

int
GameObject::type_id_to_value(const std::string& id) const
{
  const GameObjectTypes types = get_types();
  for (int i = 0; i < static_cast<int>(types.size()); i++)
  {
    if (types[i].id == id)
      return i;
  }
  throw std::runtime_error("Unknown ID '" + id + "' for " + get_class_name() + ".");
}

std::string
GameObject::type_value_to_id(int value) const
{
  const GameObjectTypes types = get_types();
  if (value >= 0 && value < static_cast<int>(types.size()))
  {
    return types[value].id;
  }
  throw std::runtime_error("Unknown value " + std::to_string(value) + " for " + get_class_name() + ".");
}

void
GameObject::update(float dt_sec)
{
  for (auto& h : m_fade_helpers)
  {
    h->update(dt_sec);
  }

  auto new_end = std::remove_if(m_fade_helpers.begin(), m_fade_helpers.end(), [](const std::unique_ptr<FadeHelper>& h) {
    return h->completed();
  });

  m_fade_helpers.erase(new_end, m_fade_helpers.end());
}

/* EOF */
