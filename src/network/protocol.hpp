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

#ifndef HEADER_SUPERTUX_NETWORK_PROTOCOL_HPP
#define HEADER_SUPERTUX_NETWORK_PROTOCOL_HPP

#include <enet/enet.h>

#include "network/peer.hpp"

namespace network {

/** Represents a protocol, which handles events, occuring on the host
    its binded with. */
class Protocol
{
public:
  Protocol() {}
  virtual ~Protocol() {}

  virtual void on_connect(Peer& peer) {}
  virtual void on_disconnect(Peer& peer) {}
  virtual void on_packet_recieve(ENetPacket& packet) {}
};

} // namespace network

#endif

/* EOF */
