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

#ifndef HEADER_SUPERTUX_NETWORK_REQUEST_HPP
#define HEADER_SUPERTUX_NETWORK_REQUEST_HPP

#include "network/packet.hpp"
#include "supertux/timer.hpp"

#include <memory>

namespace network {

/** Stores information about a request, consisting of a staged,
    and on success, a response packet. */
class Request final
{
  friend class Host;

public:
  enum class FailReason
  {
    REQUEST_TIMED_OUT, // Cannot send request: Timed out
    RESPONSE_TIMED_OUT // No response: Timed out
  };

public:
  Request(std::unique_ptr<StagedPacket> staged_packet, float response_sec);

private:
  UID id;

  const float response_time;
  Timer timer;

public:
  std::unique_ptr<StagedPacket> staged;
  RecievedPacket* recieved;

private:
  Request(const Request&) = delete;
  Request& operator=(const Request&) = delete;
};

} // namespace network

#endif

/* EOF */
