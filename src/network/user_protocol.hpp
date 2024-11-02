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

#ifndef HEADER_SUPERTUX_NETWORK_USER_PROTOCOL_HPP
#define HEADER_SUPERTUX_NETWORK_USER_PROTOCOL_HPP

#include "network/protocol.hpp"

#include <unordered_map>

namespace network {

class Host;
class ServerUser;

template<class U>
class UserManager;

/** Abstract protocol, allowing for server user management and registration. */
template<class U = ServerUser>
class UserProtocol : public Protocol
{
  static_assert(std::is_base_of<ServerUser, U>::value, "U must derive from ServerUser!");

public:
  static bool verify_username(const std::string& username);

public:
  enum DisconnectionReason
  {
    DISCONNECTED_REGISTER_TIMED_OUT // The time for the client to send a registration packet in has expired.
      = network::DISCONNECTION_REASONS_END,
    DISCONNECTED_USERNAME_INVALID, // The provided username is invalid.
    DISCONNECTED_USERNAME_TAKEN // The provided username has already been taken by another client.
  };
  enum Operation
  {
    OP_USER_REGISTER,
    OP_USER_SERVER_CONNECT,
    OP_USER_SERVER_DISCONNECT,
    OP_USER_END
  };
  enum Channel
  {
    CH_USER_SERVER_CONNECTIONS,
    CH_USER_END
  };

public:
  UserProtocol(UserManager<U>& user_manager, Host& host);

  virtual size_t get_channel_count() const override { return CH_USER_END; }

protected:
  virtual void update() override;

  virtual void on_server_connect(Peer& peer) override;
  virtual void on_server_disconnect(Peer& peer, uint32_t) override;

  virtual bool allow_packet_send(Peer& peer) const override;
  virtual bool verify_packet(StagedPacket& packet) const override;
  virtual uint8_t get_packet_channel(const StagedPacket& packet) const override;

  virtual void on_packet_receive(ReceivedPacket packet) override;

  virtual StagedPacket on_request_receive(const ReceivedPacket& packet) override;
  virtual void on_request_response(const Request& request) override;

  virtual void get_remote_user_data(RemoteUser& user) const override;

  /** User-oriented virtual functions */
  virtual void on_user_connect(U& user) {}
  virtual void on_user_disconnect(U& user) {}

  /* If the host is a server, the return value indicates whether the packet should be broadcasted to all other users. */
  virtual bool on_user_packet_receive(const ReceivedPacket& packet, U& user) { return false; }

protected:
  UserManager<U>& m_user_manager;
  Host& m_host;

  /** Pending users without a received registration data packet. */
  std::unordered_map<ENetPeer*, std::unique_ptr<Timer>> m_pending_users;

private:
  UserProtocol(const UserProtocol&) = delete;
  UserProtocol& operator=(const UserProtocol&) = delete;
};

} // namespace network

#endif

/* EOF */
