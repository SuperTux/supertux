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

#ifndef HEADER_SUPERTUX_NETWORK_CONNECTION_POOL_HPP
#define HEADER_SUPERTUX_NETWORK_CONNECTION_POOL_HPP

#include <string>

#include <network/connection.hpp>

namespace network {

class ConnectionPool
{
public:
  ConnectionPool();

  void send_all(const std::string& data);
  void send_all_except(const std::string& data, Connection* connection);

  void add_connection(std::unique_ptr<Connection> connection);
  void remove_connection(Connection* connection);
  void clear_all_connections();
  void clear_closed_connections();

protected:
  std::vector<std::unique_ptr<Connection>> m_connections;

private:
  ConnectionPool(const ConnectionPool&) = delete;
  ConnectionPool& operator=(const ConnectionPool&) = delete;
};

} // namespace network

#endif

/* EOF */
