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

#ifndef HEADER_SUPERTUX_NETWORK_SERVER_USER_HPP
#define HEADER_SUPERTUX_NETWORK_SERVER_USER_HPP

#include <string>

#include "video/color.hpp"

class ReaderMapping;
class Writer;

namespace network {

/** Represents a remote peer, connected to a server, indirectly with other clients. */
class ServerUser
{
public:
  ServerUser(const std::string& nickname, Color nickname_color = Color(1, 1, 1, 1));
  ServerUser(const ReaderMapping& reader);

  virtual void write(Writer& writer) const;
  std::string serialize() const;

public:
  std::string nickname;
  Color nickname_color;

private:
  ServerUser(const ServerUser&) = delete;
  ServerUser& operator=(const ServerUser&) = delete;
};

} // namespace network

#endif

/* EOF */
