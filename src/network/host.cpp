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
  m_packet_timers(),
  m_request_uid_generator(),
  m_requests()
{
}

Host::~Host()
{
  assert(m_host);
  enet_host_destroy(m_host);
}

void
Host::update()
{
  if (m_protocol)
  {
    /** Event handling on binded protocol */
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
            RecievedPacket packet(*event.packet, &peer);
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
              StagedPacket response = m_protocol->on_request_recieve(packet);
              response.response_id = packet.request_id;

              // Send the response over
              send_packet(event.peer, response, event.channelID); 
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
                request->recieved = &packet;
                m_protocol->on_request_response(*request);

                // Remove the request now that it has completed
                m_requests.erase(it);
              }
            }
            else // The packet is not a part of a request, notify protocol of packet recieve
            {
              m_protocol->on_packet_recieve(packet);
            }
          }
          catch (const std::exception& err)
          {
            log_warning << "Error processing recieved packet: " << err.what() << std::endl;
          }

          // Clean up the packet now that we're done using it
          enet_packet_destroy(event.packet);
          break;
        }
        default:
          process_event(event);
          break;
      }
    }

    /** Abort timed-out sent packets */
    {
      auto it = m_packet_timers.begin();
      while (it != m_packet_timers.end())
      {
        if (it->second->check()) // Packet send time has expired
        {
          log_warning << "Unable to send packet: Send time of " << it->second->get_timegone() << " exceeded." << std::endl;

          if (m_protocol)
          {
            RecievedPacket packet(*it->first);
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
                m_protocol->on_request_fail(**it_req, Request::FailReason::REQUEST_TIMED_OUT);
                m_requests.erase(it_req);
              }
            }
            else // The packet is not a part of a request, notify protocol of packet abort
            {
              m_protocol->on_packet_abort(packet);
            }
          }

          it->first->freeCallback = nullptr; // Prevent calling `on_packet_send()`
          enet_packet_destroy(it->first);

          it = m_packet_timers.erase(it);
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
          log_warning << "Unable to recieve request response: Wait time of "
                      << request->response_time << " exceeded." << std::endl;

          m_protocol->on_request_fail(*request, Request::FailReason::RESPONSE_TIMED_OUT);
          it = m_requests.erase(it);
        }
        else
          it++;
      }
    }

    /** Update binded protocol */
    m_protocol->update();
  }
}

ENetPacket*
Host::create_packet(StagedPacket& packet)
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
                                               ENET_PACKET_FLAG_RELIABLE);
  enet_packet->userData = this;
  enet_packet->freeCallback = &on_enet_packet_free;

  // Set time period, in which the packet should be sent
  auto timer = std::make_unique<Timer>();
  timer->start(packet.send_time);
  m_packet_timers[enet_packet] = std::move(timer);

  return enet_packet;
}

ENetPacket*
Host::send_packet(ENetPeer* peer, StagedPacket& packet, uint8_t channel_id)
{
  if (!peer) return nullptr;
  assert(peer->host == m_host);

  ENetPacket* enet_packet = create_packet(packet);
  if (!enet_packet) return nullptr;

  // Send the packet over
  enet_peer_send(peer, static_cast<enet_uint8>(channel_id), enet_packet);

  return enet_packet;
}

ENetPacket*
Host::broadcast_packet(StagedPacket& packet, uint8_t channel_id)
{
  ENetPacket* enet_packet = create_packet(packet);
  if (!enet_packet) return nullptr;

  // Send the packet over to all peers
  enet_host_broadcast(m_host, static_cast<enet_uint8>(channel_id), enet_packet);

  return enet_packet;
}

void
Host::send_request(ENetPeer* peer, std::unique_ptr<Request> request, uint8_t channel_id)
{
  assert(m_protocol); // A protocol is required to later handle the response

  request->id = m_request_uid_generator.next();
  request->staged->request_id = request->id;

  if (send_packet(peer, *request->staged, channel_id))
    m_requests.push_back(std::move(request));
}

void
Host::on_packet_send(ENetPacket* packet)
{
  assert(packet->userData == this);

  if (m_protocol)
  {
    RecievedPacket packet_info(*packet);
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
      m_protocol->on_packet_send(packet_info);
    }
  }

  m_packet_timers.erase(packet);
}

} // namespace network

/* EOF */
