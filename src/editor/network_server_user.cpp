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

#include "editor/network_server_user.hpp"

#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

EditorServerUser::EditorServerUser(const std::string& username_, Color username_color_) :
  network::ServerUser(username_, username_color_),
  sector(),
  mouse_cursor()
{
}

EditorServerUser::EditorServerUser(const ReaderMapping& reader) :
  network::ServerUser(reader),
  sector(),
  mouse_cursor()
{
  reader.get("sector", sector);

  std::optional<ReaderMapping> cursor_mapping;
  if (reader.get("cursor", cursor_mapping))
    mouse_cursor.parse_state(*cursor_mapping);
}

void
EditorServerUser::write(Writer& writer) const
{
  network::ServerUser::write(writer);

  writer.write("sector", sector);

  writer.start_list("cursor");
  mouse_cursor.write_state(writer);
  writer.end_list("cursor");
}

/* EOF */
