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
#include <vector>

#include "util/uid.hpp"

namespace network {

class Peer;
class ReceivedPacket;

/** Stores data to be sent over via an ENet packet.
    Converts it all to S-Expression format, when creating the packet. */
class StagedPacket final
{
  friend class Host;

public:
  StagedPacket(int code = -1, const std::string& data = {},
               float send_sec = 5.f);
  StagedPacket(int code = -1, std::vector<std::string> data = {},
               float send_sec = 5.f);
  StagedPacket(const ReceivedPacket& packet);

  std::string get_staged_data() const;

  bool is_part_of_request() const { return request_id || response_id; }

private:
  const float send_time;

  std::string protocol;
  UID request_id; // Set if the packet is a part of a request
  UID response_id; // Set if the packet is a response to a request

public:
  int code;
  std::vector<std::string> data;

  /** If set, this is a foreign packet being broadcasted to other peers. */
  const bool foreign_broadcast;

private:
  StagedPacket(const StagedPacket&) = default;
  StagedPacket& operator=(const StagedPacket&) = default;
};

/** Retrieves S-Expression-formatted ENet packet data.
    Can represent locally sent data,
    or received data from a remote peer (`peer` will be set). */
class ReceivedPacket final
{
  friend class Host;

public:
  ReceivedPacket(ENetPacket& packet, Peer* peer = nullptr);

public:
  ENetPacket& enet;
  Peer* peer;

private:
  std::string protocol;
  UID request_id; // Set if the packet is a part of a request
  UID response_id; // Set if the packet is a response to a request

public:
  int code;
  std::vector<std::string> data;

private:
  ReceivedPacket(const ReceivedPacket&) = default;
  ReceivedPacket& operator=(const ReceivedPacket&) = default;
};

} // namespace network

#endif

/* EOF */
