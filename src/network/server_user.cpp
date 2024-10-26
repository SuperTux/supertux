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

#include "network/server_user.hpp"

#include <sstream>

#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

namespace network {

ServerUser::ServerUser(const std::string& nickname_, Color nickname_color_) :
  nickname(nickname_),
  nickname_color(nickname_color_)
{
}

ServerUser::ServerUser(const ReaderMapping& reader) :
  nickname(),
  nickname_color(1, 1, 1, 1)
{
  reader.get("nickname", nickname);
  std::vector<float> v_nickcolor;
  if (reader.get("nickname-color", v_nickcolor))
    nickname_color = Color(v_nickcolor);
}

void
ServerUser::write(Writer& writer) const
{
  writer.write("nickname", nickname);
  auto v_nickcolor = nickname_color.toVector();
  v_nickcolor.pop_back(); // Remove alpha
  writer.write("nickname-color", v_nickcolor);
}

std::string
ServerUser::serialize() const
{
  std::ostringstream stream;
  Writer writer(stream);

  writer.start_list("supertux-server-user");
  write(writer);
  writer.end_list("supertux-server-user");

  return stream.str();
}

} // namespace network

/* EOF */
