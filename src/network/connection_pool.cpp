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

#include "network/connection_pool.hpp"

#include "util/log.hpp"

namespace network {

ConnectionPool::ConnectionPool() :
  m_connections()
{
}

void
ConnectionPool::send_all(const std::string& data)
{
  clear_closed_connections();
  for (auto c = m_connections.begin(); c != m_connections.end(); ++c)
    (*c)->send(data);
}

void
ConnectionPool::send_all_except(const std::string& data, Connection* connection)
{
  clear_closed_connections();
  for (auto c = m_connections.begin(); c != m_connections.end(); ++c)
    if (*connection != **c)
      (*c)->send(data);
}

void
ConnectionPool::add_connection(std::unique_ptr<Connection> connection)
{
  m_connections.push_back(std::move(connection));
}

void
ConnectionPool::remove_connection(Connection* connection)
{
  for (auto c = m_connections.begin(); c != m_connections.end(); ++c)
  {
    if (*connection == **c)
    {
      m_connections.erase(c);
      break;
    }
  }
}

void
ConnectionPool::clear_all_connections()
{
  m_connections.clear();
}

// Awfully hacky code to remove all objects in m_connections which are closed
void
ConnectionPool::clear_closed_connections()
{
  for (;;)
  {
    bool has_removed_one = false;
    for (auto c = m_connections.begin(); c != m_connections.end(); ++c)
    {
      if (!c->get() || c->get()->is_closed())
      {
        m_connections.erase(c);
        has_removed_one = true;
        break;
      }
    }
    if (!has_removed_one)
      return;
  }
}


} // namespace network

/* EOF */
