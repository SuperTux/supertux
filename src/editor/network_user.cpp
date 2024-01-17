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

#include "editor/network_user.hpp"

#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

EditorNetworkUser::EditorNetworkUser(const std::string& nickname_, Color nickname_color_) :
  nickname(nickname_),
  nickname_color(nickname_color_),
  sector(),
  mouse_cursor()
{
}

EditorNetworkUser::EditorNetworkUser(const ReaderMapping& reader) :
  nickname(),
  nickname_color(1, 1, 1, 1),
  sector(),
  mouse_cursor()
{
  reader.get("nickname", nickname);
  std::vector<float> v_nickcolor;
  if (reader.get("nickname-color", v_nickcolor))
    nickname_color = Color(v_nickcolor);

  reader.get("sector", sector);

  std::optional<ReaderMapping> cursor_mapping;
  if (reader.get("cursor", cursor_mapping))
    mouse_cursor.parse_state(*cursor_mapping);
}

void
EditorNetworkUser::write(Writer& writer) const
{
  writer.write("nickname", nickname);
  auto v_nickcolor = nickname_color.toVector();
  v_nickcolor.pop_back(); // Remove alpha
  writer.write("nickname-color", v_nickcolor);

  writer.write("sector", sector);

  writer.start_list("cursor");
  mouse_cursor.write_state(writer);
  writer.end_list("cursor");
}

std::string
EditorNetworkUser::serialize() const
{
  std::ostringstream stream;
  Writer writer(stream);

  writer.start_list("supertux-editor-network-user");
  write(writer);
  writer.end_list("supertux-editor-network-user");

  return stream.str();
}

/* EOF */
