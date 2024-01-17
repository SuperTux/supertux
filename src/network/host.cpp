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

#include "network/host.hpp"

#include <cassert>

#include "supertux/timer.hpp"
#include "util/log.hpp"

namespace network {

void
Host::on_enet_packet_free(ENetPacket* packet)
{
  assert(packet->userData);

  Host* host = static_cast<Host*>(packet->userData);
  host->on_packet_send(packet);
}


Host::Host() :
  m_host(),
  m_protocol(),
  m_scheduled_for_removal(false),
  m_staged_packets(),
  m_staged_packets_new(),
  m_request_uid_generator(),
  m_requests(),
  m_requests_new()
{
}

Host::~Host()
{
  assert(m_host);
  enet_host_destroy(m_host);
}

void
Host::set_protocol(std::unique_ptr<Protocol> protocol)
{
  if (protocol)
  {
    enet_host_channel_limit(m_host, protocol->get_channel_count());
  }
  m_protocol = std::move(protocol);
}

void
Host::flush_packets()
{
  /** Merge in newly added packets */
  {
    auto it = m_staged_packets_new.begin();
    while (it != m_staged_packets_new.end())
    {
      m_staged_packets.insert(std::move(*it));
      it = m_staged_packets_new.erase(it);
    }
  }
  /** Merge in newly added pending requests */
  {
    auto it = m_requests_new.begin();
    while (it != m_requests_new.end())
    {
      m_requests.push_back(std::move(*it));
      it = m_requests_new.erase(it);
    }
  }
}

void
Host::update()
{
  flush_packets();

  /** Event handling on binded protocol */
  if (m_protocol)
  {
    ENetEvent event;
    while (enet_host_service(m_host, &event, 0) > 0)
    {
      switch (event.type)
      {
        case ENET_EVENT_TYPE_RECEIVE:
        {
          Peer peer(*event.peer);
          try
          {
            ReceivedPacket packet(*event.packet, &peer);
            if (m_protocol && packet.protocol != m_protocol->get_name())
            {
              throw std::runtime_error("Protocols '" + packet.protocol + "' and '" +
                                       m_protocol->get_name() + "' do not match.");
            }
            else if (!m_protocol && !packet.protocol.empty())
            {
              throw std::runtime_error("Remote protocol '" + packet.protocol +
                                       "' provided, but no local one set.");
            }

            if (packet.request_id) // The packet is a part of a request, ask protocol for response
            {
              try
              {
                StagedPacket response = m_protocol->on_request_receive(packet);
                response.response_id = packet.request_id;

                // Send the response over
                send_packet(event.peer, response, true, event.channelID);
              }
              catch (const std::exception& err)
              {
                log_warning << "Error processing received request in protocol: " << err.what() << std::endl;
              }
            }
            else if (packet.response_id) // The packet is a response to a request, notify protocol of request response
            {
              auto it = std::find_if(m_requests.begin(), m_requests.end(),
                                     [req_id = packet.response_id](const auto& req)
                {
                  return req->id == req_id;
                });
              if (it == m_requests.end())
              {
                log_warning << "Cannot notify protocol of request response: No request with ID " << packet.response_id << std::endl;
              }
              else
              {
                const auto& request = *it;
                request->received = &packet;
                try
                {
                  m_protocol->on_request_response(*request);
                }
                catch (const std::exception& err)
                {
                  log_warning << "Error processing request response in protocol: " << err.what() << std::endl;
                }

                // Remove the request now that it has completed
                m_requests.erase(it);
              }
            }
            else // The packet is not a part of a request, notify protocol of packet receive
            {
              try
              {
                m_protocol->on_packet_receive(std::move(packet));
              }
              catch (const std::exception& err)
              {
                log_warning << "Error processing received packet in protocol: " << err.what() << std::endl;
              }
            }
          }
          catch (const std::exception& err)
          {
            log_warning << "Error processing received packet: " << err.what() << std::endl;
          }

          // Clean up the packet now that we're done using it
          enet_packet_destroy(event.packet);
          break;
        }
        default:
          process_event(event);
          break;
      }

      flush_packets();
    }
  }

  /** Abort timed-out sent packets */
  {
    auto it = m_staged_packets.begin();
    while (it != m_staged_packets.end())
    {
      if (it->second->check()) // Packet send time has expired
      {
        log_warning << "Unable to send packet: Send time of " << it->second->get_timegone() << " exceeded." << std::endl;

        if (m_protocol)
        {
          ReceivedPacket packet(*it->first);
          if (packet.request_id) // The packet is a part of a request, notify protocol of request fail
          {
            auto it_req = std::find_if(m_requests.begin(), m_requests.end(),
                                       [req_id = packet.request_id](const auto& req)
              {
                return req->id == req_id;
              });
            if (it_req == m_requests.end())
            {
              log_warning << "Cannot notify protocol of request fail: No request with ID " << packet.request_id << std::endl;
            }
            else
            {
              try
              {
                m_protocol->on_request_fail(**it_req, Request::FailReason::REQUEST_TIMED_OUT);
              }
              catch (const std::exception& err)
              {
                log_warning << "Error processing request failure in protocol: " << err.what() << std::endl;
              }
              m_requests.erase(it_req);
            }
          }
          else // The packet is not a part of a request, notify protocol of packet abort
          {
            try
            {
              m_protocol->on_packet_abort(std::move(packet));
            }
            catch (const std::exception& err)
            {
              log_warning << "Error processing packet abort in protocol: " << err.what() << std::endl;
            }
          }
        }

        it->first->freeCallback = nullptr; // Prevent calling `on_packet_send()`
        enet_packet_destroy(it->first);

        it = m_staged_packets.erase(it);
      }
      else
        it++;
    }
  }

  /** Cancel timed-out requests */
  {
    auto it = m_requests.begin();
    while (it != m_requests.end())
    {
      const auto& request = *it;
      if (request->timer.check()) // Request response wait time has expired
      {
        log_warning << "Unable to receive request response: Wait time of "
                    << request->response_time << " exceeded." << std::endl;

        try
        {
          m_protocol->on_request_fail(*request, Request::FailReason::RESPONSE_TIMED_OUT);
        }
        catch (const std::exception& err)
        {
          log_warning << "Error processing request failure in protocol: " << err.what() << std::endl;
        }
        it = m_requests.erase(it);
      }
      else
        it++;
    }
  }

  /** Update binded protocol */
  if (m_protocol)
    m_protocol->update();
}

ENetPacket*
Host::create_packet(StagedPacket& packet, bool reliable)
{
  if (m_protocol)
  {
    if (!m_protocol->verify_packet(packet))
    {
      log_warning << "Aborting sending over packet with code " << packet.code
                  << ": Deemed as invalid by protocol '" << m_protocol->get_name()
                  << "'." << std::endl;
      return nullptr;
    }
    packet.protocol = m_protocol->get_name();
  }

  // Create a reliable packet containing the provided data
  const std::string staged_data = packet.get_staged_data();
  ENetPacket* enet_packet = enet_packet_create(staged_data.c_str(), staged_data.length() + 1,
                                               reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
  if (reliable)
  {
    enet_packet->userData = this;
    enet_packet->freeCallback = &on_enet_packet_free;

    // Set time period, in which the packet should be sent
    auto timer = std::make_unique<Timer>();
    timer->start(packet.send_time);
    m_staged_packets_new[enet_packet] = std::move(timer);
  }

  return enet_packet;
}

ENetPacket*
Host::send_packet(ENetPeer* peer, StagedPacket& packet,
                  bool reliable, uint8_t channel_id)
{
  if (!peer) return nullptr;
  assert(peer->host == m_host);

  // Do not send over to peers, blocked by the protocol
  if (m_protocol)
  {
    Peer peer_info(*peer);
    if (!m_protocol->allow_packet_send(peer_info))
    {
      log_warning << "Aborting sending over packet with code " << packet.code
                  << ": Communication with peer " << peer->incomingPeerID
                  << " not allowed." << std::endl;
      return nullptr;
    }
  }

  ENetPacket* enet_packet = create_packet(packet, reliable);
  if (!enet_packet) return nullptr;

  // If a protocol is binded, use the channel the protocol has determined for the packet
  if (m_protocol)
    channel_id = m_protocol->get_packet_channel(packet);

  // Send the packet over
  enet_peer_send(peer, static_cast<enet_uint8>(channel_id), enet_packet);

  return enet_packet;
}

ENetPacket*
Host::broadcast_packet(StagedPacket& packet, bool reliable,
                       ENetPeer* except_peer, uint8_t channel_id)
{
  ENetPacket* enet_packet = create_packet(packet, reliable);
  if (!enet_packet) return nullptr;

  // If a protocol is binded, use the channel the protocol has determined for the packet
  if (m_protocol)
    channel_id = m_protocol->get_packet_channel(packet);

  // Send the packet over to all peers, except the provided one
  for (ENetPeer* peer = m_host->peers; peer < &m_host->peers[m_host->peerCount]; peer++)
  {
    if (peer->state != ENET_PEER_STATE_CONNECTED ||
        peer == except_peer)
      continue;

    // Do not send over to peers, blocked by the protocol
    if (m_protocol)
    {
      Peer peer_info(*peer);
      if (!m_protocol->allow_packet_send(peer_info))
        continue;
    }

    enet_peer_send(peer, static_cast<enet_uint8>(channel_id), enet_packet);
  }

  // Destroy packet, if it was not sent to any peer
  if (enet_packet->referenceCount == 0)
  {
    enet_packet_destroy(enet_packet);
    return nullptr;
  }

  return enet_packet;
}

void
Host::send_request(ENetPeer* peer, std::unique_ptr<Request> request, uint8_t channel_id)
{
  assert(m_protocol); // A protocol is required to later handle the response

  request->id = m_request_uid_generator.next();
  request->staged->request_id = request->id;

  if (send_packet(peer, *request->staged, true, channel_id))
    m_requests_new.push_back(std::move(request));
}

void
Host::on_packet_send(ENetPacket* packet)
{
  assert(packet->userData == this);

  if (m_protocol && packet->referenceCount > 0) // Make sure the packet has been sent to at least 1 peer
  {
    ReceivedPacket packet_info(*packet);
    if (packet_info.request_id) // The packet is a part of a request, start the request's response timer
    {
      auto it = std::find_if(m_requests.begin(), m_requests.end(),
                             [req_id = packet_info.request_id](const auto& req)
        {
          return req->id == req_id;
        });
      if (it == m_requests.end())
      {
        log_warning << "Cannot start request response timer: No request with ID " << packet_info.request_id << std::endl;
      }
      else
      {
        const auto& request = *it;
        request->timer.start(request->response_time);
      }
    }
    else // The packet is not a part of a request, notify protocol of packet send
    {
      try
      {
        m_protocol->on_packet_send(std::move(packet_info));
      }
      catch (const std::exception& err)
      {
        log_warning << "Error processing packet send in protocol: " << err.what() << std::endl;
      }
    }
  }

  m_staged_packets.erase(packet);
  m_staged_packets_new.erase(packet);
}

} // namespace network

/* EOF */
