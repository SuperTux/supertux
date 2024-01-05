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

#include "network/server.hpp"

#include <stdexcept>

namespace network {

Server::Server(enet_uint16 port, size_t peer_count, size_t channel_limit,
               enet_uint32 incoming_bandwidth, enet_uint32 outgoing_bandwidth) :
  Host()
{
  ENetAddress address;
  address.host = ENET_HOST_ANY;
  address.port = port;

  m_host = enet_host_create(&address, peer_count, channel_limit,
                            incoming_bandwidth, outgoing_bandwidth);
  if (!m_host)
    throw std::runtime_error("Error initializing ENet server!");
}

} // namespace network

/* EOF */
