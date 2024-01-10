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

#include "supertux/game_object_change.hpp"

#include "util/log.hpp"
#include "util/reader_iterator.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

GameObjectChange::GameObjectChange(const std::string& name_, const UID& uid_,
                                   const std::string& data_, bool creation_) :
  name(name_),
  uid(uid_),
  data(data_),
  creation(creation_)
{
}

GameObjectChange::GameObjectChange(const ReaderMapping& reader) :
  name(),
  uid(),
  data(),
  creation()
{
  reader.get("name", name);
  reader.get("uid", uid);
  reader.get("data", data);
  reader.get("creation", creation);
}

void
GameObjectChange::save(Writer& writer) const
{
  writer.write("name", name);
  writer.write("uid", uid);
  writer.write("data", data);
  writer.write("creation", creation);
}


GameObjectChanges::GameObjectChanges(const UID& uid_, std::vector<GameObjectChange> objects_) :
  uid(uid_),
  objects(std::move(objects_))
{
}

GameObjectChanges::GameObjectChanges(const ReaderMapping& reader) :
  uid(),
  objects()
{
  auto iter = reader.get_iter();
  while (iter.next())
  {
    if (iter.get_key() != "object-change")
    {
      log_warning << "Unknown key '" << iter.get_key() << "' in GameObjectChanges data. Ignoring." << std::endl;
      continue;
    }

    objects.push_back(GameObjectChange(iter.as_mapping()));
  }
}

void
GameObjectChanges::save(Writer& writer) const
{ 
  for (const auto& change : objects)
  {
    writer.start_list("object-change");
    change.save(writer);
    writer.end_list("object-change");
  }
}

/* EOF */
