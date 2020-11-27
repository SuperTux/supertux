//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#include "network/client.hpp"

#include <boost/asio.hpp>

namespace network {

Client::Client(int port, const std::string& ip, std::function<void(Connection*, const std::string&)> handler) :
  Connection(nullptr, handler),
  m_port(port),
  m_ip(ip),
  m_runner(),
  io_service()
{
  m_socket = new boost::asio::ip::tcp::socket(io_service);
}

void
Client::init()
{
  m_socket->connect(boost::asio::ip::tcp::endpoint(
                              boost::asio::ip::address::from_string(m_ip),
                              static_cast<short unsigned int>(m_port)));

  m_runner = std::make_unique<std::thread>([this](){
    io_service.run();
  });

  m_runner->detach();
  
  Connection::init();
}

} // namespace network

/* EOF */
