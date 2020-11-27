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

#ifndef HEADER_SUPERTUX_NETWORK_CLIENT_HPP
#define HEADER_SUPERTUX_NETWORK_CLIENT_HPP

#include "network/connection.hpp"

namespace network {

// TODO: Make ServerTCP and ServertUDP
class Client : public Connection
{
public:
  Client(int port, const std::string& ip, std::function<void(Connection*, const std::string&)> handler);
  
  virtual void init() override;

private:
  int m_port;
  std::string m_ip;
  /** Thread used to handle connections */
  std::unique_ptr<std::thread> m_runner;

  // Internal - used by Boost
  boost::asio::io_service io_service;

private:
  Client(const Client&) = delete;
  Client operator=(const Client&) = delete;
};

} // namespace network

#endif

/* EOF */
