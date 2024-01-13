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

#ifndef HEADER_SUPERTUX_NETWORK_PACKET_HPP
#define HEADER_SUPERTUX_NETWORK_PACKET_HPP

#include <enet/enet.h>

#include <string>

#include "util/uid.hpp"

namespace network {

class Peer;

/** Stores data to be sent over via an ENet packet.
    Converts it all to S-Expression format, when creating the packet. */
class StagedPacket final
{
  friend class Host;

public:
  StagedPacket(int code = -1, const std::string& data = {},
               float send_sec = 5.f);

  std::string get_staged_data() const;

private:
  const float send_time;

  std::string protocol;
  UID request_id; // Set if the packet is a part of a request
  UID response_id; // Set if the packet is a response to a request

public:
  int code;
  std::string data;

private:
  StagedPacket(const StagedPacket&) = delete;
  StagedPacket& operator=(const StagedPacket&) = delete;
};

/** Retrieves S-Expression-formatted ENet packet data.
    Can represent locally sent data,
    or recieved data from a remote peer (`peer` will be set). */
class RecievedPacket final
{
  friend class Host;

public:
  RecievedPacket(ENetPacket& packet, Peer* peer = nullptr);

public:
  ENetPacket& enet;
  Peer* peer;

private:
  std::string protocol;
  UID request_id; // Set if the packet is a part of a request
  UID response_id; // Set if the packet is a response to a request

public:
  int code;
  std::string data;

private:
  RecievedPacket(const RecievedPacket&) = delete;
  RecievedPacket& operator=(const RecievedPacket&) = delete;
};

} // namespace network

#endif

/* EOF */
