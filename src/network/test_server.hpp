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

#ifndef HEADER_SUPERTUX_NETWORK_TEST_SERVER_HPP
#define HEADER_SUPERTUX_NETWORK_TEST_SERVER_HPP

#include "network/connection.hpp"
#include "network/connection_pool.hpp"
#include "network/connection_ptr.hpp"
#include "network/server.hpp"
#include "supertux/screen.hpp"

#include <string>

class TestServer final : public Screen
{
public:
  TestServer();
  virtual ~TestServer();

  virtual void setup() override;

  virtual void draw(Compositor& compositor) override {}
  virtual void update(float dt_sec, const Controller& controller) override;

  virtual IntegrationStatus get_status() const override { return IntegrationStatus(); }

private:
  void on_connect(network::ConnectionPtr connection);
  void on_receive(network::Connection* connection, const std::string& data);

private:
  network::Server m_server;
  std::unique_ptr<network::ConnectionPool> m_pool;

private:
  TestServer(const TestServer&) = delete;
  TestServer& operator=(const TestServer&) = delete;
};

#endif

/* EOF */
