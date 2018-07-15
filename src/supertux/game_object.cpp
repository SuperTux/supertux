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

#include "supertux/object_remove_listener.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "video/color.hpp"

GameObject::GameObject() :
  wants_to_die(false),
  remove_listeners(NULL),
  name()
{
}

GameObject::GameObject(const GameObject& rhs) :
  wants_to_die(rhs.wants_to_die),
  remove_listeners(NULL),
  name(rhs.name)
{
}

GameObject::GameObject(const ReaderMapping& reader) :
  GameObject()
{
  reader.get("name", name, "");
}

GameObject::~GameObject()
{
  // call remove listeners (and remove them from the list)
  auto entry = remove_listeners;
  while(entry != NULL) {
    auto next = entry->next;
    entry->listener->object_removed(this);
    delete entry;
    entry = next;
  }
}

void
GameObject::add_remove_listener(ObjectRemoveListener* listener)
{
  auto entry = new RemoveListenerListEntry();
  entry->next = remove_listeners;
  entry->listener = listener;
  remove_listeners = entry;
}

void
GameObject::del_remove_listener(ObjectRemoveListener* listener)
{
  auto entry = remove_listeners;
  if (entry->listener == listener) {
    remove_listeners = entry->next;
    delete entry;
    return;
  }
  auto next = entry->next;
  while(next != NULL) {
    if (next->listener == listener) {
      entry->next = next->next;
      delete next;
      break;
    }
    entry = next;
    next = next->next;
  }
}

void
GameObject::save(Writer& writer) {
  if(name != "") {
    writer.write("name", name, false);
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
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  return result;
}

/* EOF */
