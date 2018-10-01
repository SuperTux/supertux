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
  m_wants_to_die(false),
  m_remove_listeners(),
  m_name()
{
}

GameObject::GameObject(const GameObject& rhs) :
  m_wants_to_die(rhs.m_wants_to_die),
  m_remove_listeners(),
  m_name(rhs.m_name)
{
}

GameObject::GameObject(const ReaderMapping& reader) :
  GameObject()
{
  reader.get("name", m_name, "");
}

GameObject::~GameObject()
{
  for(const auto& entry : m_remove_listeners) {
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
GameObject::save(Writer& writer) {
  if(m_name != "") {
    writer.write("name", m_name, false);
  }
  auto settings = get_settings();
  for(auto& option : settings.options)
  {
    if(option.is_savable()) {
      switch(option.type) {
        case MN_SCRIPT:
        case MN_TEXTFIELD:
        case MN_FILE:
        {
          auto value = *(reinterpret_cast<std::string*>(option.option));
          if(!(option.flags & OPTION_ALLOW_EMPTY) && value.empty())
            continue;
          writer.write(option.key, value);
        }
          break;
        case MN_NUMFIELD:
          writer.write(option.key, *(reinterpret_cast<float*>(option.option)));
          break;
        case MN_INTFIELD:
        case MN_STRINGSELECT:
          writer.write(option.key, *(reinterpret_cast<int*>(option.option)));
          break;
        case MN_TOGGLE:
          writer.write(option.key, *(reinterpret_cast<bool*>(option.option)));
          break;
        case MN_BADGUYSELECT:
          writer.write(option.key, *(reinterpret_cast<std::vector<std::string>*>(option.option)));
          break;
        case MN_COLOR:
          writer.write(option.key, reinterpret_cast<Color*>(option.option)->toVector());
          break;
        default:
          break;
      }
    }
  }
}

ObjectSettings
GameObject::get_settings() {
  ObjectSettings result(get_display_name());
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &m_name));
  return result;
}

/* EOF */
