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

#ifndef HEADER_SUPERTUX_NETWORK_PEER_HPP
#define HEADER_SUPERTUX_NETWORK_PEER_HPP

#include "network/address.hpp"

namespace network {

/** Collects data about an ENet peer. */
class Peer final
{
public:
  Peer(ENetPeer& peer);

public:
  ENetPeer& enet;
  Address address;
};

} // namespace network

#endif

/* EOF */
