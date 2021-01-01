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

#include "supertux/object_remove_listener.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "video/color.hpp"

GameObject::GameObject() :
  m_name(),
  m_fade_helpers(),
  m_uid(),
  m_scheduled_for_removal(false),
  m_components(),
  m_remove_listeners()
{
}

GameObject::GameObject(const std::string& name) :
  m_name(name),
  m_fade_helpers(),
  m_uid(),
  m_scheduled_for_removal(false),
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
  for (const auto& option_ptr : settings.get_options())
  {
    const auto& option = *option_ptr;
    option.save(writer);
  }
}

ObjectSettings
GameObject::get_settings()
{
  ObjectSettings result(get_display_name());
  result.add_text(_("Name"), &m_name, "name", std::string());
  return result;
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

void
GameObject::backup(Writer& writer) const
{
  writer.write("classname", get_class());
  writer.start_list(GameObject::get_class());
  writer.write("name", m_name);
  writer.end_list(GameObject::get_class());
}

void
GameObject::restore(const ReaderMapping& reader)
{
  boost::optional<ReaderMapping> subreader(ReaderMapping(reader.get_doc(), reader.get_sexp()));

  if (reader.get(GameObject::get_class().c_str(), subreader))
  {
    int id;
    if (subreader->get("uid", id))
    {
      m_uid = UID(id);
    }
    subreader->get("name", m_name);
  }
}

/* EOF */
