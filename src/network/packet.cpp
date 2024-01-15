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

#include "network/packet.hpp"

#include <string>
#include <sstream>
#include <stdexcept>

#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

namespace network {

StagedPacket::StagedPacket(int code_, const std::string& data_, float send_sec) :
  StagedPacket(code_, std::vector<std::string>({ data_ }), send_sec)
{
}

StagedPacket::StagedPacket(int code_, std::vector<std::string> data_, float send_sec) :
  send_time(send_sec),
  protocol(),
  request_id(),
  response_id(),
  code(code_),
  data(std::move(data_)),
  foreign_broadcast(false)
{
}

StagedPacket::StagedPacket(const ReceivedPacket& packet) :
  send_time(5.f),
  protocol(),
  request_id(),
  response_id(),
  code(packet.code),
  data(packet.data),
  foreign_broadcast(true)
{
}

std::string
StagedPacket::get_staged_data() const
{
  std::ostringstream stream;
  Writer writer(stream);

  writer.start_list("supertux-network-packet");

  writer.write("protocol", protocol);
  if (request_id)
    writer.write("request-id", request_id);
  else if (response_id)
    writer.write("response-id", response_id);
  writer.write("code", code);
  writer.write("data", data);

  writer.end_list("supertux-network-packet");

  return stream.str();
}


ReceivedPacket::ReceivedPacket(ENetPacket& packet, Peer* peer_) :
  enet(packet),
  peer(peer_),
  protocol(),
  request_id(),
  response_id(),
  code(-1),
  data()
{
  std::istringstream stream(reinterpret_cast<const char*>(packet.data));
  auto doc = ReaderDocument::from_stream(stream);
  auto root = doc.get_root();
  if (root.get_name() != "supertux-network-packet")
    throw std::runtime_error("Received packet is not a valid 'supertux-network-packet'.");

  auto reader = root.get_mapping();
  reader.get("protocol", protocol);
  if (!reader.get("request-id", request_id))
    reader.get("response-id", response_id);
  if (!reader.get("code", code))
    throw std::runtime_error("Received packet does not contain a response code.");
  reader.get("data", data);
}

} // namespace network

/* EOF */
