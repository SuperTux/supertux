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
#include <unordered_map>

#include "network/protocol.hpp"
#include "util/uid_generator.hpp"

class Timer;

namespace network {

/** Represents an ENet host (server, client), which can send and recieve packets. */
class Host
{
private:
  static void on_enet_packet_free(ENetPacket* packet);

public:
  Host();
  virtual ~Host();

  virtual void update();

  /** Sending packets and requests.
      Provided channel ID will be ignored, if a protocol is binded. */
  ENetPacket* send_packet(ENetPeer* peer, StagedPacket& packet,
                          bool reliable, uint8_t channel_id = 0);
  ENetPacket* broadcast_packet(StagedPacket& packet, bool reliable, uint8_t channel_id = 0);
  ENetPacket* broadcast_packet_except(ENetPeer* peer, StagedPacket& packet,
                                      bool reliable, uint8_t channel_id = 0);
  void send_request(ENetPeer* peer, std::unique_ptr<Request> request, uint8_t channel_id = 0);

  size_t get_channel_limit() const { return m_host->channelLimit; }
  size_t get_peer_limit() const { return m_host->peerCount; }
  size_t get_connected_peers() const { return m_host->connectedPeers; }

  void set_protocol(std::unique_ptr<Protocol> protocol);

protected:
  virtual void process_event(const ENetEvent& event) {}

private:
  ENetPacket* create_packet(StagedPacket& packet, bool reliable);

  void on_packet_send(ENetPacket* packet);

protected:
  ENetHost* m_host;
  std::unique_ptr<Protocol> m_protocol;

private:
  std::unordered_map<ENetPacket*, std::unique_ptr<Timer>> m_packet_timers;

  UIDGenerator m_request_uid_generator;
  std::vector<std::unique_ptr<Request>> m_requests;

private:
  Host(const Host&) = delete;
  Host& operator=(const Host&) = delete;
};

} // namespace network

#endif

/* EOF */
