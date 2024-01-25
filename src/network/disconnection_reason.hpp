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

#ifndef HEADER_SUPERTUX_NETWORK_DISCONNECTION_REASON_HPP
#define HEADER_SUPERTUX_NETWORK_DISCONNECTION_REASON_HPP

namespace network {

/** A server's reason for disconnecting a client. */
enum DisconnectionReason
{
  DISCONNECTED_PING_TIMEOUT, // The remote peer is no longer accessible.
  DISCONNECTED_OK, // Disconnected by a proper request.
  DISCONNECTED_SERVER_CLOSED, // The server was closed.
  DISCONNECTED_VERSION_MISMATCH, // Server and client SuperTux versions do not match.
  DISCONNECTED_NOT_WHITELISTED, // The client is not whitelisted on the server.
  DISCONNECTED_KICKED, // The client was kicked from the server.
  DISCONNECTED_BANNED, // The client was banned from the server.

  DISCONNECTION_REASONS_END // (Custom disconnection reasons in protocols should be implemented from here)
};

} // namespace network

#endif

/* EOF */
