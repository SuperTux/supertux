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

#ifndef HEADER_SUPERTUX_NETWORK_HOST_HPP
#define HEADER_SUPERTUX_NETWORK_HOST_HPP

#include <memory>

#include "network/protocol.hpp"

namespace network {

/** Represents an ENet host (server, client). */
class Host
{
public:
  Host();
  virtual ~Host();

  virtual void update();

  void send_packet(ENetPeer* peer, const char* data, enet_uint8 channel_id);

  void set_protocol(std::unique_ptr<Protocol> protocol) { m_protocol = std::move(protocol); }

protected:
  ENetHost* m_host;
  std::unique_ptr<Protocol> m_protocol;

private:
  Host(const Host&) = delete;
  Host& operator=(const Host&) = delete;
};

} // namespace network

#endif

/* EOF */
