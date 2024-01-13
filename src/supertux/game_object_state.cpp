//  SuperTux
//  Copyright (C) 2024 Vankata453
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

#include "supertux/game_object_state.hpp"

#include "util/log.hpp"
#include "util/reader_iterator.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

GameObjectState::GameObjectState(const std::string& name_, const UID& uid_,
                                 const std::string& data_, Action action_) :
  name(name_),
  uid(uid_),
  data(data_),
  action(action_)
{
}

GameObjectState::GameObjectState(const ReaderMapping& reader) :
  name(),
  uid(),
  data(),
  action()
{
  reader.get("name", name);
  reader.get("uid", uid);
  reader.get("data", data);
  reader.get("action", reinterpret_cast<int&>(action));
}

void
GameObjectState::save(Writer& writer) const
{
  writer.write("name", name);
  writer.write("uid", uid);
  writer.write("data", data);
  writer.write("action", reinterpret_cast<const int&>(action));
}


GameObjectStates::GameObjectStates(const UID& uid_, std::vector<GameObjectState> objects_) :
  uid(uid_),
  objects(std::move(objects_))
{
}

GameObjectStates::GameObjectStates(const ReaderMapping& reader) :
  uid(),
  objects()
{
  auto iter = reader.get_iter();
  while (iter.next())
  {
    if (iter.get_key() != "object-state")
    {
      log_warning << "Unknown key '" << iter.get_key() << "' in GameObjectStates data. Ignoring." << std::endl;
      continue;
    }

    objects.push_back(GameObjectState(iter.as_mapping()));
  }
}

void
GameObjectStates::save(Writer& writer) const
{ 
  for (const auto& change : objects)
  {
    writer.start_list("object-state");
    change.save(writer);
    writer.end_list("object-state");
  }
}

/* EOF */
