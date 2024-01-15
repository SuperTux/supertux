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

#ifndef HEADER_SUPERTUX_NETWORK_CLIENT_HPP
#define HEADER_SUPERTUX_NETWORK_CLIENT_HPP

#include "network/host.hpp"

#include "network/connection_result.hpp"

namespace network {

/** A client, which can connect to servers. */
class Client final : public Host
{
public:
  Client(size_t outgoing_connections, size_t channel_limit = 1,
         uint32_t incoming_bandwidth = 0, uint32_t outgoing_bandwidth = 0);

  /** Connect/disconnect from a peer.
      On connection, if a protocol is binded, the provided
      allocated channel count will be ignored. */
  ConnectionResult connect(const char* hostname, uint16_t port,
                           uint32_t wait_ms, size_t allocated_channels = 0);
  void disconnect(ENetPeer* peer);

protected:
  void process_event(const ENetEvent& event) override;

private:
  ConnectionResult connect_internal(const char* hostname, uint16_t port,
                                    uint32_t wait_ms, size_t allocated_channels);

private:
  Client(const Client&) = delete;
  Client& operator=(const Client&) = delete;
};

} // namespace network

#endif

/* EOF */
