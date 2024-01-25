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

#ifndef HEADER_SUPERTUX_NETWORK_SERVER_HPP
#define HEADER_SUPERTUX_NETWORK_SERVER_HPP

#include "network/host.hpp"

#include "network/address.hpp"
#include "network/remote_user.hpp"

namespace network {

/** A server, which clients can connect to. */
class Server final : public Host
{
public:
  Server(uint16_t port, size_t peer_count, size_t channel_limit = 1,
         uint32_t incoming_bandwidth = 0, uint32_t outgoing_bandwidth = 0);
  ~Server() override;

  void disconnect(ENetPeer* peer, uint32_t code = 0);

  /** Server moderation */
  void kick(ENetPeer* peer);
  void ban(ENetPeer* peer);

  std::vector<RemoteUser> get_users() const;
  Address get_address() const;

protected:
  void process_event(const ENetEvent& event) override;

private:
  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;
};

} // namespace network

#endif

/* EOF */
