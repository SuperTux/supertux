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

#ifndef HEADER_SUPERTUX_NETWORK_REMOTE_USER_HPP
#define HEADER_SUPERTUX_NETWORK_REMOTE_USER_HPP

#include <string>

#include "network/peer.hpp"

namespace network {

/** Represents a peer as a user with additional data.
    Used in ServerManagementMenu. */
class RemoteUser final
{
public:
  RemoteUser(ENetPeer& peer);

public:
  const Peer peer;

  std::string display_text;
};

} // namespace network

#endif

/* EOF */
