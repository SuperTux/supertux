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

#include "network/test_server.hpp"

#include <functional>

#include "network/server.hpp"
#include "util/log.hpp"

using namespace network;

TestServer::TestServer() :
  m_server(3474,
          [this](std::unique_ptr<Connection> c){ on_connect(std::move(c)); },
          [this](Connection* c, std::string d) { on_receive(c, d); }),
  m_pool()
{
  m_pool = std::make_unique<ConnectionPool>();
}

TestServer::~TestServer()
{
}

void
TestServer::setup()
{
  m_server.start();
  log_warning << "Server started" << std::endl;
}

void
TestServer::update(float dt_sec, const Controller& controller)
{
  //log_warning << "frame" << std::endl;
  //m_server.poll();
}

void
TestServer::on_connect(std::unique_ptr<Connection> connection)
{
  log_warning << "A new client has connected to the server:" << connection.get() << std::endl;
  m_pool->add_connection(std::move(connection));
}

void
TestServer::on_receive(Connection* connection, std::string data)
{
  connection->send("Received: " + data);
  if (data == "stop") {
    log_warning << "Stopping server" << std::endl;
    m_server.stop();
  }
  m_pool->send_all_except(connection->get_uuid() + ": " + data, connection);
}

/* EOF */
